// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include <cstdio>
#include "HitInfo.h"
#include "mt_random.h"
#include "PointLight.h"

using namespace optix;

bool PointLight::sample(const float3& pos, float3& dir, float3& L) const
{
    // Compute output and return value given the following information.
    //
    // Input:  pos (the position of the geometry in the scene)
    //
    // Output: dir (the direction toward the light)
    //         L   (the radiance received from the direction dir)
    //
    // Return: true if not in shadow
    //
    // Relevant data fields that are available (see PointLight.h and Light.h):
    // shadows    (on/off flag for shadows)
    // tracer     (pointer to ray tracer)
    // light_pos  (position of the point light)
    // intensity  (intensity of the emitted light)
    //
    // Hint: Construct a shadow ray using the Ray datatype. Trace it using the
    //       pointer to the ray tracer.


    // Test if there's a hit before the object
    dir = light_pos - pos;
    float dist = length(dir);
    dir = normalize(dir);

    // Light fading
    L = intensity/(pow(dist, 2));

    Ray shadow_ray = Ray(light_pos, -dir, 0, 0, dist - 1e-4);
    HitInfo hit;

    if (!shadows) {
        return true;
    }

    return !tracer->trace_to_any(shadow_ray, hit);
}

bool PointLight::emit(Ray& r, HitInfo& hit, float3& Phi) const
{
    // Emit a photon by creating a ray, tracing it, and computing its flux.
    //
    // Output: r    (the photon ray)
    //         hit  (the photon ray hit info)
    //         Phi  (the photon flux)
    //
    // Return: true if the photon hits a surface
    //
    // Relevant data fields that are available (see PointLight.h and Light.h):
    // tracer     (pointer to ray tracer)
    // light_pos  (position of the point light)
    // intensity  (intensity of the emitted light)
    //
    // Hint: When sampling the ray direction, use the function
    //       mt_random() to get a random number in [0,1].

    // Sample ray direction and create ray
    float x, y, z;
    do {
        x = mt_random() * 2 - 1; // Random value between -1 and 1
        y = mt_random() * 2 - 1; // Random value between -1 and 1
        z = mt_random() * 2 - 1; // Random value between -1 and 1
    } while (pow(x, 2) + pow(y, 2) + pow(z, 2) > 1);
    float3 dir = normalize(make_float3(x, y, z));

    // Trace ray
    r = Ray(light_pos, dir, 0, 1e-4, RT_DEFAULT_MAX);

    // If a surface was hit, compute Phi and return true
    if (tracer->trace_to_closest(r, hit)) {
        //Surface was hit
        Phi = intensity*4*M_PIf;
        return true;
    }

    return false;
}
