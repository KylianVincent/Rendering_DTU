// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "Lambertian.h"

using namespace optix;

// The following macro defines 1/PI
#ifndef M_1_PIf
#define M_1_PIf 0.31830988618379067154
#endif

float3 Lambertian::shade(const Ray& r, HitInfo& hit, bool emit) const
{
    float3 rho_d = get_diffuse(hit);
    float3 result = make_float3(0.0f);

    // Implement Lambertian reflection here.
    //
    // Input:  r          (the ray that hit the material)
    //         hit        (info about the ray-surface intersection)
    //         emit       (passed on to Emission::shade)
    //
    // Return: radiance reflected to where the ray was coming from
    //
    // Relevant data fields that are available (see Lambertian.h, HitInfo.h, and above):
    // lights             (vector of pointers to the lights in the scene)
    // hit.position       (position where the ray hit the material)
    // hit.shading_normal (surface normal where the ray hit the material)
    // rho_d              (difuse reflectance of the material)
    //
    // Hint: Call the sample function associated with each light in the scene.

    for (int i = 0; i < lights.size(); i++) {
        Light* light = lights[i];
        float3 dir, Li;
        if (light->sample(hit.position, dir, Li)) {
            float cosine = dot(dir, hit.shading_normal)/(length(dir) + length(hit.shading_normal));
            if (cosine > 0) {
                result += cosine * Li;
            }
        }

    }

    return result*rho_d + Emission::shade(r, hit, emit);
}
