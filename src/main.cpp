#include "ray.hh"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static u32 Get_Total_Pixel_Size(Image32 image) {
    return sizeof(u32) * image.width * image.height;
}

static Image32 Allocate_Image(u32 width, u32 height) {
    Image32 image = {};
    image.width = width;
    image.height = height;
    u32 output_pixel_size = Get_Total_Pixel_Size(image);
    image.pixels = (u32*)malloc(output_pixel_size);
    return image;
}

static void Write_Image(char *output_file_name, Image32 image) {
    u32 output_pixel_size = Get_Total_Pixel_Size(image);

    Bitmap_Header header = {};
    header.file_type = 0x4D42;
    header.file_size = sizeof(header) + output_pixel_size;
    header.bitmap_offset = sizeof(header);
    header.size = sizeof(header) - 14;
    header.width = image.width;
    header.height = image.height;
    header.planes = 1;
    header.bits_per_pixel = 32;
    header.bitmap_size = output_pixel_size;

    FILE *output_file = fopen(output_file_name, "wb");
    if (output_file) {
        //write header
        fwrite(&header, sizeof(header), 1, output_file);
        //write bmp body
        fwrite(image.pixels, output_pixel_size, 1, output_file);
        fclose(output_file);
    } else {
        fprintf(stderr, "**ERROR** Unable to write output file %s.\n", output_file_name);
    }
}

static f32 Random_Unilateral() {
    return static_cast<f32>(rand()) / static_cast<f32>(RAND_MAX);
}

static f32 Random_Bilateral() {
    f32 result = -1.0f + 2.0f*Random_Unilateral();
    return result; 
}

static vec3 Ray_Cast(World *world, vec3 ray_origin, vec3 ray_direction) {
    f32 min_hit_distance = 0.001f;
    vec3 result = {};
    
    vec3 attenuation = {1.0f, 1.0f, 1.0f};
    
    for (u32 ray_count = 0; ray_count < 8; ++ray_count) {
        f32 hit_distance = FLT_MAX;
        u32 hit_material_index = 0;
        vec3 next_origin = {};
        vec3 next_normal = {};

        // casting planes
        for (u32 i = 0; i < world->plane_count; ++i) {
            Plane this_plane = world->planes[i];
            f32 denom = Inner_Dot(this_plane.normal, ray_direction);
            if ((denom < -0.0001f) || (denom > 0.0001f)) { // 0 or very close means nothing was hit
                // ray plane intersection test
                f32 t = (-this_plane.d - Inner_Dot(this_plane.normal, ray_origin)) / denom; 
                if ((t > min_hit_distance) && (t < hit_distance)) {
                    hit_distance = t;
                    hit_material_index = this_plane.material_index;
                    next_origin = ray_origin + t*ray_direction;
                    next_normal = this_plane.normal;
                }
            }
        }
        
        // casting spheres
        for (u32 i = 0; i < world->sphere_count; ++i) {
            Sphere this_sphere = world->spheres[i];

            // sphere equation and ray equation form a quadratic collision equation
            vec3 sphere_relative_ray_origin = ray_origin - this_sphere.position; // (x-h)^2 + (y-k)^2 + (z-l)^2 = r^2
            f32 a = Inner_Dot(ray_direction, ray_direction);
            f32 b = 2.0f * Inner_Dot(ray_direction, sphere_relative_ray_origin);
            f32 c = Inner_Dot(sphere_relative_ray_origin, sphere_relative_ray_origin) - this_sphere.radius * this_sphere.radius;

            f32 denom = 2.0f*a;
            f32 root = static_cast<f32>(sqrt(b*b - 4*a*c));
            if (root > 0.0001f) { 
                // two solutions from two intersections
                f32 tp = (-b + root) / denom;
                f32 tn = (-b - root) / denom;

                f32 t = tp;
                if ((tn > min_hit_distance) && (tn < tp)) { // better hit because closer
                    t = tn;
                }
                if ((t > min_hit_distance) && (t < hit_distance)) { // but not too close
                    hit_distance = t;
                    hit_material_index = this_sphere.material_index;
                    next_origin = ray_origin + t*ray_direction;
                    next_normal = Normalise_Zero(t*ray_direction + sphere_relative_ray_origin); 
                }
            }
        }
        if (hit_material_index) { // if a hit was made at all, set new origin for bouncing cast
            Material mat = world->materials[hit_material_index];
            result += Hadamard(attenuation, mat.emit_color); // diminishing light propogated to camera after bouncing
            attenuation = Hadamard(attenuation, mat.reflect_color);
            ray_origin = next_origin;
            
            vec3 bounce = ray_direction - 2.0f*Inner_Dot(ray_direction, next_normal)*next_normal; // inner dot is distance on normal, add twice for axis reflection
            vec3 random_bounce = Normalise_Zero(next_normal + vec3{Random_Bilateral(), Random_Bilateral(), Random_Bilateral()});
            ray_direction = Normalise_Zero(Lerp(bounce, mat.specular, random_bounce));
        } else { 
            Material mat = world->materials[hit_material_index];
            result += Hadamard(attenuation, mat.emit_color);
            break;
        }
    }
    return result;
}

auto main(int argc, char **argv) -> int {
    Material materials[3] = {};
    materials[0].emit_color = {0.3f, 0.4f, 0.5f};    // sky
    materials[1].reflect_color = {0.5f, 0.5f, 0.5f}; // plane
    materials[2].reflect_color = {0.7f, 0.5f, 0.3f}; // object materials from here on out
    
    Plane planes[1] = {};
    planes[0].normal = vec3{0, 0, 1};
    planes[0].d = 0.0f;
    planes[0].material_index = 1;

    Sphere spheres[1] = {};
    spheres[0].position = vec3{0, 0, 0};
    spheres[0].radius = 1.0f;
    spheres[0].material_index = 2;
    
    World world = {};
    world.material_count = ARRAYCOUNT(materials);
    world.materials = materials;
    world.plane_count = ARRAYCOUNT(planes);
    world.planes = planes;
    world.sphere_count = ARRAYCOUNT(spheres);
    world.spheres = spheres;

    Image32 image = Allocate_Image(1920, 1280);    

    vec3 camera_position = vec3{0, -10, 1};
    vec3 camera_z = Normalise_Zero(camera_position);
    vec3 camera_x = Normalise_Zero(Cross_Product(vec3{0, 0, 1}, camera_z)); // camera-z axis crossed with universal z-axis (away x up).
    vec3 camera_y = Normalise_Zero(Cross_Product(camera_z, camera_x));

    // construct location and aperture
    f32 film_dist = 1.0f;
    f32 film_h = 1.0f;
    f32 film_w = 1.0f;

    // image aperture correction against dimensional ratios
    if (image.height < image.width) {
        film_h = film_w * (static_cast<f32>(image.height) / static_cast<f32>(image.width)); // width = height / width obtains height
    } else if (image.height > image.width) {
        film_w = film_h * (static_cast<f32>(image.width) / static_cast<f32>(image.height));
    }

    f32 rays_per_pixel = 8;
    f32 half_film_h = 0.5f*film_h;
    f32 half_film_w = 0.5f*film_w;
    vec3 film_centre = camera_position - camera_z * film_dist;

    u32 *out = image.pixels;
    for (u32 y = 0; y < image.height; ++y) {
        f32 film_y = -1.0f + 2.0f * static_cast<f32>(y) / static_cast<f32>(image.height); 
        for (u32 x = 0; x < image.width; ++x) {
            f32 film_x = -1.0f + 2.0f * static_cast<f32>(x) / static_cast<f32>(image.width);
          
            vec3 color = {};
            f32 contrib = 1.0f / static_cast<f32>(rays_per_pixel);
            for (u32 i = 0; i < rays_per_pixel; ++i) {
                // anti-aliasing here soon
                vec3 film_position = film_centre + (camera_x * half_film_w * film_x) + (camera_y * half_film_h * film_y); 
                vec3 ray_origin = camera_position;
                vec3 ray_direction = Normalise_Zero(film_position - camera_position);

                color += contrib*Ray_Cast(&world, ray_origin, ray_direction);
            }
            vec4 bmp_color = {255.0f * color, 255.0f};
            u32 bmp_value = Bgra_Pack_4x8(bmp_color);
            *out++ = bmp_value;
        }
        printf("\rRaycasting progress: %d%%", 100 * y / image.height);
        fflush(stdout);
    }
    Write_Image("test.bmp", image);
    printf("\n** Complete! **");
    return 0;
} 
