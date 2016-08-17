#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/synchronization.h>
#include <3ds/services/apt.h>
#include <3ds/applets/swkbd.h>
#include <3ds/ipc.h>
#include <3ds/env.h>
#include <3ds/util/utf.h>

static char* swkbdSharedMem;
static Handle swkbdSharedMemHandle;

void swkbdInit(SwkbdState* swkbd, SwkbdType type, int numButtons, int maxTextLength)
{
	memset(swkbd, 0, sizeof(*swkbd));
	swkbd->type = type;
	swkbd->num_buttons_m1 = numButtons-1;
	swkbd->max_text_len = maxTextLength > 0 ? maxTextLength : 0xFDE8;
	swkbd->button_submits_text[SWKBD_BUTTON_CONFIRM] = true;
	swkbd->initial_text_offset = -1;
	swkbd->dict_offset = -1;
	swkbd->initial_status_offset = -1;
	swkbd->initial_learning_offset = -1;
	swkbd->version = 5;
	swkbd->result = SWKBD_NONE;
	swkbd->status_offset = -1;
	swkbd->learning_offset = -1;
	swkbd->text_offset = -1;
}

static void swkbdConvertToUTF8(char* out, const u16* in, int max)
{
	if (!in || !*in)
	{
		out[0] = 0;
		return;
	}

	ssize_t units = utf16_to_utf8((uint8_t*)out, in, max);
	if (units < 0)
	{
		out[0] = 0;
		return;
	}

	out[units] = 0;
}

static void swkbdConvertToUTF16(u16* out, const char* in, int max)
{
	if (!in || !*in)
	{
		out[0] = 0;
		return;
	}

	ssize_t units = utf8_to_utf16(out, (const uint8_t*)in, max);
	if (units < 0)
	{
		out[0] = 0;
		return;
	}

	out[units] = 0;
}

static const u16 swkbdFeatures[] =
{
	offsetof(SwkbdState, is_parental_screen),
	offsetof(SwkbdState, darken_top_screen),
	offsetof(SwkbdState, predictive_input),
	offsetof(SwkbdState, multiline),
	offsetof(SwkbdState, fixed_width),
	offsetof(SwkbdState, allow_home),
	offsetof(SwkbdState, allow_reset),
	offsetof(SwkbdState, allow_power),
	offsetof(SwkbdState, unknown),
	offsetof(SwkbdState, default_qwerty),
};

void swkbdSetFeatures(SwkbdState* swkbd, u32 features)
{
	int i;
	for (i = 0; i < (sizeof(swkbdFeatures)/sizeof(u16)); i ++)
		*((u8*)swkbd + swkbdFeatures[i]) = (features & BIT(i)) ? 1 : 0;
}

void swkbdSetHintText(SwkbdState* swkbd, const char* text)
{
	swkbdConvertToUTF16(swkbd->hint_text, text, SWKBD_MAX_HINT_TEXT_LEN);
}

void swkbdSetButton(SwkbdState* swkbd, SwkbdButton button, const char* text, bool submit)
{
	swkbdConvertToUTF16(swkbd->button_text[button], text, SWKBD_MAX_BUTTON_TEXT_LEN);
	swkbd->button_submits_text[button] = submit;
}

void swkbdSetInitialText(SwkbdState* swkbd, const char* text)
{
	swkbd->extra.initial_text = text;
}

void swkbdSetDictWord(SwkbdDictWord* word, const char* reading, const char* text)
{
	swkbdConvertToUTF16(word->reading, reading, SWKBD_MAX_WORD_LEN);
	swkbdConvertToUTF16(word->word, text, SWKBD_MAX_WORD_LEN);
	word->language = 0;
	word->all_languages = true;
}

void swkbdSetDictionary(SwkbdState* swkbd, const SwkbdDictWord* dict, int wordCount)
{
	swkbd->dict_word_count = dict ? wordCount : 0;
	swkbd->extra.dict = dict;
}

void swkbdSetStatusData(SwkbdState* swkbd, SwkbdStatusData* data, bool in, bool out)
{
	swkbd->extra.status_data = data;
	swkbd->initial_status_offset = (data&&in) ? 0 : -1;
	if (data&&out) swkbd->save_state_flags |= BIT(0);
	else           swkbd->save_state_flags &= ~BIT(0);
}

void swkbdSetLearningData(SwkbdState* swkbd, SwkbdLearningData* data, bool in, bool out)
{
	swkbd->extra.learning_data = data;
	swkbd->initial_learning_offset = (data&&in) ? 0 : -1;
	if (data&&out) swkbd->save_state_flags |= BIT(1);
	else           swkbd->save_state_flags &= ~BIT(1);
}

void swkbdSetFilterCallback(SwkbdState* swkbd, SwkbdCallbackFn callback, void* user)
{
	swkbd->extra.callback = callback;
	swkbd->extra.callback_user = callback ? user : NULL;
}

static void swkbdMessageCallback(void* user, NS_APPID sender, void* msg, size_t msgsize)
{
	SwkbdExtra* extra = (SwkbdExtra*)user;
	SwkbdState* swkbd = (SwkbdState*)msg;

	if (sender != APPID_SOFTWARE_KEYBOARD || msgsize != sizeof(SwkbdState))
		return;

	u16* text16 = (u16*)(swkbdSharedMem + swkbd->text_offset);
	ssize_t units = utf16_to_utf8(NULL, text16, 0);
	if (units < 0) svcBreak(USERBREAK_PANIC); // Shouldn't happen.
	char* text8 = (char*)malloc(units+1);
	if (!text8) svcBreak(USERBREAK_PANIC); // Shouldn't happen.
	swkbdConvertToUTF8(text8, text16, units);

	const char* retmsg = NULL;
	swkbd->callback_result = extra->callback(extra->callback_user, &retmsg, text8, units);
	if (swkbd->callback_result > SWKBD_CALLBACK_OK)
		swkbdConvertToUTF16(swkbd->callback_msg, retmsg, SWKBD_MAX_CALLBACK_MSG_LEN);
	else
		swkbd->callback_msg[0] = 0;

	free(text8);
	APT_SendParameter(envGetAptAppId(), sender, APTCMD_MESSAGE, swkbd, sizeof(*swkbd), 0);
}

SwkbdButton swkbdInputText(SwkbdState* swkbd, char* buf, size_t bufsize)
{
	SwkbdExtra extra = swkbd->extra; // Struct copy

	// Calculate sharedmem size
	size_t sharedMemSize = 0;
	sharedMemSize += (sizeof(u16)*(swkbd->max_text_len+1) + 3) &~ 3;
	size_t dictOff = sharedMemSize;
	sharedMemSize += (sizeof(SwkbdDictWord)*swkbd->dict_word_count + 3) &~ 3;
	size_t statusOff = sharedMemSize;
	sharedMemSize += swkbd->initial_status_offset >= 0 ? sizeof(SwkbdStatusData) : 0;
	size_t learningOff = sharedMemSize;
	sharedMemSize += swkbd->initial_learning_offset >= 0 ? sizeof(SwkbdLearningData) : 0;
	if (swkbd->save_state_flags & BIT(0))
	{
		swkbd->status_offset = sharedMemSize;
		sharedMemSize += sizeof(SwkbdStatusData);
	}
	if (swkbd->save_state_flags & BIT(1))
	{
		swkbd->learning_offset = sharedMemSize;
		sharedMemSize += sizeof(SwkbdLearningData);
	}
	sharedMemSize  = (sharedMemSize + 0xFFF) &~ 0xFFF;
	swkbd->shared_memory_size = sharedMemSize;

	// Allocate sharedmem
	swkbdSharedMem = (char*)memalign(0x1000, sharedMemSize);
	if (!swkbdSharedMem)
	{
		swkbd->result = SWKBD_OUTOFMEM;
		return SWKBD_BUTTON_NONE;
	}

	// Create sharedmem block
	Result res = svcCreateMemoryBlock(&swkbdSharedMemHandle, (u32)swkbdSharedMem, sharedMemSize, MEMPERM_READ|MEMPERM_WRITE, MEMPERM_READ|MEMPERM_WRITE);
	if (R_FAILED(res))
	{
		free(swkbdSharedMem);
		swkbd->result = SWKBD_OUTOFMEM;
		return SWKBD_BUTTON_NONE;
	}

	// Copy stuff to shared mem
	if (extra.initial_text)
	{
		swkbd->initial_text_offset = 0;
		swkbdConvertToUTF16((u16*)swkbdSharedMem, extra.initial_text, swkbd->max_text_len);
	}
	if (extra.dict)
	{
		swkbd->dict_offset = dictOff;
		memcpy(swkbdSharedMem+dictOff, extra.dict, sizeof(SwkbdDictWord)*swkbd->dict_word_count);
	}
	if (swkbd->initial_status_offset >= 0)
	{
		swkbd->initial_status_offset = statusOff;
		memcpy(swkbdSharedMem+statusOff, extra.status_data, sizeof(SwkbdStatusData));
	}
	if (swkbd->initial_learning_offset >= 0)
	{
		swkbd->initial_learning_offset = learningOff;
		memcpy(swkbdSharedMem+learningOff, extra.learning_data, sizeof(SwkbdLearningData));
	}

	if (extra.callback) swkbd->filter_flags |= SWKBD_FILTER_CALLBACK;
	else                swkbd->filter_flags &= ~SWKBD_FILTER_CALLBACK;

	// Launch swkbd
	memset(swkbd->reserved, 0, sizeof(swkbd->reserved));
	if (extra.callback) aptSetMessageCallback(swkbdMessageCallback, &extra);
	bool ret = aptLaunchLibraryApplet(APPID_SOFTWARE_KEYBOARD, swkbd, sizeof(*swkbd), swkbdSharedMemHandle);
	if (extra.callback) aptSetMessageCallback(NULL, NULL);
	svcCloseHandle(swkbdSharedMemHandle);

	SwkbdButton button = SWKBD_BUTTON_NONE;

	if (ret)
	{
		u16* text16 = (u16*)(swkbdSharedMem+swkbd->text_offset);
		text16[swkbd->text_length] = 0;
		swkbdConvertToUTF8(buf, text16, bufsize-1);
		if (swkbd->save_state_flags & BIT(0)) memcpy(extra.status_data, swkbdSharedMem+swkbd->status_offset, sizeof(SwkbdStatusData));
		if (swkbd->save_state_flags & BIT(1)) memcpy(extra.learning_data, swkbdSharedMem+swkbd->learning_offset, sizeof(SwkbdLearningData));

		switch (swkbd->result)
		{
			case SWKBD_D1_CLICK0:
			case SWKBD_D2_CLICK0:
				button = SWKBD_BUTTON_LEFT;
				break;
			case SWKBD_D2_CLICK1:
				button = SWKBD_BUTTON_MIDDLE;
				break;
			case SWKBD_D0_CLICK:
			case SWKBD_D1_CLICK1:
			case SWKBD_D2_CLICK2:
				button = SWKBD_BUTTON_RIGHT;
				break;
			default:
				break;
		}
	}

	free(swkbdSharedMem);
	return button;
}
