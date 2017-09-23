#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/services/apt.h>

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
	if(R_SUCCEEDED(ret) && returnbuf) {
		memcpy(returnbuf, &ctx.ret, sizeof(MiiSelectorReturn));
	}

	return ret;
}

static u16 crc16_ccitt(void const *buf, size_t len, uint32_t starting_val)
{
	if(buf == NULL) {
		return -1;
	}

	u8 const *cbuf = buf;
	u32 crc        = starting_val;

	static const u16 POLY = 0x1021;

	for(size_t i = 0; i < len; i++) {
		for(int bit = 7; bit >= 0; bit--) {
			crc = ((crc << 1) | ((cbuf[i] >> bit) & 0x1)) ^ (crc & 0x8000 ? POLY : 0);
		}
	}

	for(int _ = 0; _ < 16; _++) {
		crc = (crc << 1) ^ (crc & 0x8000 ? POLY : 0);
	}

	return (u16)(crc & 0xffff);
}

bool miiSelectorChecksumIsValid(const MiiSelectorReturn *returnbuf)
{
	u16 computed =
	    crc16_ccitt(&returnbuf->mii, sizeof(returnbuf->mii) + sizeof(returnbuf->_pad0x68), 0x0000);
	u16 chk_little_endian = __builtin_bswap16(returnbuf->checksum);
	return computed == chk_little_endian;
}
