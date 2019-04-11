#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/services/apt.h>
#include <3ds/util/utf.h>

#include <3ds/applets/miiselector.h>

#include <string.h> // for memcpy

Result miiSelectorLaunch(const MiiSelectorConf *conf, MiiSelectorReturn *returnbuf)
{
	union {
		MiiSelectorConf config;
		MiiSelectorReturn ret;
	} ctx;

	memcpy(&ctx.config, conf, sizeof(MiiSelectorConf));
	ctx.config.magic = MIISELECTOR_MAGIC;

	Result ret = aptLaunchLibraryApplet(APPID_APPLETED, &ctx.config, sizeof(MiiSelectorConf), 0);
	if(R_SUCCEEDED(ret) && returnbuf)
		memcpy(returnbuf, &ctx.ret, sizeof(MiiSelectorReturn));

	return ret;
}

static void miiSelectorConvertToUTF8(char* out, const u16* in, int max)
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

static void miiSelectorConvertToUTF16(u16* out, const char* in, int max)
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

void miiSelectorSetTitle(MiiSelectorConf *conf, const char* text)
{
	miiSelectorConvertToUTF16(conf->title, text, MIISELECTOR_TITLE_LEN);
}

static const char miiSelectorOptions[] =
{
	offsetof(MiiSelectorConf, enable_cancel_button),
	offsetof(MiiSelectorConf, enable_selecting_guests),
	offsetof(MiiSelectorConf, show_on_top_screen),
	offsetof(MiiSelectorConf, show_guest_page),
};

void miiSelectorSetOptions(MiiSelectorConf *conf, u8 options)
{
	for (int i = 0; i < (sizeof(miiSelectorOptions)/sizeof(char)); i ++)
		*((u8*)conf + miiSelectorOptions[i]) = (options & BIT(i)) ? 1 : 0;
}

void miiSelectorWhitelistGuestMii(MiiSelectorConf *conf, u32 index)
{
	if (index < MIISELECTOR_GUESTMII_SLOTS)
		conf->mii_guest_whitelist[index] = 1;
	else if (index == MIISELECTOR_GUESTMII_SLOTS)
		for (int i = 0; i < MIISELECTOR_GUESTMII_SLOTS; i ++)
			conf->mii_guest_whitelist[i] = 1;
}

void miiSelectorBlacklistGuestMii(MiiSelectorConf *conf, u32 index)
{
	if (index < MIISELECTOR_GUESTMII_SLOTS)
		conf->mii_guest_whitelist[index] = 0;
	else if (index == MIISELECTOR_GUESTMII_SLOTS)
		for (int i = 0; i < MIISELECTOR_GUESTMII_SLOTS; i ++)
			conf->mii_guest_whitelist[i] = 0;
}

void miiSelectorWhitelistUserMii(MiiSelectorConf *conf, u32 index)
{
	if (index < MIISELECTOR_USERMII_SLOTS)
		conf->mii_whitelist[index] = 1;
	else if (index == MIISELECTOR_USERMII_SLOTS)
		for (int i = 0; i < MIISELECTOR_USERMII_SLOTS; i ++)
			conf->mii_whitelist[i] = 1;
}

void miiSelectorBlacklistUserMii(MiiSelectorConf *conf, u32 index)
{
	if (index < MIISELECTOR_USERMII_SLOTS)
		conf->mii_whitelist[index] = 0;
	else if (index == MIISELECTOR_USERMII_SLOTS)
		for (int i = 0; i < MIISELECTOR_USERMII_SLOTS; i ++)
			conf->mii_whitelist[i] = 0;
}

void miiSelectorSetInitalIndex(MiiSelectorConf *conf, u32 index) {
	conf->initial_index = index;
}

void miiSelectorReturnGetName(const MiiSelectorReturn *returnbuf, char* out)
{
	if (!out)
		return;

	if (returnbuf->guest_mii_was_selected)
		miiSelectorConvertToUTF8(out, returnbuf->guest_mii_name, 36);
	else
		miiSelectorConvertToUTF8(out, returnbuf->mii.mii_name, 36);
}

void miiSelectorReturnGetAuthor(const MiiSelectorReturn *returnbuf, char* out)
{
	miiSelectorConvertToUTF8(out, returnbuf->mii.author_name, 30);
}

static u16 crc16_ccitt(void const *buf, size_t len, uint32_t starting_val)
{
	if(buf == NULL)
		return -1;

	u8 const *cbuf = buf;
	u32 crc        = starting_val;

	static const u16 POLY = 0x1021;

	for(size_t i = 0; i < len; i++)
	{
		for(int bit = 7; bit >= 0; bit--)
			crc = ((crc << 1) | ((cbuf[i] >> bit) & 0x1)) ^ (crc & 0x8000 ? POLY : 0);
	}

	for(int _ = 0; _ < 16; _++)
		crc = (crc << 1) ^ (crc & 0x8000 ? POLY : 0);

	return (u16)(crc & 0xffff);
}

bool miiSelectorChecksumIsValid(const MiiSelectorReturn *returnbuf)
{
	u16 computed =
	    crc16_ccitt(&returnbuf->mii, sizeof(returnbuf->mii) + sizeof(returnbuf->_pad0x68), 0x0000);
	u16 chk_little_endian = __builtin_bswap16(returnbuf->checksum);
	return computed == chk_little_endian;
}
