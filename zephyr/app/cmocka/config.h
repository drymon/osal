/* Minimal cmocka build configuration for Zephyr native_sim (host libc). */
#ifndef DMOSAL_CMOCKA_CONFIG_H
#define DMOSAL_CMOCKA_CONFIG_H

#define HAVE_ASSERT_H				  1
#define HAVE_INTTYPES_H				  1
#define HAVE_MALLOC_H				  1
#define HAVE_SETJMP_H				  1
#define HAVE_SIGNAL_H				  1
#define HAVE_STDARG_H				  1
#define HAVE_STDDEF_H				  1
#define HAVE_STDINT_H				  1
#define HAVE_STDIO_H				  1
#define HAVE_STDLIB_H				  1
#define HAVE_STRING_H				  1
#define HAVE_TIME_H					  1
#define HAVE_STRUCT_TIMESPEC		  1
#define HAVE_UINTPTR_T				  1
#define HAVE_CALLOC					  1
#define HAVE_EXIT					  1
#define HAVE_FPRINTF				  1
#define HAVE_SNPRINTF				  1
#define HAVE_VSNPRINTF				  1
#define HAVE_FREE					  1
#define HAVE_LONGJMP				  1
#define HAVE_MALLOC					  1
#define HAVE_MEMCPY					  1
#define HAVE_MEMSET					  1
#define HAVE_PRINTF					  1
#define HAVE_SETJMP					  1
#define HAVE_SIGNAL					  1
#define HAVE_STRCMP					  1
#define HAVE_STRCPY					  1
#define HAVE_GCC_THREAD_LOCAL_STORAGE 1
#define HAVE_CLOCK_GETTIME			  1
#define HAVE_CLOCK_REALTIME			  1
#define WORDS_SIZEOF_VOID_P			  8

#endif
