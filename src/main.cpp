#include "ray.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// there shouldn't be any instance of millions of objects being allocated
// but just in case, u32 (uint32_t) is used over s64 (int64_t).

static constexpr u32 Get_Total_Pixel_Size(Image32 image) {
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

static u32 Xor_Shift_Rand(Random_State *series) {
    u32 x = series->state; // series of self-xors
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    series->state = x;
    return x;
}

static f32 Random_Unilateral(Random_State *series) {
    return static_cast<f32>(Xor_Shift_Rand(series)) / static_cast<f32>(((u32)-1));
}

static f32 Random_Bilateral(Random_State *series) {
    return -1.0f + 2.0f*Random_Unilateral(series);
}

static inline vec3 Reflect_Ray(vec3 ray_direction, vec3 next_normal) {
    return ray_direction - 2.0f*Inner_Dot(ray_direction, next_normal)*next_normal; 
}

// TODO: Material differences
static inline void Refract_Ray(vec3 ray_direction, vec3 next_normal) {}

static vec3 Ray_Cast(World *world, vec3 ray_origin, vec3 ray_direction, Random_State *series) {
    f32 min_hit_distance = 0.001f;
    vec3 result = {};
    vec3 attenuation = {1.0f, 1.0f, 1.0f};
    u32 ray_bounce_limit = 8;
    for (u32 ray_count = 0; ray_count < ray_bounce_limit; ++ray_count) {
        f32 hit_distance = FLT_MAX;
        u32 hit_material_index = 0;
        vec3 next_origin = {};
        vec3 next_normal = {};

        // casting planes
        for (u32 i = 0; i < world->plane_count; ++i) {
            Plane this_plane = world->planes[i];
            f32 denom = Inner_Dot(this_plane.normal, ray_direction);
            // 0 or very close means nothing was hit; ignoring very close hits fixes the "shadow acne" problem
            if ((denom < -0.0001f) || (denom > 0.0001f)) { 
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
        // TODO: extract casting code into function, in function vectorize sphere data
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
            result += Hadamard(attenuation, mat.emit_colour); // diminishing light propogated to camera after bouncing
            
            attenuation = Hadamard(attenuation, mat.reflect_colour);
            ray_origin = next_origin;

            // inner dot is distance on normal, add twice for axis reflection
            // Lambertian Reflection?
            vec3 bounce = Reflect_Ray(ray_direction, next_normal);
            vec3 random_bounce = Normalise_Zero(next_normal + vec3{Random_Bilateral(series),
                                                                   Random_Bilateral(series),
                                                                   Random_Bilateral(series)});
            ray_direction = Normalise_Zero(Lerp(bounce, mat.specular, random_bounce));
        } else { 
            Material mat = world->materials[hit_material_index];
            result += Hadamard(attenuation, mat.emit_colour);
            break;
        }
    }
    return result;
}

// more accurate colour encoding
static f32 Exact_Linear_To_SRGB(f32 linear_value) {
    if (linear_value < 0.0f) linear_value = 0.0f;
    if (linear_value > 1.0f) linear_value = 1.0f;
    f32 s_value;
    if (linear_value <= 0.0031308) {
        s_value = linear_value * 12.92f;
    } else {
        s_value = 1.055f * static_cast<f32>(pow(linear_value, 1.0f/2.4f)) - 0.055f;
    }
    return s_value;
}

static void Load_Merl_Brdf(char *file_name, BRDF_Table *destination) {
    FILE *file = fopen("file_name", "rb");
    if (file) {
        fread(destination->count, sizeof(destination->count), 1, file);
        destination->values = (f32*)malloc(sizeof(f32));
    } else {
        fprintf(stderr, "** Error, could not load BRDF data from %s\n. **", file_name);
    }
    fclose(file);
}

int main(int argc, char **argv) {
    // novel for adding more materials but breaks when the struct alignment changes, also less clear
    Material materials[] = {
        // specular, emit, reflect
        // if it generates light then it won't reflect
        { 0,    {0.3f, 0.4f, 0.5f}, {                 }},
        { 0,    {                }, {0.5f , 0.5f, 0.5f}},// floor and ceiling
        { 0.6f, {                }, {0.7f , 0.5f, 0.3f}}, 
        { 0.0f, {1.0f, 1.0f, 1.0f}, {                 }},
        { 0.9f, {                }, {0.2f , 0.8f, 0.2f}},
        { 0,    {0.9f, 0.0f, 0.0f}, { 0.9f, 0.0f, 0.0f}},
    };
    
    Plane planes[] = {
        // normal, d, mat index
        {{0, 0, 1}, {0.0f}, 1},
    };

    Sphere spheres[] = {
        // position, radius, mat index
        {{ 0,  0, 0}, 1.0f, 2},
        {{ 0, -2, 2}, 0.2f, 3},
        {{-2, -1, 1}, 1.0f, 4},
        {{2, -1, 2}, 0.5f, 5},
    };

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

    Random_State *series = (Random_State *)malloc(sizeof(Random_State));   

    u32 rays_per_pixel;
    if (argc == 1) {
        rays_per_pixel = 128;
    } else if (argc == 2) {
        // atoi is deprecated because atoi("0") and atoi("abc") result in 0 while only
        // the former is meaningful while the latter is an error. In this case, both
        // are errors so atoi is fine
        rays_per_pixel = atoi(argv[1]);
        if (rays_per_pixel == 0) {
            rays_per_pixel = 128;
        }
    }
    printf("rays per pixel: %d", rays_per_pixel);


    f32 half_film_h = 0.5f*film_h;
    f32 half_film_w = 0.5f*film_w;
    vec3 film_centre = camera_position - camera_z * film_dist;

    f32 half_pixel_w = 0.5f/image.width;
    f32 half_pixel_h = 0.5f/image.height;
    u32 *out = image.pixels;
    for (u32 y = 0; y < image.height; ++y) {
        f32 film_y = -1.0f + 2.0f * static_cast<f32>(y) / static_cast<f32>(image.height); 
        for (u32 x = 0; x < image.width; ++x) {
            f32 film_x = -1.0f + 2.0f * static_cast<f32>(x) / static_cast<f32>(image.width);
          
            vec3 colour = {};
            f32 contrib = 1.0f / static_cast<f32>(rays_per_pixel);
            for (u32 i = 0; i < rays_per_pixel; ++i) {
                // in-pipe anti-aliasing, scatter a pixel in either direction
                f32 off_x = film_x + Random_Bilateral(series) * half_pixel_w;
                f32 off_y = film_y + Random_Bilateral(series) * half_pixel_h;
                vec3 film_position = film_centre + (camera_x * half_film_w * off_x) + (camera_y * half_film_h * off_y); 
                vec3 ray_origin = camera_position;
                vec3 ray_direction = Normalise_Zero(film_position - camera_position);

                colour += contrib*Ray_Cast(&world, ray_origin, ray_direction, series);
            }
            vec4 bmp_colour = {
                255.0f*Exact_Linear_To_SRGB(colour.r),
                255.0f*Exact_Linear_To_SRGB(colour.g),
                255.0f*Exact_Linear_To_SRGB(colour.b),
                255.0f
            };
            u32 bmp_value = Bgra_Pack_4x8(bmp_colour);
            *out++ = bmp_value;
        }
        printf("\rRaycasting progress: %d%%", 100 * y / image.height);
        fflush(stdout);
    }
    Write_Image("test.bmp", image);
    printf("\n <><> DONE <><>><><><><><><><><><><><><><><><>");
    return 0;
} 
