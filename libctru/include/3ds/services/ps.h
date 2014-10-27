#pragma once

typedef enum
{
	ps_CBC_ENC,
	ps_CBC_DEC,
	ps_CTR_ENC,
	ps_CTR_DEC,
	ps_CCM_ENC,
	ps_CCM_DEC,
} ps_aes_algo;

typedef enum
{
	ps_KEYSLOT_0D,
	ps_KEYSLOT_2D,
	ps_KEYSLOT_31,
	ps_KEYSLOT_38,
	ps_KEYSLOT_32,
	ps_KEYSLOT_39,
	ps_KEYSLOT_2E,
	ps_KEYSLOT_INVALID,
	ps_KEYSLOT_36
} ps_aes_keytypes;

/*
	Requires access to "ps:ps" service
*/

Result psInit();
Result psExit();

/* PS_EncryptDecryptAes()
About: Is an interface for the AES Engine, you can only use predetermined keyslots though.
Note: Does not support AES CCM, see PS_EncryptSignDecryptVerifyAesCcm()

  size			size of data
  in			input buffer ptr
  out			output buffer ptr
  aes_algo		AES Algorithm to use, see ps_aes_algo
  key_type		see ps_aes_keytypes
  iv			ptr to the CTR/IV (This is updated before returning)
*/
Result PS_EncryptDecryptAes(u32 size, u8* in, u8* out, u32 aes_algo, u32 key_type, u8* iv);

/* PS_EncryptSignDecryptVerifyAesCcm()
About: Is an interface for the AES Engine (CCM Encrypt/Decrypt only), you can only use predetermined keyslots though.
Note: When encrypting, the output buffer size must include the MAC size, when decrypting, the input buffer size must include MAC size.
MAC: When decrypting, if the MAC is invalid, 0xC9010401 is returned. After encrypting the MAC is located at inputbufptr+(totalassocdata+totaldatasize)

  in			input buffer ptr
  in_size		size of input buffer
  out			output buffer ptr
  out_size		size of output buffer
  data_len		length of data to be crypted
  mac_data_len	length of data associated with MAC
  mac_len		length of MAC
  aes_algo		AES Algorithm to use, see ps_aes_algo
  key_type		see ps_aes_keytypes
  nonce			ptr to the nonce
*/
Result PS_EncryptSignDecryptVerifyAesCcm(u8* in, u32 in_size, u8* out, u32 out_size, u32 data_len, u32 mac_data_len, u32 mac_len, u32 aes_algo, u32 key_type, u8* nonce);

/* PS_GetLocalFriendCodeSeed()
About: Gets a 64bit console id, it's used for some key slot inits

  seed			ptr to where the seed is written to
*/
Result PS_GetLocalFriendCodeSeed(u64* seed);

/* PS_GetDeviceId()
About: Gets a 32bit device id, it's used for some key slot inits

  device_id		ptr to where the device id is written to
*/
Result PS_GetDeviceId(u32* device_id);