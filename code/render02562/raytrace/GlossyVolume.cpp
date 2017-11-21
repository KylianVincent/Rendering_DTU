// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "int_pow.h"
#include "GlossyVolume.h"

using namespace optix;

#ifndef M_1_PIf
#define M_1_PIf 0.31830988618379067154
#endif

float3 GlossyVolume::shade(const Ray& r, HitInfo& hit, bool emit) const
{
    // Compute the specular part of the glossy shader and attenuate it
    // by the transmittance of the material if the ray is inside (as in
    // the volume shader).

    float3 rho_s = get_specular(hit);
    float s = get_shininess(hit);

    float3 wi, wo, wr, Li;
    float3 Lr = make_float3(0.0);
    for (int i = 0; i < lights.size(); i++) {
        Light *light = lights[i];
        if (light->sample(hit.position, wi, Li)) {
            // Not in shadows, we add this light participation
            wr = normalize(reflect(-wi, hit.shading_normal));
            wo = normalize(-r.direction);

            // Compute phong for this light
            Li = make_float3(fmax(dot(wi, hit.shading_normal), 0));

            float dotproduct = dot(wo, wr);
            dotproduct = fmax(dotproduct, 0);

            Li *= rho_s*(s + 2)*pow(dotproduct, s)*M_1_PIf/2;
            Lr += Li;
        }
    }

    if(hit.trace_depth >= max_depth)
        return make_float3(0.0f);

    float R;
    Ray reflected, refracted;
    HitInfo hit_reflected, hit_refracted;
    tracer->trace_reflected(r, hit, reflected, hit_reflected);
    tracer->trace_refracted(r, hit, refracted, hit_refracted, R);

    float3 res = R*shade_new_ray(reflected, hit_reflected) + (1.0f - R)*shade_new_ray(refracted, hit_refracted) + Lr;

    if (dot(r.direction, hit.geometric_normal) > 0) {
        return get_transmittance(hit) * res;
    }

    return res;
}
