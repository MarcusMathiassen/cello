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

#include "math.cc"

typedef struct
{
    f32 x;
    f32 y;
    f32 z;
} v3;

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
