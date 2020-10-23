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
// DEALINGS IsN THE SOFTWARE.
#include "kernel_common.cc"

METAL_INTERNAL v3 directLight(const METAL(constant) Light& light, v3 eye, v3 P, v3 N)
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
        const f32 glossiness = 32.0;
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

METAL_INTERNAL v3 Irradiance_SphericalHarmonics(v3 n)
{
    // Irradiance from "Ditch River" IBL
    // (http://www.hdrlabs.com/sibl/archive.html)
    return max(
        v3(0.754554516862612, 0.748542953903366, 0.790921515418539)
        + v3(0.3, 0.3, 0.3) * (n.y) + v3(0.35, 0.36, 0.35) * (n.z) + 
        v3(-0.2, -0.24, -0.24) * (n.x),
        v3(0,0,0));
}

METAL_INTERNAL v3 ambientLight(v3 P, v3 N)
{
    const v3 al = Irradiance_SphericalHarmonics(N) * (v3){0.7, 0.76, 0.85};
    return al;
}

METAL_INTERNAL METAL(kernel) void
steps(
    METAL(constant) Uniform& uniform        METAL([[buffer(0)]]),
    METAL(constant) Light_Info& light_info  METAL([[buffer(1)]]),
    METAL(constant) Material* materials     METAL([[buffer(2)]]), 
    METAL(constant) Edit_Info& edit_info    METAL([[buffer(3)]]),
    METAL(device)   u32* pixels             METAL([[buffer(4)]]),
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

        const auto scene = (Scene) { edit_info };

        const f32 farClip = (distance(ro, rd * v3(40,40,40)));
        const s32 maxStepCount = 256;
        const f32 nearClip = PIXEL_RADIUS;

        const auto hit = castRay(ro, rd, maxStepCount, nearClip, farClip, scene);

        v3 color = v3(1,1,1)*0.0;

        if (hit.t < farClip)
        {
            f32 xxx = (f32)hit.steps / maxStepCount;
            color = v3(0.0,xxx,0.0);
        }


        // color = OECF_sRGBFast(color);
        // color = ACES(color);

        const u8 R = saturate(color.x) * 255.0;
        const u8 G = saturate(color.y) * 255.0;
        const u8 B = saturate(color.z) * 255.0;
        const u8 A = 255;

        const s32 index = y * uniform.viewport_size.x + x;
        pixels[index] = ((R << 0) | (G << 8) | (B << 16) | (A << 24));
    }
}

METAL_INTERNAL METAL(kernel) void
normals(
    METAL(constant) Uniform& uniform        METAL([[buffer(0)]]),
    METAL(constant) Light_Info& light_info  METAL([[buffer(1)]]),
    METAL(constant) Material* materials     METAL([[buffer(2)]]), 
    METAL(constant) Edit_Info& edit_info    METAL([[buffer(3)]]),
    METAL(device)   u32* pixels             METAL([[buffer(4)]]),
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

        const auto scene = (Scene) { edit_info };

        const f32 farClip = (distance(ro, rd * v3(40,40,40)));
        const s32 maxStepCount = 256;
        const f32 nearClip = PIXEL_RADIUS;

        const auto hit = castRay(ro, rd, maxStepCount, nearClip, farClip, scene);

        v3 color = v3(1,1,1)*0.0;

        if (hit.t < farClip)
        {
            v3 P = ro + rd * hit.t;
            v3 N = calcNormal(P, scene);
            color = N;
        }

        // color = OECF_sRGBFast(color);
        // color = ACES(color);

        const u8 R = saturate(color.x) * 255.0;
        const u8 G = saturate(color.y) * 255.0;
        const u8 B = saturate(color.z) * 255.0;
        const u8 A = 255;

        const s32 index = y * uniform.viewport_size.x + x;
        pixels[index] = ((R << 0) | (G << 8) | (B << 16) | (A << 24));
    }
}


METAL_INTERNAL METAL(kernel) void
uber(
    METAL(constant) Uniform& uniform        METAL([[buffer(0)]]),
    METAL(constant) Light_Info& light_info  METAL([[buffer(1)]]),
    METAL(constant) Material* materials     METAL([[buffer(2)]]), 
    METAL(constant) Edit_Info& edit_info    METAL([[buffer(3)]]),
    METAL(device)   u32* pixels             METAL([[buffer(4)]]),
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

        const auto scene = (Scene) { edit_info };

        const f32 farClip = (distance(ro, rd * v3(50,50,50)));
        const s32 maxStepCount = 256;
        const f32 nearClip = PIXEL_RADIUS;

        const auto hit = castRay(ro, rd, maxStepCount, nearClip, farClip, scene);

        v3 color = v3(1,1,1)*0.0;

        if (hit.t < farClip)
        {
            v3 P = ro + rd * hit.t;
            v3 N = calcNormal(P, scene);

            // Shadow
            f32 sha = 0.0;
            {
                // for (s8 i = 0; i < light_info.count; ++i)
                // {
                const METAL(constant) auto& light = light_info.lights[0]; // only the sun casts shadow
                const v3 L = normalize(light.pos - P);
                sha += shadow(P+N*PIXEL_RADIUS, L, nearClip, farClip, scene);
                // }
            }

            // Ambient Occlusion
            f32 ao = 1.0;
            {
                ao = ambientOcclusion(P, N, scene);
            }

            // Direct Illumination
            v3 directLightContrib = {};
            {
                const v3 eye = ro;
                for (s8 i = 0; i < light_info.count; ++i) {
                    const METAL(constant) auto& light = light_info.lights[i];
                    directLightContrib += directLight(light, eye, P, N);
                }
            }

            // Ambient Illumination
            v3 ambientLightContrib = {};
            {
                ambientLightContrib = ambientLight(P, N);
            }

            {
                const v3 albedo = materials[hit.material_id].color;
                color = albedo * (sha * directLightContrib + ao * ambientLightContrib);
            }
        }

        // Draw workload grid
        // if (x == tid.x ||
        //     y == tid.y || 
        //     x == uniform.viewport_size.x-1 ||
        //     y == uniform.viewport_size.y-1) {
        //     color = v3(0.0, 1.0, 0.0) * 0.5;
        // }

        // color = OECF_sRGBFast(color);
        color = ACES(color);

        const u8 R = saturate(color.x) * 255.0;
        const u8 G = saturate(color.y) * 255.0;
        const u8 B = saturate(color.z) * 255.0;
        const u8 A = 255;

        const s32 index = y * uniform.viewport_size.x + x;
        pixels[index] = ((R << 0) | (G << 8) | (B << 16) | (A << 24));
    }
}

METAL_INTERNAL METAL(kernel) void
clear(
    METAL(constant) Uniform& uniform        METAL([[buffer(0)]]),
    METAL(constant) v4 color                METAL([[buffer(1)]]),
    METAL(device)   u32* pixels             METAL([[buffer(2)]]),
    ushort2 tid                             METAL([[thread_position_in_grid]]),
    ushort2 gs                              METAL([[threads_per_grid]]))
{
    for (u16 y = tid.y; y < gs.y; ++y)
    for (u16 x = tid.x; x < gs.x; ++x)
    {
        const u8 R = saturate(color.x) * 255.0;
        const u8 G = saturate(color.y) * 255.0;
        const u8 B = saturate(color.z) * 255.0;
        const u8 A = saturate(color.w) * 255.0;

        const s32 index = y * uniform.viewport_size.x + x;
        pixels[index] = ((R << 0) | (G << 8) | (B << 16) | (A << 24));
    }
}

METAL_INTERNAL METAL(kernel) void
tiles(
    METAL(constant) Uniform& uniform        METAL([[buffer(0)]]),
    METAL(constant) Light_Info& light_info  METAL([[buffer(1)]]),
    METAL(constant) Material* materials     METAL([[buffer(2)]]), 
    METAL(constant) Edit_Info& edit_info    METAL([[buffer(3)]]),
    METAL(device)   u32* pixels             METAL([[buffer(4)]]),
    ushort2 tid                             METAL([[thread_position_in_grid]]),
    ushort2 gs                              METAL([[threads_per_grid]]))
{
    for (u16 y = tid.y; y < gs.y; ++y)
    for (u16 x = tid.x; x < gs.x; ++x)
    {
        v3 color = v3(0.0, 0.0, 0.0);
        if (x == tid.x ||
            y == tid.y || 
            x == uniform.viewport_size.x-1 ||
            y == uniform.viewport_size.y-1) {
            color = v3(0.0, 1.0, 0.0) * 0.5;
        } else continue;

        const u8 R = saturate(color.x) * 255.0;
        const u8 G = saturate(color.y) * 255.0;
        const u8 B = saturate(color.z) * 255.0;
        const u8 A = 255;

        const s32 index = y * uniform.viewport_size.x + x;
        pixels[index] = ((R << 0) | (G << 8) | (B << 16) | (A << 24));
    }
}
