// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "ObjMaterial.h"
#include "fresnel.h"
#include "RayTracer.h"
#include <stdio.h>

using namespace optix;

bool RayTracer::trace_reflected(const Ray& in, const HitInfo& in_hit, Ray& out, HitInfo& out_hit) const
{
    // Initialize the reflected ray and trace it.
    //
    // Input:  in         (the ray to be reflected)
    //         in_hit     (info about the ray-surface intersection)
    //
    // Output: out        (the reflected ray)
    //         out_hit    (info about the reflected ray)
    //
    // Return: true if the reflected ray hit anything
    //
    // Hints: (a) There is a reflect function available in the OptiX math library.
    //        (b) Set out_hit.ray_ior and out_hit.trace_depth.
    out.direction = optix::reflect(in.direction, in_hit.shading_normal);
    out.origin = in_hit.position;
    out.ray_type = 0;
    out.tmax = RT_DEFAULT_MAX;
    out.tmin = 1.0e-4f;

    out_hit.ray_ior = in_hit.ray_ior;
    out_hit.trace_depth = in_hit.trace_depth + 1;

    return trace_to_closest(out, out_hit);
}

bool RayTracer::trace_refracted(const Ray& in, const HitInfo& in_hit, Ray& out, HitInfo& out_hit) const {
    // Initialize the refracted ray and trace it.
    //
    // Input:  in         (the ray to be refracted)
    //         in_hit     (info about the ray-surface intersection)
    //
    // Output: out        (the refracted ray)
    //         out_hit    (info about the refracted ray)
    //
    // Return: true if the refracted ray hit anything
    //
    // Hints: (a) There is a refract function available in the OptiX math library.
    //        (b) Set out_hit.ray_ior and out_hit.trace_depth.
    //        (c) Remember that the function must handle total internal reflection.
    float3 normal;
    float cos_theta_in;
    out_hit.ray_ior = get_ior_out(in, in_hit, out.direction, normal, cos_theta_in);
    out_hit.trace_depth = in_hit.trace_depth + 1;

    bool is_refracted = optix::refract(out.direction, in.direction, normal, out_hit.ray_ior/in_hit.ray_ior);
    if (!is_refracted) {
        // Fully reflected so we call the reflexion for the refractive part
        //printf("Full reflection");
        return trace_reflected(in, in_hit, out, out_hit);
    }

    out.origin = in_hit.position;
    out.ray_type = 0;
    out.tmax = RT_DEFAULT_MAX;
    out.tmin = 1.0e-4f;

    return trace_to_closest(out, out_hit);
}

bool RayTracer::trace_refracted(const Ray& in, const HitInfo& in_hit, Ray& out, HitInfo& out_hit, float& R) const
{
    // Initialize the refracted ray and trace it.
    // Compute the Fresnel reflectance (see fresnel.h) and return it in R.
    //
    // Input:  in         (the ray to be refracted)
    //         in_hit     (info about the ray-surface intersection)
    //
    // Output: out        (the refracted ray)
    //         out_hit    (info about the refracted ray)
    //
    // Return: true if the refracted ray hit anything
    //
    // Hints: (a) There is a refract function available in the OptiX math library.
    //        (b) Set out_hit.ray_ior and out_hit.trace_depth.
    //        (c) Remember that the function must handle total internal reflection.

    float3 normal;
    float cos_theta_in;
    out_hit.ray_ior = get_ior_out(in, in_hit, out.direction, normal, cos_theta_in);
    out_hit.trace_depth = in_hit.trace_depth + 1;

    bool is_refracted = optix::refract(out.direction, in.direction, normal, out_hit.ray_ior/in_hit.ray_ior);
    if (!is_refracted) {
        // Detect total internal reflection
        //printf("Total reflection\n");
        R = 1;
        return false;
    }

    R = fresnel_R(dot(normal, -in.direction), dot(-normal, out.direction), in_hit.ray_ior, out_hit.ray_ior);
    //printf("Values : %f - %f - %f - %f\n", length(in_hit.geometric_normal), length(in.direction), length(-in_hit.geometric_normal), length(out.direction));
    //printf("Values : %f / %f / %f /%f\n", dot(normal, in.direction), dot(normal, out.direction), in_hit.ray_ior, out_hit.ray_ior);
    //printf("R : %f\n", R);
    out.origin = in_hit.position;
    out.ray_type = 0;
    out.tmax = RT_DEFAULT_MAX;
    out.tmin = 1.0e-4f;

    return trace_to_closest(out, out_hit);
}

float RayTracer::get_ior_out(const Ray& in, const HitInfo& in_hit, float3& dir, float3& normal, float& cos_theta_in) const
{
    // Get the refractive index of the material is which the ray is entering by refraction, only support interfaces
    // air/material or material/air.
    // If getting out of a material (detected because the normal is pointing to the outside) the refractive index is
    // automatically set to 1.0f
    // Sets normal and cos_theta_in according to the ray and hit_info
    normal = in_hit.shading_normal;
    dir = -in.direction;
    cos_theta_in = dot(normal, dir);
    if(cos_theta_in < 0.0)
    {
        normal = -normal;
        cos_theta_in = -cos_theta_in;
        return 1.0f;
    }
    const ObjMaterial* m = in_hit.material;
    return m ? m->ior : 1.0f;
}
