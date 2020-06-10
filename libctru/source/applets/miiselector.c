#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/services/apt.h>
#include <3ds/util/utf.h>

#include <3ds/applets/miiselector.h>

#include <string.h> // for memcpy

void miiSelectorInit(MiiSelectorConf *conf)
{
	memset(conf, 0, sizeof(*conf));

	for (int i = 0; i < MIISELECTOR_GUESTMII_SLOTS; i ++)
		conf->mii_guest_whitelist[i] = 1;

	for (int i = 0; i < MIISELECTOR_USERMII_SLOTS; i ++)
		conf->mii_whitelist[i] = 1;
}

void miiSelectorLaunch(const MiiSelectorConf *conf, MiiSelectorReturn *returnbuf)
{
	union {
		MiiSelectorConf config;
		MiiSelectorReturn ret;
	} ctx;

	memcpy(&ctx.config, conf, sizeof(MiiSelectorConf));
	ctx.config.magic = MIISELECTOR_MAGIC;

	aptLaunchLibraryApplet(APPID_APPLETED, &ctx.config, sizeof(MiiSelectorConf), 0);
	if(returnbuf)
		memcpy(returnbuf, &ctx.ret, sizeof(MiiSelectorReturn));
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

void miiSelectorSetOptions(MiiSelectorConf *conf, u32 options)
{
	static const u8 miiSelectorOptions[] =
	{
		offsetof(MiiSelectorConf, enable_cancel_button),
		offsetof(MiiSelectorConf, enable_selecting_guests),
		offsetof(MiiSelectorConf, show_on_top_screen),
		offsetof(MiiSelectorConf, show_guest_page),
	};
	for (int i = 0; i < sizeof(miiSelectorOptions); i ++)
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

void miiSelectorReturnGetName(const MiiSelectorReturn *returnbuf, char* out, size_t max_size)
{
	if (!out)
		return;

	if (returnbuf->guest_mii_was_selected)
		miiSelectorConvertToUTF8(out, returnbuf->guest_mii_name, max_size);
	else
	{
		u16 temp[10];
		memcpy(temp, returnbuf->mii.mii_name, sizeof(temp));

		miiSelectorConvertToUTF8(out, temp, max_size);
	}
}

void miiSelectorReturnGetAuthor(const MiiSelectorReturn *returnbuf, char* out, size_t max_size)
{
	if (!out)
		return;

	u16 temp[10];
	memcpy(temp, returnbuf->mii.author_name, sizeof(temp));

	miiSelectorConvertToUTF8(out, temp, max_size);
}

static u16 crc16_ccitt(void const *buf, size_t len, uint32_t starting_val)
{
	if (!buf)
		return -1;

	u8 const *cbuf = buf;
	u32 crc        = starting_val;

	static const u16 POLY = 0x1021;

	for (size_t i = 0; i < len; i++)
	{
		for (int bit = 7; bit >= 0; bit--)
			crc = ((crc << 1) | ((cbuf[i] >> bit) & 0x1)) ^ (crc & 0x8000 ? POLY : 0);
	}

	for (int _ = 0; _ < 16; _++)
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
