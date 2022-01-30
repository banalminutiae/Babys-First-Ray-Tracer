# Babys-First-Ray-Tracer

A basic implementation of using ray tracing to draw interesting graphics to an image. A bitmap is written and saved rather than an OS specific window to remain reasonably system independant.

Learning materials used include the [Ray Tracing in One Weekend Series](https://raytracing.github.io/) and [Handmade Hero](https://handmadehero.org/) on youtube.

Reflection quality is accurate but only at very high rays per pixel.

Currently, materials are limited to magnitude of specularity. 

TODO:
- Load BRDF data to objects.
- Multithreading and SIMD: At 1024 rays per pixel the image can take up to an hour to write to.
- More realistic lighting calculations.
- Accomodate more objects than simple spheres and planes with the goal of implementing the Cornell box.

![BMP](./data/test.bmp)
