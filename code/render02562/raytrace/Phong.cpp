// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "Phong.h"
#include <stdio.h>

using namespace optix;

#ifndef M_1_PIf
#define M_1_PIf 0.31830988618379067154
#endif

float3 Phong::shade(const Ray& r, HitInfo& hit, bool emit) const
{
    float3 rho_d = get_diffuse(hit);
    float3 rho_s = get_specular(hit);
    float s = get_shininess(hit);

    // Implement Phong reflection here.
    //
    // Input:  r          (the ray that hit the material)
    //         hit        (info about the ray-surface intersection)
    //         emit       (passed on to Emission::shade)
    //
    // Return: radiance reflected to where the ray was coming from
    //
    // Relevant data fields that are available (see Lambertian.h, Ray.h, and above):
    // lights             (vector of pointers to the lights in the scene)
    // hit.position       (position where the ray hit the material)
    // hit.shading_normal (surface normal where the ray hit the material)
    // rho_d              (difuse reflectance of the material)
    // rho_s              (specular reflectance of the material)
    // s                  (shininess or Phong exponent of the material)
    //
    // Hint: Call the sample function associated with each light in the scene.
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

            float3 first_coeff = rho_d*M_1_PIf;
            float dotproduct = dot(wo, wr);
            dotproduct = fmax(dotproduct, 0);
            float3 second_coeff = rho_s*(s + 2)*pow(dotproduct, s)*M_1_PIf/2;

            Li *= (first_coeff + second_coeff);
//        printf("Value of Li : %f - %f - %f\n", Li.x, Li.y, Li.z);
            Lr += Li;
        }
    }
    return Lr + Lambertian::shade(r, hit, emit);
}
