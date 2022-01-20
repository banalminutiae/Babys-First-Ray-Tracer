#include <stdint.h>
#include "ray_math.hh"

#pragma pack(push, 1)
struct Bitmap_Header {
    u16 file_type;
    u32 file_size;
    u16 reserved;
    u16 reserved2;
    u32 bitmap_offset;
    u32 size;
    s32 width;
    s32 height;
    u16 planes;
    u16 bits_per_pixel;
    u32 compression;
    u32 bitmap_size;
    s32 h_resolution;
    s32 v_resolution;
    u32 colors_used;
    u32 colors_important;
};
#pragma pack(pop)

struct Image32 {
    u32 width;
    u32 height;
    u32 *pixels;
};

struct Material {
    f32 specular;
    vec3 emit_color;
    vec3 reflect_color;
    vec3 color;
};

struct Plane {
    vec3 normal;
    f32 d;
    u32 material_index;
};

struct Sphere {
    vec3 position;
    f32 radius;
    u32 material_index;
};

struct World {
    u32 material_count;
    u32 plane_count;
    u32 sphere_count;
    
    Material *materials;
    Plane *planes;
    Sphere *spheres;
};
