#pragma once

#if !defined(__ORDER_LITTLE_ENDIAN__)
#define __ORDER_LITTLE_ENDIAN__ PLATFORM_LITTLE_ENDIAN
#endif

#pragma warning( push )
#pragma warning( disable : 4334)

#if PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#endif

#undef malloc
#undef free
#undef realloc
#undef memset
#undef memcpy

#define malloc(Count)				FMemory::Malloc(Count)
#define free(Original)				FMemory::Free(Original)
#define realloc(Original, Count)	FMemory::Realloc(Original, Count)
#define memset(Dest, Char, Count)	FMemory::Memset(Dest, Char, Count)
#define memcpy(Dest, Source, Count)	FMemory::Memcpy(Dest, Source, Count)

THIRD_PARTY_INCLUDES_START
#include "miniz.h"
#include "miniz.c"
THIRD_PARTY_INCLUDES_END

#undef malloc
#undef free
#undef realloc
#undef memset
#undef memcpy

#pragma warning( pop )