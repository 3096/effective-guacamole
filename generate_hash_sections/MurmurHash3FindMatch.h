//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// This specific version of MurmurHash3 32 is modified by 3096 to identify
// hash matches to find the sections hashed in Animal Crossing: New Horizon
// save files.

#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER) && (_MSC_VER < 1600)

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

// Other compilers

#else  // defined(_MSC_VER)

#    include <stdint.h>

#endif  // !defined(_MSC_VER)

//-----------------------------------------------------------------------------

#include <stddef.h>

int MurmurHash3_32_FindMatch(const void* key, int maxlen, uint32_t seed,
                             uint32_t target);

//-----------------------------------------------------------------------------

#endif  // _MURMURHASH3_H_
