// Georgy Treshchev 2024.

#pragma once

#include "CoreTypes.h"

#if !defined(__ORDER_LITTLE_ENDIAN__)
#define __ORDER_LITTLE_ENDIAN__ PLATFORM_LITTLE_ENDIAN
#endif

#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES PLATFORM_SUPPORTS_UNALIGNED_LOADS
#define MINIZ_LITTLE_ENDIAN PLATFORM_LITTLE_ENDIAN
#define MINIZ_HAS_64BIT_REGISTERS PLATFORM_64BITS

#define _LARGEFILE64_SOURCE

#pragma warning( push )
#pragma warning( disable : 4334)

#if PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include "Windows/AllowWindowsPlatformTypes.h"
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
#include "miniz_export.h"
#include "miniz.c"
#include "miniz_zip.c"
#include "miniz_tinfl.c"
#include "miniz_tdef.c"
THIRD_PARTY_INCLUDES_END

#undef malloc
#undef free
#undef realloc
#undef memset
#undef memcpy

#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#pragma warning( pop )