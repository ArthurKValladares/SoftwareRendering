# Software Rendering
CPU-only Software Rendererer, using hand-rolled SIMD types, and concurrency structures (i.e thread Pool, thread-safe queue, etc. Includes some simple debug features like wireframe drawing, overdraw display, depth buffer display, etc. Only supports obj models, and basic materials/diffuse textures. In orde to efficiently rasterize a larger number of triangles, we partition the screen space into several tiles and sort the triangles into their respective tiles, so the work of rasterizing them (using an efficient scanline technique) can be easily multi-treaded.

## Model Rendering
<img width="901" alt="teapot" src="https://github.com/ArthurKValladares/SoftwareRendering/assets/23410311/a85b28fc-2002-4b2c-8284-53a4c8af0fb1">
<img width="901" alt="box" src="https://github.com/ArthurKValladares/SoftwareRendering/assets/23410311/f18111d2-a1c7-48f1-b553-9fbebd16b287">

## Wireframe Rendering
<img width="901" alt="teapot_w" src="https://github.com/ArthurKValladares/SoftwareRendering/assets/23410311/3ea29f8f-2480-44bb-8696-bb0831c56b1e">
<img width="901" alt="box_w" src="https://github.com/ArthurKValladares/SoftwareRendering/assets/23410311/54eb6e46-7202-41b4-b01e-0b758b4f9eb8">

## Overdraw Indicator
Pixels in green were only written to once, pixels in red were written to more than once, either due the pixel being occluded by a triangle closer to the camera in the rendering process, or due to the edges of two connected triangles occupying overlapping pixels.
<img width="901" alt="teapot_o" src="https://github.com/ArthurKValladares/SoftwareRendering/assets/23410311/0d54d8ac-e9c8-4f6b-b960-a9d822b73158">

## Depth Buffer Rendering
<img width="901" alt="teapot_d" src="https://github.com/ArthurKValladares/SoftwareRendering/assets/23410311/d6e7fe39-c6e6-4936-8fff-7e358eef9a59">

## Building and Running

We provide a `build.bat` script for building on Windows, and a `build.sh` file for building on MacOS. The output files are saved to `${project_dir}/build`.
