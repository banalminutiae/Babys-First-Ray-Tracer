# Babys-First-Ray-Tracer

A basic implementation of using ray tracing to draw interesting graphics to an image. A bitmap is written and saved rather than an OS specific window to remain reasonably system agnostic.

Learning materials used include the [Ray Tracing in One Weekend Series](https://raytracing.github.io/) and [Handmade Hero](https://handmadehero.org/) on youtube.

Lambertian reflection quality is accurate but only at very high rays per pixel. Rudimentary anti-aliasing is implemented.

Currently, materials are limited to magnitude of specularity. 

TODO:
- Refraction as an alternative to reflection for glass properties; currently only metallic materials are supported
- Load BRDF data to objects.
- Multithreading and SIMD: At 1024 rays per pixel the image can take up to half an hour to write to.
- More realistic lighting calculations alongisde authentic light sampling.
- Proper camera aperture code.
- Accomodate more objects than simple spheres and planes with the goal of implementing the Cornell box.

# Usage 
- Run build.bat on a windows sytem. This compiles and runs the program, opening the outputted image. This requires right environment variables for the native windows compiler to be configured first, either through VCVarsall.bat or a visual studio developers terminal.
- There is no usage of the Windows API or any external library, so any reasonable C++ compiler is feasible if compiling on OSX or Linux.

# Result 
![BMP](./data/test.bmp)
