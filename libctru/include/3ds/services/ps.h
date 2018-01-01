/**
 * @file ps.h
 * @brief PS service.
 */
#pragma once

/// PS AES algorithms.
typedef enum
{
	PS_ALGORITHM_CBC_ENC, ///< CBC encryption.
	PS_ALGORITHM_CBC_DEC, ///< CBC decryption.
	PS_ALGORITHM_CTR_ENC, ///< CTR encryption.
	PS_ALGORITHM_CTR_DEC, ///< CTR decryption(same as PS_ALGORITHM_CTR_ENC).
	PS_ALGORITHM_CCM_ENC, ///< CCM encryption.
	PS_ALGORITHM_CCM_DEC, ///< CCM decryption.
} PS_AESAlgorithm;

/// PS key slots.
typedef enum
{
	PS_KEYSLOT_0D,      ///< Key slot 0x0D.
	PS_KEYSLOT_2D,      ///< Key slot 0x2D.
	PS_KEYSLOT_31,      ///< Key slot 0x31.
	PS_KEYSLOT_38,      ///< Key slot 0x38.
	PS_KEYSLOT_32,      ///< Key slot 0x32.
	PS_KEYSLOT_39_DLP,  ///< Key slot 0x39. (DLP)
	PS_KEYSLOT_2E,      ///< Key slot 0x2E.
	PS_KEYSLOT_INVALID, ///< Invalid key slot.
	PS_KEYSLOT_36,      ///< Key slot 0x36.
	PS_KEYSLOT_39_NFC   ///< Key slot 0x39. (NFC)
} PS_AESKeyType;

/// RSA context.
typedef struct {
	u8 modulo[0x100];
	u8 exponent[0x100];
	u32 rsa_bitsize;//The signature byte size is rsa_bitsize>>3.
	u32 unk;//Normally zero?
} psRSAContext;

/// Initializes PS.
Result psInit(void);

/**
 * @brief Initializes PS with the specified session handle.
 * @param handle Session handle.
 */
Result psInitHandle(Handle handle);

/// Exits PS.
void psExit(void);

/// Returns the PS session handle.
Handle psGetSessionHandle(void);

/**
 * @brief Signs a RSA signature.
 * @param hash SHA256 hash to sign.
 * @param ctx RSA context.
 * @param signature RSA signature.
 */
Result PS_SignRsaSha256(u8 *hash, psRSAContext *ctx, u8 *signature);

/**
 * @brief Verifies a RSA signature.
 * @param hash SHA256 hash to compare with.
 * @param ctx RSA context.
 * @param signature RSA signature.
 */
Result PS_VerifyRsaSha256(u8 *hash, psRSAContext *ctx, u8 *signature);

/**
 * @brief Encrypts/Decrypts AES data. Does not support AES CCM.
 * @param size Size of the data.
 * @param in Input buffer.
 * @param out Output buffer.
 * @param aes_algo AES algorithm to use.
 * @param key_type Key type to use.
 * @param iv Pointer to the CTR/IV. The output CTR/IV is also written here.
 */
Result PS_EncryptDecryptAes(u32 size, u8* in, u8* out, PS_AESAlgorithm aes_algo, PS_AESKeyType key_type, u8* iv);

/**
 * @brief Encrypts/Decrypts signed AES CCM data.
 * When decrypting, if the MAC is invalid, 0xC9010401 is returned. After encrypting the MAC is located at inputbufptr.
 * @param in Input buffer.
 * @param in_size Size of the input buffer. Must include MAC size when decrypting.
 * @param out Output buffer.
 * @param out_size Size of the output buffer. Must include MAC size when encrypting.
 * @param data_len Length of the data to be encrypted/decrypted.
 * @param mac_data_len Length of the MAC data.
 * @param mac_len Length of the MAC.
 * @param aes_algo AES algorithm to use.
 * @param key_type Key type to use.
 * @param nonce Pointer to the nonce.
 */
Result PS_EncryptSignDecryptVerifyAesCcm(u8* in, u32 in_size, u8* out, u32 out_size, u32 data_len, u32 mac_data_len, u32 mac_len, PS_AESAlgorithm aes_algo, PS_AESKeyType key_type, u8* nonce);

/**
 * @brief Gets the 64-bit console friend code seed.
 * @param seed Pointer to write the friend code seed to.
 */
Result PS_GetLocalFriendCodeSeed(u64* seed);

/**
 * @brief Gets the 32-bit device ID.
 * @param device_id Pointer to write the device ID to.
 */
Result PS_GetDeviceId(u32* device_id);

/**
 * @brief Generates cryptographically secure random bytes.
 * @param out Pointer to the buffer to write the bytes to.
 * @param len Number of bytes to write.
 */
Result PS_GenerateRandomBytes(void* out, size_t len);
