// 02576 Rendering Framework
// Written by Jeppe Revall Frisvad, 2010
// Copyright (c) DTU Informatics 2010

#include <optix_world.h>
#include "HitInfo.h"
#include "Volume.h"
#include <stdio.h>

using namespace optix;

float3 Volume::shade(const Ray& r, HitInfo& hit, bool emit) const
{
    // If inside the volume, Find the direct transmission through the volume by using
    // the transmittance to modify the result from the Transparent shader.
    // If inside the ray going out and the material normal (pointing outwards) are pointing in the same direction
    // (same hemisphere)
    if (dot(r.direction, hit.geometric_normal) > 0) {
        return get_transmittance(hit) * Transparent::shade(r, hit, emit);
    }
    return Transparent::shade(r, hit, emit);
}

float3 Volume::get_transmittance(const HitInfo& hit) const
{
    if(hit.material)
    {
        // Compute and return the transmittance using the diffuse reflectance of the material.
        // Diffuse reflectance rho_d does not make sense for a specular material, so we can use
        // this material property as an absorption coefficient. Since absorption has an effect
        // opposite that of reflection, using 1/rho_d-1 makes it more intuitive for the user.
        float3 rho_d = make_float3(hit.material->diffuse[0], hit.material->diffuse[1], hit.material->diffuse[2]);
        float3 rho_d_corrected = make_float3(fmax(rho_d.x, 1e-5), fmax(rho_d.y, 1e-5), fmax(rho_d.z, 1e-5));
        float3 absorption = 1/rho_d_corrected - 1;
        return make_float3(exp(-absorption.x*hit.dist), exp(-absorption.y*hit.dist), exp(-absorption.z*hit.dist));
    }
    //No material, full transmission
    return make_float3(1.0f);
}
