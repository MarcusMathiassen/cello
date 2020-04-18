#include "common.h"
#include "shader_common.h"

#define PIXEL_RADIUS 0.0001

METAL_INTERNAL f32 hash21(v2 p)
{
    p = fract(p * v2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

METAL_INTERNAL v3 mmod(v3 x, v3 y)
{
    return x - y * floor(x / y);
}
#define mod(x, y) mmod(x, y)

METAL_INTERNAL v3 opRep(v3 p, v3 c)
{
    return mod(p + 0.5 * c, c) - 0.5 * c;
}
METAL_INTERNAL v3 opRepLim(v3 p, f32 c, v3 l)
{
    return p - c * clamp(round(p / c), -l, l);
}

METAL_INTERNAL v3 rotateX(v3 p, f32 a)
{
    f32 c = cos(a);
    f32 s = sin(a);
    return v3(p.x, c * p.y - s * p.z, s * p.y + c * p.z);
}

METAL_INTERNAL v3 rotateY(v3 p, f32 a)
{
    f32 c = cos(a);
    f32 s = sin(a);
    return v3(c * p.x + s * p.z, p.y, -s * p.x + c * p.z);
}

METAL_INTERNAL v3 rotateZ(v3 p, f32 a)
{
    f32 c = cos(a);
    f32 s = sin(a);
    return v3(c * p.x - s * p.y, s * p.x + c * p.y, p.z);
}
METAL_INTERNAL f32 sdBox(v3 p, v3 b)
{
    v3 d = fabs(p) - b;
    return length(max(d, v3(0,0,0))) + min(max(d.x, max(d.y, d.z)), 0.0f); // remove this line for an only partially signed sdf
}
METAL_INTERNAL f32 sdRoundBox(v3 p, v3 b, f32 r)
{
    v3 d = fabs(p) - b;
    return length(max(d, v3(0,0,0))) - r + min(max(d.x, max(d.y, d.z)), 0.0f); // remove this line for an only partially signed sdf
}
METAL_INTERNAL f32 sdCappedCylinder(v3 p, f32 h, f32 r)
{
    v2 d = fabs(v2(length(p.xz), p.y)) - v2(h, r);
    return min(max(d.x, d.y), 0.0f) + length(max(d, v2(0,0)));
}
METAL_INTERNAL f32 sdTorus(v3 p, v2 t)
{
    v2 q = v2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

METAL_INTERNAL f32 sdSphere(v3 p, f32 s)
{
    return length(p) - s;
}

METAL_INTERNAL f32 dot2(v3 v)
{
    return dot(v, v);
}
METAL_INTERNAL f32 udTriangle(v3 p, v3 a, v3 b, v3 c)
{
    v3 ba = b - a;
    v3 pa = p - a;
    v3 cb = c - b;
    v3 pb = p - b;
    v3 ac = a - c;
    v3 pc = p - c;
    v3 nor = cross(ba, ac);

    return sqrt(
        (sign(dot(cross(ba, nor), pa)) + sign(dot(cross(cb, nor), pb)) + sign(dot(cross(ac, nor), pc)) < 2.0)
            ? min(min(
                      dot2(ba * saturate(dot(ba, pa) / dot2(ba)) - pa),
                      dot2(cb * saturate(dot(cb, pb) / dot2(cb)) - pb)),
                dot2(ac * saturate(dot(ac, pc) / dot2(ac)) - pc))
            : dot(nor, pa) * dot(nor, pa) / dot2(nor));
}

METAL_INTERNAL f32 sdPlane(v3 p, v3 n, f32 h)
{
    return dot(p, n) - (h);
}

METAL_INTERNAL v2 pSmoothUnion(v2 d1, v2 d2, f32 k)
{
    auto a = d1.x;
    auto b = d2.x;
    auto h = saturate(0.5f + 0.5f * (b - a) / k);
    auto s = mix(b, a, h) - k * h * (1.0 - h);
    return v2(s, h > 0.5 ? d1.y : d2.y);
}

METAL_INTERNAL v2 pSmoothSubtraction(v2 d2, v2 d1, f32 k)
{
    auto h = saturate(0.5f - 0.5f * (d2.x + d1.x) / k);
    auto x = mix(d2.x, -d1.x, h) + k * h * (1.0 - h);
    return v2(x, h > 0.5 ? d1.y : d2.y);
}

METAL_INTERNAL v2 pSmoothIntersection(v2 d2, v2 d1, f32 k)
{
    auto h = saturate(0.5f - 0.5f * (d2.x - d1.x) / k);
    auto x = mix(d2.x, d1.x, h) + k * h * (1.0 - h);
    return v2(x, h > 0.5 ? d1.y : d2.y);
}

METAL_INTERNAL v2
pUnion(v2 d1, v2 d2)
{
    return (d1.x < d2.x) ? d1 : d2;
}

METAL_INTERNAL v2
pSub(v2 d2, v2 d1)
{
    return (-d1.x > d2.x) ? v2(-d1.x, d1.y) : d2;
}

METAL_INTERNAL v2
pIntersect(v2 d2, v2 d1)
{
    return (d1.x > d2.x) ? d1 : d2;
}

// Return the distance and material id of the closest object hit in the scene.
// @Todo: Smoothing amount can be based on the edits pos.x
v2 map(v3 p, Scene scene, v2 res = v2(FLT_MAX, 0.0))
{
    // holds the temporary result of each operation
    v2 d;

    // We make a copy of the position so we can reset later.
    v3 pp = p;

    // state
    v3 size = v3(0.01,0.01,0.01);
    f32 material_id = 11.0;
    f32 rounding = 0.1;

    auto count = scene.edit_info.count;
    METAL(constant) auto& edits = scene.edit_info.edits;

    // clang-format off
    for (s8 i = 0; i < count; ++i) {
        METAL(constant) Edit& e = edits[i];
        switch (e.kind) {

            case SET_SIZE:            size = e.data;                                                         break;
            case SET_MATERIAL_ID:     material_id = e.data.x;                                                break;
            case SET_ROUNDING:        rounding = e.data.x;                                                   break;

            case SD_TRIANGLE:
            {
                // get the three next edits as vertices
                v3 p1 = e.data;
                v3 p2 = edits[i+1].data;
                v3 p3 = edits[i+2].data;
                d = v2(udTriangle(pp, p1, p2, p3)-PIXEL_RADIUS, material_id);
                i += 2;
            } break;

            case SD_PLANE:            d = v2(sdPlane(pp, e.data, size.x), material_id);                      break;
            case SD_SPHERE:           d = v2(sdSphere(pp - e.data, size.x), material_id);                    break;
            case SD_BOX:              d = v2(sdBox(pp - e.data, size), material_id);                         break;
            case SD_ROUND_BOX:        d = v2(sdRoundBox(pp - e.data, size, rounding), material_id);          break;
            case SD_TORUS:            d = v2(sdTorus(pp - e.data, size.xy), material_id);                    break;
            case SD_CAPPED_CYLINDER:  d = v2(sdCappedCylinder(pp - e.data, size.x, size.y), material_id);    break;
            case OP_UNION:            res = pUnion(res, d);                                                 break;
            case OP_SUBTRACT:         res = pSub(res, d);                                                   break;
            case OP_INTERSECT:        res = pIntersect(res, d);                                             break;
            case OP_SMOOTH_UNION:     res = pSmoothUnion(res, d, e.data.x);                                  break;
            case OP_SMOOTH_SUBTRACT:  res = pSmoothSubtraction(res, d, e.data.x);                            break;
            case OP_SMOOTH_INTERSECT: res = pSmoothIntersection(res, d, e.data.x);                           break;

            case OP_ROUNDED:          d.x -= e.data.x;                                                       break;
            case OP_ANNULAR:          d.x = fabs(d.x) - e.data.x;                                            break;

            case OP_REP:              pp = opRep(pp, e.data);                                                break;
            case OP_ROTATE_X:         pp = rotateX(pp, e.data.x);                                            break;
            case OP_ROTATE_Y:         pp = rotateY(pp, e.data.x);                                            break;
            case OP_ROTATE_Z:         pp = rotateZ(pp, e.data.x);                                            break;

            case OP_RESET:            pp = p;                                                               break;
        }
    }
    // clang-format on
    return res;
}

METAL_INTERNAL v2 SS2NDC(v2 uv, v2 res)
{
    return (uv - res*0.5) / res.y;
}

template <class T>
METAL_INTERNAL v3 calcNormal(v3 p, T scene)
{
    const f32 e = PIXEL_RADIUS;
    return normalize(
        (v3)
        {
            map(p+(v3){e,0,0}, scene).x - map(p-(v3){e,0,0}, scene).x,
            map(p+(v3){0,e,0}, scene).x - map(p-(v3){0,e,0}, scene).x,
            map(p+(v3){0,0,e}, scene).x - map(p-(v3){0,0,e}, scene).x
        }
    );
}

template <class T>
METAL_INTERNAL Hit castRay(v3 ro, v3 rd, s32 steps, f32 t_min, f32 t_max, T scene)
{
    f32 t = t_min;
    for (s32 i = 0; i < steps && t < t_max; ++i)
    {
        const v2 r = map(ro+rd*t, scene);
        if (r.x < t_min) return (Hit){ (t), (s16)(r.y), (s16)(i) };
        t += r.x;
    }
    return (Hit){ FLT_MAX, 0, 0 };
}

METAL_INTERNAL v3 OECF_sRGBFast(v3 linear)
{
    return pow(linear, v3(1.0/2.2,1.0/2.2,1.0/2.2));
}

METAL_INTERNAL v3 ACES(v3 x)
{
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    f32 a = 2.51;
    f32 b = 0.03;
    f32 c = 2.43;
    f32 d = 0.59;
    f32 e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}
template <class T>
METAL_INTERNAL f32 ambientOcclusion(v3 p, v3 n, T scene)
{
    f32 stepDist = 0.2;
    f32 maxDist = 8.0;
    s32 samples = 5;
    f32 sca = 1.0;
    f32 ao = 0.0;
    for (s32 i = 1; i <= samples; ++i) {
        f32 h = stepDist * i / maxDist;
        f32 d = map(p + n * h, scene).x;
        ao += (h - d) * sca;
        sca *= 0.95;
    }
    return saturate(1.0f - ao);
}

template <class T>
METAL_INTERNAL f32 shadow(v3 ro, v3 rd, f32 nearClip, f32 farClip, T scene)
#ifdef SOFT_SHADOWS
{
    f32 res = 1.0;
    f32 k = 32.0;
    for (f32 t = nearClip; t < farClip;) {
        f32 h = map(ro + rd * t, scene).x;
        if (h < nearClip) return 0.0;
        res = min(res, k * h / t);
        t += h;
    }
    return res;
}
#else
{
    for (f32 t = nearClip; t < farClip;) {
        f32 h = map(ro + rd * t, scene).x;
        if (h < nearClip) return 0.0;
        t += h;
    }
    return 1.0;
}
#endif