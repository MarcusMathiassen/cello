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

#ifndef _SHADER_TYPES_H_
#define _SHADER_TYPES_H_

#ifdef __METAL_VERSION__

#else

#define v2(x,y) make_float2(x,y)
#define v3(x,y,z) make_float3(x,y,z)
#define ushort2(x,y) make_ushort2(x,y)

#endif

#include <stdint.h>
#include <simd/simd.h>

typedef simd::float2 v2;
typedef simd::float3 v3;
typedef simd::float4 v4;

using simd::ushort2;
using simd::make_ushort2;
using simd::make_float2;
using simd::make_float3;
using simd::make_float4;
using simd::dot;
using simd::cross;
using simd::normalize;
using simd::length;
using simd::max;
using simd::min;
using simd::exp2;
using simd::distance;
using simd::pow;

// typedef vector_half2 h2;
// typedef vector_half3 h3;
// typedef vector_half4 h4;

typedef simd::float2x2 mat2;
typedef simd::float3x3 mat3;
typedef simd::float4x4 mat4;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

// typedef half f16;
typedef float f32;

typedef struct {
    f32 t;
    s16 material_id;
    s16 steps;
} Hit;

typedef struct {
  v3 pos;
  v3 color;
  f32 intensity;
} Light;

#define MAX_LIGHTS ((1024 * 4-sizeof(s8)) / sizeof(Light))
typedef struct {
  s8 count;
  Light lights[MAX_LIGHTS];
} Light_Info;

typedef struct
{
    v3 camera_position;
    v3 camera_target;
    f32 camera_zoom;
    mat3 camera_matrix;
    ushort2 viewport_size;
} Uniform;

#endif /* _SHADER_TYPES_H_ */
