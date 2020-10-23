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

typedef struct
{
    f32 x;
    f32 y;
} v2;

inline static v2 v2_add(v2 a, v2 b) { return (v2) { a.x + b.x, a.y + b.y }; }
inline static v2 v2_add_scl(v2 a, f32 b) { return (v2) { a.x + b, a.y + b }; }
inline static v2 v2_sub(v2 a, v2 b) { return (v2) { a.x - b.x, a.y - b.y }; }
inline static v2 v2_sub_scl(v2 a, f32 b) { return (v2) { a.x - b, a.y - b }; }
inline static v2 v2_mul(v2 a, v2 b) { return (v2) { a.x * b.x, a.y * b.y }; }
inline static v2 v2_mul_scl(v2 a, f32 b) { return (v2) { a.x * b, a.y * b }; }
inline static v2 v2_div(v2 a, v2 b) { return (v2) { a.x / b.x, a.y / b.y }; }
inline static v2 v2_div_scl(v2 a, f32 b) { return (v2) { a.x / b, a.y / b }; }
