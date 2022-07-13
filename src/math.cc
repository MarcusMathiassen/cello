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

#include <math.h>

inline static f32 fract(f32 a) { return a - floor(a); }
inline static f32 min(f32 a, f32 b) { return a < b ? a : b; }
inline static f32 max(f32 a, f32 b) { return a > b ? a : b; }

typedef struct { f32 x,y; } v2;
inline static v2    v2_add       (v2 a, v2 b)        { return (v2) { a.x + b.x, a.y + b.y }; }
inline static v2    v2_add_scl   (v2 a, f32 b)       { return (v2) { a.x + b, a.y + b }; }
inline static v2    v2_sub       (v2 a, v2 b)        { return (v2) { a.x - b.x, a.y - b.y }; }
inline static v2    v2_sub_scl   (v2 a, f32 b)       { return (v2) { a.x - b, a.y - b }; }
inline static v2    v2_mul       (v2 a, v2 b)        { return (v2) { a.x * b.x, a.y * b.y }; }
inline static v2    v2_mul_scl   (v2 a, f32 b)       { return (v2) { a.x * b, a.y * b }; }
inline static v2    v2_div       (v2 a, v2 b)        { return (v2) { a.x / b.x, a.y / b.y }; }
inline static v2    v2_div_scl   (v2 a, f32 b)       { return (v2) { a.x / b, a.y / b }; }

typedef struct { f32 x,y,z; } v3;
inline static v3    v3_add       (v3 a, v3 b)        { return (v3) { a.x + b.x, a.y + b.y, a.z + b.z }; }
inline static v3    v3_add_scl   (v3 a, f32 b)       { return (v3) { a.x + b, a.y + b, a.z + b }; }
inline static v3    v3_sub       (v3 a, v3 b)        { return (v3) { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline static v3    v3_sub_scl   (v3 a, f32 b)       { return (v3) { a.x - b, a.y - b, a.z - b }; }
inline static v3    v3_mul       (v3 a, v3 b)        { return (v3) { a.x * b.x, a.y * b.y, a.z * b.z }; }
inline static v3    v3_mul_scl   (v3 a, f32 b)       { return (v3) { a.x * b, a.y * b, a.z * b }; }
inline static v3    v3_div       (v3 a, v3 b)        { return (v3) { a.x / b.x, a.y / b.y, a.z / b.z }; }
inline static v3    v3_div_scl   (v3 a, f32 b)       { return (v3) { a.x / b, a.y / b, a.z / b }; }
inline static v3    v3_max       (v3 a, v3 b)        { return (v3) { max(a.x, b.x), max(a.y, b.y), max(a.z, b.z) }; }
inline static v3    v3_min       (v3 a, v3 b)        { return (v3) { min(a.x, b.x), min(a.y, b.y), min(a.z, b.z) }; }
inline static v3    v3_mix       (v3 a, v3 b, f32 t) { return (v3){sqrt(a.x*a.x*(1.0-t)+b.x*b.x*t), sqrt(a.y*a.y*(1.0-t)+b.y*b.y*t), sqrt(a.z*a.z*(1.0-t)+b.z*b.z*t)}; }
inline static f32   v3_dot       (v3 a, v3 b)        { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline static f32   v3_length    (v3 a)              { return sqrt(v3_dot(a, a)); }
inline static v3    v3_normalize (v3 a)              { return v3_div_scl(a, v3_length(a)); }
inline static f32   v3_distance  (v3 a, v3 b)        { return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y)); }
inline static v3    v3_cross     (v3 a, v3 b)        { return (v3) { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x }; }
inline static v3    v3_reflect   (v3 v, v3 n)        { return v3_sub(v, v3_mul_scl(n, v3_dot(v, n) * 2.0)); }
inline static v3    v3_refract   (v3 v, v3 n, f32 r) { 
    const float d = v3_dot(n, v);
    const float k = 1.0 - r * r * (1.0 - d*d);
    if (k < 0.0) return (v3) { 0.0, 0.0, 0.0 };
    return v3_sub(v3_mul_scl(v, r), v3_mul_scl(n, (r * d + sqrt(k))));
}

typedef struct { f32 x,y,z,w; } v4;
inline static v4    v4_add       (v4 a, v4 b)        { return (v4) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
inline static v4    v4_add_scl   (v4 a, f32 b)       { return (v4) { a.x + b, a.y + b, a.z + b, a.w + b }; }
inline static v4    v4_sub       (v4 a, v4 b)        { return (v4) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
inline static v4    v4_sub_scl   (v4 a, f32 b)       { return (v4) { a.x - b, a.y - b, a.z - b, a.w - b }; }
inline static v4    v4_mul       (v4 a, v4 b)        { return (v4) { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }; }
inline static v4    v4_mul_scl   (v4 a, f32 b)       { return (v4) { a.x * b, a.y * b, a.z * b, a.w * b }; }
inline static v4    v4_div       (v4 a, v4 b)        { return (v4) { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w }; }
inline static v4    v4_div_scl   (v4 a, f32 b)       { return (v4) { a.x / b, a.y / b, a.z / b, a.w / b }; }
inline static f32   v4_dot       (v4 a, v4 b)        { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
inline static v4    v4_fract     (v4 a)              { return (v4) { fract(a.x), fract(a.y), fract(a.z), fract(a.w) }; }
inline static v4    v4_mix       (v4 a, v4 b, f32 t) { return (v4) { sqrt(a.x*a.x*(1.0-t)+b.x*b.x*t),sqrt(a.y*a.y*(1.0-t)+b.y*b.y*t),sqrt(a.z*a.z*(1.0-t)+b.z*b.z*t),sqrt(a.w*a.w*(1.0-t)+b.w*b.w*t) }; }

typedef struct { f32 data[9]; } mat3;
inline static mat3 mat3_make(f32 m00, f32 m01, f32 m02, f32 m10, f32 m11, f32 m12, f32 m20, f32 m21, f32 m22)
{
    return (mat3){ m00, m01, m02, m10, m11, m12, m20, m21, m22 };
}

inline static v3 mat3_mul(mat3 m, v3 v)
{
    return (v3)
    {
        m.data[0] * v.x + m.data[3] * v.y + m.data[6] * v.z,
        m.data[1] * v.x + m.data[4] * v.y + m.data[7] * v.z,
        m.data[2] * v.x + m.data[5] * v.y + m.data[8] * v.z,
    };
}
