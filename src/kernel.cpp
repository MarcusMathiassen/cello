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

#include "shader_common.h"

#ifdef __METAL__
    #define METAL(x) #x
#else
    #define METAL(x)
#endif

#ifndef __METAL__
#define clamp(x,a,b) simd_clamp(x,(f32)a,(f32)b)
#define saturate(x) clamp(x, 0.0f, 1.0f)
#endif

inline static v3 directLight(const METAL(constant) Light& light, v3 eye, v3 P, v3 N)
{
    const v3 lightDelta = light.pos - P;
    const v3 L = normalize(lightDelta);
    const f32 NdotL = saturate(dot(L, N));

    // Diffuse
    v3 lightContrib = v3(0,0,0);
    {
        const f32 halfLambert = NdotL * 0.5 + 0.5; // half lambert
        const f32 lightDistSqr = dot(lightDelta, lightDelta);
        lightContrib = light.intensity * light.color * halfLambert / lightDistSqr;
    }

    // Specular
    const v3 V = normalize(eye - P);
    v3 specularContrib = v3(0,0,0);
    {
        const v3 halfVector = normalize(L + V);
        const f32 NdotH = saturate(dot(N, halfVector));
        const f32 glossiness = 16.0;
        const f32 specularIntensity = pow(NdotH, glossiness * glossiness);
        const f32 specularIntensitySmooth = specularIntensity;
        const v3 specColor =light.color;

        specularContrib = specularIntensitySmooth * specColor;
    }

    // Rim
    v3 rimContrib = v3(0,0,0);
    {
        f32 threshold = 0.5;
        f32 NdotV = dot(N, V);
        f32 rimDot = (1.0 - NdotV) * pow(NdotL, threshold);
        f32 intensity = rimDot;
        rimContrib = intensity * light.color;
    }

    return (lightContrib + specularContrib + rimContrib);
}

inline static v3 Irradiance_SphericalHarmonics(v3 n)
{
    // Irradiance from "Ditch River" IBL
    // (http://www.hdrlabs.com/sibl/archive.html)
    return max(
        v3(0.754554516862612, 0.748542953903366, 0.790921515418539)
        + v3(0.3, 0.3, 0.3) * (n.y) + v3(0.35, 0.36, 0.35) * (n.z) + 
        v3(-0.2, -0.24, -0.24) * (n.x),
        v3(0,0,0));
}

inline static v3 ambientLight(v3 P, v3 N)
{
    const v3 al = Irradiance_SphericalHarmonics(N) * (v3){0.7, 0.76, 0.85};
    return al;
}

inline static v2 SS2NDC(v2 uv, v2 res)
{
    return (uv - res*0.5) / res.y;
}

inline static f32 sdSphere(v3 p, f32 r) { return length(p) - r; }
inline static f32 sdPlane(v3 p, v3 n, f32 h) { return dot(p, n) - (h); }

inline static v2 map(v3 p)
{
    v2 result = { FLT_MAX, 0.0 };

    f32 d = sdSphere(p, 1.0);
    result.x = d;

    return result;
}

#define PIXEL_RADIUS 0.0001

inline static v3 calcNormal(v3 p)
{
    const f32 e = PIXEL_RADIUS;
    return normalize(
        (v3)
        {
            map(p+(v3){e,0,0}).x - map(p-(v3){e,0,0}).x,
            map(p+(v3){0,e,0}).x - map(p-(v3){0,e,0}).x,
            map(p+(v3){0,0,e}).x - map(p-(v3){0,0,e}).x
        }
    );
}

inline static Hit castRay(v3 ro, v3 rd, s32 steps, f32 t_min, f32 t_max)
{
    f32 t = t_min;
    for (s32 i = 0; i < steps && t < t_max; ++i)
    {
        const v2 r = map(ro+rd*t);
        if (r.x < t_min) return (Hit){ (t), (s16)(r.y), (s16)(i) };
        t += r.x;
    }
    return (Hit){ FLT_MAX, 0, 0 };
}

template <class T, class A>
struct texture3d
{
    T* texels;
    A  access;
    simd::ushort3 size;

    inline simd::ushort3 index(simd::ushort3 uvw) { return uvw.x + size.x * (uvw.y + size.z * uvw.z); }
    inline simd::float3 sample(simd::float3 uvw) { return texels[index(uvw)]; }
    inline void write(simd::float4 data, simd::ushort3 uvw) { texels[index(uvw)] = data; }
};

inline static v3 OECF_sRGBFast(v3 linear)
{
    return pow(linear, v3(1.0/2.2,1.0/2.2,1.0/2.2));
}

inline static v3 ACES(v3 x)
{
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    f32 a = 2.51;
    f32 b = 0.03;
    f32 c = 2.43;
    f32 d = 0.59;
    f32 e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

METAL(kernel)
inline static void uber(
    METAL(constant) Uniform& uniform        METAL([[buffer(0)]]),
    METAL(constant) Light_Info& light_info  METAL([[buffer(1)]]),
    METAL(constant) u32* pixels             METAL([[buffer(2)]]),
    ushort2 tid                             METAL([[thread_position_in_grid]]),
    ushort2 gs                              METAL([[threads_per_grid]]))
{
    for (u16 y = tid.y; y < gs.y; ++y)
    for (u16 x = tid.x; x < gs.x; ++x)
    {
        v2 uv = SS2NDC(v2(x,y), v2(uniform.viewport_size.x,uniform.viewport_size.y));

        uv.y *= -1; // we are software rendering, so we need to flip it manually.

        const v3 ro = uniform.camera_position;
        const v3 rd = uniform.camera_matrix * normalize(v3(uv.x, uv.y, uniform.camera_zoom));

        auto hit = castRay(ro, rd, 256, 0.0001, 1000.0);

        v3 color = v3(0,0,0);

        if (hit.t < 1000.0)
        {
            v3 P = ro+rd*hit.t;
            v3 N = calcNormal(P);

            // Direct Illumination
            v3 directLightContrib = v3(0,0,0);
            {
                v3 eye = ro;
                for (s8 i = 0; i < light_info.count; ++i)
                {
                    const METAL(constant) auto& light = light_info.lights[i];
                    directLightContrib += directLight(light, eye, P, N);
                }
            }

            // Ambient Illumination
            v3 ambientLightContrib = v3(0,0,0);
            {
                ambientLightContrib = ambientLight(P, N);
            }

            f32 sha = 1.0;
            f32 ao = 1.0;
            {
                const v3 albedo = v3(1,1,1);
                color = albedo * (sha*directLightContrib + ao*ambientLightContrib);
            }
        }

        // Draw workload grid
        if (x == tid.x ||
            y == tid.y || 
            x == uniform.viewport_size.x-1 ||
            y == uniform.viewport_size.y-1) {
            color = v3(0.05,0.05,0.05);
        }

        // color = ACES(color);
        color = OECF_sRGBFast(color);

        const u8 R = saturate(color.x) * 255.0;
        const u8 G = saturate(color.y) * 255.0;
        const u8 B = saturate(color.z) * 255.0;
        const u8 A = 255;

        const s32 index = y * uniform.viewport_size.x + x;
        pixels[index] = ((R << 0) | (G << 8) | (B << 16) | (A << 24));
    }
}
