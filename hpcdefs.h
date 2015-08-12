#pragma once

#if defined(__GNUC__)
	// gcc or gcc-compatible compiler
	#define CSE6230_RESTRICT __restrict__
#else
	#warning Compiler is not recognized and restrict qualifier is not used.
	#define CSE6230_RESTRICT
#endif

#define CSE6230_ESCAPE_NORMAL_COLOR "\x1B[0m"
#define CSE6230_ESCAPE_RED_COLOR "\x1B[31m"
#define CSE6230_ESCAPE_GREEN_COLOR "\x1B[32m"

#if defined(__GNUC__)
	#if defined(__MMX__)
		#define CSE6230_MMX_INTRINSICS_SUPPORTED
	#endif
	#if defined(__SSE__)
		#define CSE6230_SSE_INTRINSICS_SUPPORTED
	#endif
	#if defined(__SSE2__)
		#define CSE6230_SSE2_INTRINSICS_SUPPORTED
	#endif
	#if defined(__SSE3__)
		#define CSE6230_SSE3_INTRINSICS_SUPPORTED
	#endif
	#if defined(__SSSE3__)
		#define CSE6230_SSSE3_INTRINSICS_SUPPORTED
	#endif
	#if defined(__SSE4A__)
		#define CSE6230_SSE4A_INTRINSICS_SUPPORTED
	#endif
	#if defined(__SSE4_1__)
		#define CSE6230_SSE4_1_INTRINSICS_SUPPORTED
	#endif
	#if defined(__SSE4_2__)
		#define CSE6230_SSE4_2_INTRINSICS_SUPPORTED
	#endif
	#if defined(__AVX__)
		#define CSE6230_AVX_INTRINSICS_SUPPORTED
	#endif
	#if defined(__FMA4__) || defined(__FMA3__)
		#define CSE6230_FMA_INTRINSICS_SUPPORTED
	#endif
	#if defined(__XOP__)
		#define CSE6230_XOP_INTRINSICS_SUPPORTED
	#endif
	#if defined(__x86_64__)
		#include <x86intrin.h>
	#else
		#error Unsupported architecture
	#endif
#else
	#warning Compiler is not recognized and intrinsic functions are not used.
#endif

#include <stddef.h>
#include <stdint.h>
#include <malloc.h>

inline static uint64_t get_cpu_ticks_acquire() {
	#if defined(__GNUC__)
		#ifdef __x86_64__
			uint32_t low, high;
			__asm__ __volatile__ (
				"xor %%eax, %%eax;"
				"cpuid;"
				"rdtsc;"
			: "=a"(low), "=d"(high)
			:
			: "%rbx", "%rcx"
			);
			return (uint64_t(high) << 32) | uint64_t(low);
		#endif
	#else
		#error Unsupported compiler
	#endif
}

inline static uint64_t get_cpu_ticks_release() {
	#if defined(__GNUC__)
		#ifdef __x86_64__
			uint32_t low, high;
			#ifdef CSE6230_USE_RDTSCP
				asm volatile (
					"rdtscp;"
				: "=a"(low), "=d"(high)
				:
				: "%rcx"
				);
			#else
				__asm__ __volatile__ (
					"xor %%eax, %%eax;"
					"cpuid;"
					"rdtsc;"
				: "=a"(low), "=d"(high)
				:
				: "%rbx", "%rcx"
				);
			#endif
			return (uint64_t(high) << 32) | uint64_t(low);
		#else
			#error Unsupported architecture
		#endif
	#else
		#error Unsupported compiler
	#endif
}

inline static void* allocate_aligned_memory(size_t allocation_size, size_t alignment) {
	return memalign(alignment, allocation_size);
}

inline static void release_aligned_memory(void* memory_pointer) {
	free(memory_pointer);
}
