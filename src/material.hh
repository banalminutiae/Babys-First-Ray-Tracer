typedef union Material {

    struct {
        vec3 albedo;
    };

    struct {
        r32 refract_idx;

        // schlick's approximation for reflectance
        static r32 Reflectance(r32 cosine, r32 reflect_idx) {
            auto r0 = (1-reflect_idx) / (1+reflect_idx);
            r0 = SQUARE(r0);
            return r0 + (1-r0)*pow((1-cosine), 5);
        }
    };

}


