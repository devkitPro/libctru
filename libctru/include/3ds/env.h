/**
 * @file env.h
 * @brief Homebrew environment information.
 */
#pragma once

/// System run-flags.
enum {
	RUNFLAG_APTWORKAROUND = BIT(0), ///< Use APT workaround.
	RUNFLAG_APTREINIT     = BIT(1), ///< Reinitialize APT.
	RUNFLAG_APTCHAINLOAD  = BIT(2), ///< Chainload APT on return.
};

/**
 * @brief Gets whether the application was launched from a homebrew environment.
 * @return Whether the application was launched from a homebrew environment.
 */
static inline bool envIsHomebrew(void) {
	extern void* __service_ptr;
	return __service_ptr != NULL;
}

/**
 * @brief Retrieves a handle from the environment handle list.
 * @param name Name of the handle.
 * @return The retrieved handle.
 */
Handle envGetHandle(const char* name);

/**
 * @brief Gets the environment-recommended app ID to use with APT.
 * @return The APT app ID.
 */
static inline u32 envGetAptAppId(void) {
	extern u32 __apt_appid;
	return __apt_appid;
}

/**
 * @brief Gets the size of the application heap.
 * @return The application heap size.
 */
static inline u32 envGetHeapSize(void) {
	extern u32 __ctru_heap_size;
	return __ctru_heap_size;
}

/**
 * @brief Gets the size of the linear heap.
 * @return The linear heap size.
 */
static inline u32 envGetLinearHeapSize(void) {
	extern u32 __ctru_linear_heap_size;
	return __ctru_linear_heap_size;
}

/**
 * @brief Gets the environment argument list.
 * @return The argument list.
 */
static inline const char* envGetSystemArgList(void) {
	extern const char* __system_arglist;
	return __system_arglist;
}

/**
 * @brief Gets the environment run flags.
 * @return The run flags.
 */
static inline u32 envGetSystemRunFlags(void) {
	extern u32 __system_runflags;
	return __system_runflags;
}
