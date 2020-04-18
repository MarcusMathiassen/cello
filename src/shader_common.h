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
using simd::fract;
using simd::round;
using simd::floor;
using simd::fmod;
using simd::fabs;
using simd::sign;
using simd::mix;

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

#define METAL_INTERNAL static inline

#ifndef FLT_MIN
#define FLT_MIN 1.175494e-38
#endif
#ifndef FLT_MAX
#define FLT_MAX 3.402823e+38
#endif
#ifndef FLT_EPSILON
#define FLT_EPSILON 1.192093e-07
#endif

#ifdef __METAL__
    #define METAL(x) #x
#else
    #define METAL(x)
#endif

#ifndef __METAL__
#define clamp(x,a,b) simd_clamp(x,a,b)
#define saturate(x) clamp(x, 0.0f, 1.0f)
#endif

template <class T, class A>
struct texture3d
{
    T* texels;
    A  access;
    simd::ushort3 size;

    simd::ushort3 index(simd::ushort3 uvw) { return uvw.x + size.x * (uvw.y + size.z * uvw.z); }
    simd::float3 sample(simd::float3 uvw) { return texels[index(uvw)]; }
    void write(simd::float4 data, simd::ushort3 uvw) { texels[index(uvw)] = data; }
};

enum OpKind
{
  SET_SIZE,
  SET_MATERIAL_ID,
  SET_ROUNDING,

  OP_UNION,
  OP_SUBTRACT,
  OP_INTERSECT,

  OP_SMOOTH_UNION,
  OP_SMOOTH_SUBTRACT,
  OP_SMOOTH_INTERSECT,

  OP_PLACE_NEXT_OBJECT_AT_MOUSE_HIT,
  OP_ROUNDED,
  OP_ANNULAR,
  OP_REP,
  OP_ROTATE_X,
  OP_ROTATE_Y,
  OP_ROTATE_Z,
  OP_RESET,
  _NONE_,

  SD_PLANE,
  SD_SPHERE,
  SD_BOX,
  SD_ROUND_BOX,
  SD_TORUS,
  SD_CAPPED_CYLINDER,
  
  SD_TRIANGLE,
};


struct Edit
{
  OpKind kind;
  v3 data;
};

#define MAX_EDITS ((1024 * 4-sizeof(s8)) / sizeof(Edit))
struct Edit_Info
{
  s8 count;
  Edit edits[MAX_EDITS];
};

enum MaterialKind { DIFF, SPEC, REFR };
struct Material
{
  v3 color;
  MaterialKind kind;
  f32 emission;
  f32 roughness;
  f32 metallic;
};

struct Light
{
  v3 pos;
  v3 color;
  f32 intensity;
};

#define MAX_LIGHTS ((1024 * 4-sizeof(s8)) / sizeof(Light))
struct Light_Info
{
  s8 count;
  Light lights[MAX_LIGHTS];
};

struct Scene
{
    METAL(constant) Edit_Info& edit_info;
};


typedef struct
{
    v3 camera_position;
    v3 camera_target;
    f32 camera_zoom;
    mat3 camera_matrix;
    ushort2 viewport_size;
} Uniform;

#endif /* _SHADER_TYPES_H_ */
