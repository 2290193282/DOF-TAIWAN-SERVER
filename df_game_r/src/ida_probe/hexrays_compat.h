#ifndef OLDSRV_REBUILD_IDA_PROBE_HEXRAYS_COMPAT_H
#define OLDSRV_REBUILD_IDA_PROBE_HEXRAYS_COMPAT_H

// Compile-probe only compatibility layer for raw Hex-Rays pseudocode.
// This header must stay small: it removes compiler dialect noise, but it must
// not invent old df_game_r business classes, globals, or packet behavior.

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#ifndef _MSC_VER
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __stdcall
#define __stdcall
#endif
#ifndef __fastcall
#define __fastcall
#endif
#ifndef __thiscall
#define __thiscall
#endif
#ifndef __int8
#define __int8 char
#endif
#ifndef __int16
#define __int16 short
#endif
#ifndef __int32
#define __int32 int
#endif
#ifndef __int64
#define __int64 long long
#endif
#endif

#ifndef BYREF
#define BYREF
#endif

typedef uint8_t _BYTE;
typedef int8_t _SBYTE;
typedef uint16_t _WORD;
typedef int16_t _SWORD;
typedef uint32_t _DWORD;
typedef int32_t _SDWORD;
typedef uint64_t _QWORD;
typedef int64_t _SQWORD;
typedef uintptr_t _UNKNOWN;

typedef uint8_t _BOOL1;
typedef uint16_t _BOOL2;
typedef uint32_t _BOOL4;

#endif  // OLDSRV_REBUILD_IDA_PROBE_HEXRAYS_COMPAT_H
