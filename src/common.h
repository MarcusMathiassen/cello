// Copyright (c) 2020 Marcus Mathiassen

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#define RESET "\033[0m"
#define GRAY "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RGB_GRAY "\033[38;2;110;110;110;m"
#define RGB_WHITE "\033[38;2;255;255;255;m"

#define foreach(i, c) for (s64 (i) = 0; (i) < (c); ++(i))
#define foreach_reverse(i, c) for (s64 (i) = c-1; (i) >= 0; --(i))

#define internal static
#define global_variable static
#define local_persists static

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;
typedef s8 b8;
typedef s16 b16;
typedef s32 b32;
typedef s64 b64;

#define BYTES(n) (n)
#define KILOBYTES(n) (BYTES(n) * 1024ULL)
#define MEGABYTES(n) (KILOBYTES(n) * 1024ULL)
#define GIGABYTES(n) (MEGABYTES(n) * 1024ULL)
#define TERABYTES(n) (GIGABYTES(n) * 1024ULL)

#define TO_NANOSECONDS(n) (n)
#define TO_MICROSECONDS(n) ((u64)TO_NANOSECONDS(n) / 1000ULL)
#define TO_MILLISECONDS(n) ((u64)TO_MICROSECONDS(n) / 1000ULL)
#define TO_SECONDS(n) ((u64)TO_MILLISECONDS(n) / 1000ULL)
#define TO_MINUTES(n) ((u64)TO_SECONDS(n) / 60ULL)
#define TO_HOURS(n) ((u64)TO_MINUTES(n) / 60ULL)

#define NANOSECONDS(n) (n)
#define MICROSECONDS(n) ((u64)NANOSECONDS(n) * 1000ULL)
#define MILLISECONDS(n) ((u64)MICROSECONDS(n) * 1000ULL)
#define SECONDS(n) ((u64)MILLISECONDS(n) * 1000ULL)
#define MINUTES(n) ((u64)SECONDS(n) * 60ULL)
#define HOURS(n) ((u64)MINUTES(n) * 60ULL)

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#define DEFAULT_GIGABYTE_SUFFIX "gb"
#define DEFAULT_MEGABYTE_SUFFIX "mb"
#define DEFAULT_KILOBYTE_SUFFIX "kb"
#define DEFAULT_BYTE_SUFFIX "b"

#define DEFAULT_HOURS_SUFFIX "h"
#define DEFAULT_MINUTES_SUFFIX "m"
#define DEFAULT_SECONDS_SUFFIX "s"
#define DEFAULT_MILLISECONDS_SUFFIX "ms"
#define DEFAULT_MICROSECONDS_SUFFIX "us"
#define DEFAULT_NANOSECONDS_SUFFIX "ns"

#define UNREACHABLE assert(0)
#define UNFINISHED  assert(0)

#endif
