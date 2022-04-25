#ifndef RAY_MATH_H
#define RAY_MATH_H

#include <math.h> // for sqrt and rand
#include <float.h>
#include <stdio.h>

#define SQUARE(x) ((x) * (x))
#define ABS(a) ((a) > 0 ? (a) : -(a))
#define ARRAYCOUNT(a) (sizeof(a) / sizeof((a)[0]))

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double r32;

constexpr float PI = 3.1415926f;
constexpr float TAU = 6.283185f;

typedef union vec2 {
    struct {
        float x, y;
    };

    struct {
        f32 u, v;
    };

    struct {
        f32 left, right;
    };

    struct {
        f32 width, height;
    };

    f32 elements[2];

    inline f32 &operator[](const int &index) {
        return elements[index];
    }
} vec2;

typedef union vec3 {
    struct {
        f32 x, y, z;
    };

    struct {
        f32 u, v, w;
    };

    struct {
        f32 r, g, b;
    };

    struct {
        vec2 xy;
        f32 ignored0_;
    };

    struct {
        f32 ignored1_;
        vec2 yz;
    };

    struct {
        vec2 uv;
        f32 ignored2_;
    };

    struct {
        f32 ignored3_;
        vec2 vw;
    };

    f32 elements[3];

    inline f32 &operator[](const int &index) {
        return elements[index];
    }
} vec3;

typedef union vec4 {
    struct {
        union {
            vec3 xyz;
            struct {
                f32 x, y, z;
            };
        };
        f32 w;
    };
    struct {
        union {
            vec3 rgb;
            struct {
                f32 r, g, b;
            };
        };
        f32 a;
    };

    struct {
        vec2 xy;
        f32 ignored0_;
        f32 ignored1_;
    };

    struct {
        f32 ignored2_;
        vec2 yz;
        f32 ignored3_;
    };

    struct {
        f32 ignored4_;
        f32 ignored5_;
        vec2 zw;
    };

    f32 elements[4];

    inline f32 &operator[](const int &index) {
        return elements[index];
    }
} vec4;

typedef union Mat4 {
    float elements[4][4];
    // __m128 columns[4];

    inline Mat4 operator[](const int &index) {
        float* col = elements[index];
        Mat4 result;
        
        result.elements[0] = col[0];
        result.elements[1] = col[1];
        result.elements[2] = col[2];
        result.elements[3] = col[3];
        
        return result;
    }
}


inline vec3 operator-(vec3 a, vec3 b) {
    vec3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

inline vec3 operator+(vec3 a, vec3 b) {
    vec3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

inline vec3 &operator+=(vec3 &a, vec3 b) {
    return a = a + b;
}

inline vec3 operator*(vec3 a, f32 b) {
    vec3 result;
    result.x = a.x*b;
    result.y = a.y*b;
    result.z = a.z*b;
    return result;
}

inline vec3 operator*(f32 a, vec3 b) {
    vec3 result;
    result.x = b.x*a;
    result.y = b.y*a;
    result.z = b.z*a;
    return result;
}

inline vec3 operator/(vec3 a, f32 b) {
    vec3 result;
    result.x = a.x/b;
    result.y = a.y/b;
    result.z = a.z/b;
    return result;
}

inline bool operator==(vec3 a, vec3 b) {
    return (a.x == b.x && a.y == b.y && a.z == b.z);
}

inline bool operator!=(vec3 a, vec3 b) {
    return (a.x != b.x && a.y != b.y && a.z != b.z);
}

inline f32 Inner_Dot(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 Cross_Product(vec3 a, vec3 b) {
    vec3 result;
    result.x = (a.y * b.z) - (a.z * b.y);
    result.y = (a.z * b.x) - (a.x * b.z);
    result.z = (a.x * b.y) - (a.y * b.x);
    return result;
}

// normalises vector to a unit vector
inline vec3 Normalise_Zero(vec3 a) {
    vec3 result = {};
    f32 len_sq = Inner_Dot(a, a);
    if (len_sq != 0.0f) {
        if (len_sq > SQUARE(0.0001f)) {
            // loop: v[i] / root: dot product
            result = a * (1.0f / static_cast<f32>(sqrt(len_sq)));
        }
    }
    return result;
}

inline u32 Round_R32_To_U32(r32 a) {
    return static_cast<u32>(a + 0.5f);
}

inline u32 Bgra_Pack_4x8(vec4 unpacked) {
    return ((Round_R32_To_U32(unpacked.a) << 24) |
            (Round_R32_To_U32(unpacked.r) << 16) |
            (Round_R32_To_U32(unpacked.g) << 8)  |
            (Round_R32_To_U32(unpacked.b) << 0));
}

inline vec3 Hadamard(vec3 a, vec3 b) {
    vec3 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    return result;
}

inline vec3 Lerp(vec3 a, f32 t, vec3 b) {
    return a*t + b*(1.0f - t);
}

#endif 
