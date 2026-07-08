# KyraseImageTools
KyraseImageTools is a C++ image-processing toolkit built from first principles.
The project currently focuses on loading image data, applying basic image operations, implementing blur and sharpening algorithms,
and benchmarking CPU performance. The long-term goal is to expand the toolkit to be performance-oriented and support single-threaded CPU, multithreaded CPU, and HIP/ROCm GPU options.

## Completed Roadmap Steps
This will be updated dynamically as steps get completed

## Roadmap
1. **PPM to internal image representation**
  Convert PPM images into image information (format, width, height, channels, max color value, and byte color values)
     
2. **Internal image representation to PPM**
  Write processed image data back to PPM for validation

3. **General image operation framework**
  Build a structure for image loading, operation selection, benchmarking, and result collection

4. **General blur operation**
  Implement a configurable blur operation

5. **Single-threaded CPU sharpener**
  Implement sharpening with configuration and settings using the blur operation

6. **Single-threaded CPU sharpener benchmarking and optimization**
  Measure runtime, pixels processed per second, image size impact, and performance using different blur and sharpen configurations

7. **Multithreaded CPU sharpener**
  Improve the sharpener with a CPU multithreaded implementation

8. **Multithreaded CPU sharpener benchmarking and optimization**
  Compare single-threaded and multithreaded performance and verify correctness using the testing framework

9. **PNG to internal image representation**
  Add PNG input support before JPEG because PNG is lossless

10. **Internal image representation to PNG**
  Add PNG output support for saving processed images

11. **JPEG to internal image representation**
  Add JPEG input support. Comparison is harder due to their lossiness

12. **Internal image representation to JPEG**
  Add JPEG output support for saving processed images

13. **HIP/ROCm sharpener**
  Implement a GPU-accelerated sharpener using HIP/ROCm

14. **HIP/ROCm sharpener benchmarking and optimization**
  Compare single-threaded CPU, multithreaded CPU, and GPU implementations

15. **Benchmark against common libraries**
  Compare single-threaded CPU, multithreaded CPU, and GPU implementations against other common libraries with similar features

16. **Downscaling**
  Implement image downscaling

17. **Upscaling**
  Implement image upscaling

18. **FXAA**
  Implement Fast Approximate Anti-Aliasing

19. **SMAA**
  Implement Subpixel Morphological Anti-Aliasing

20. **Offline/video versions**
  Extend operations to support offline image sequences and video processing

21. **Real-time versions**
  Add real-time versions of some operations

22. **TAA**
  Implement Temporal Anti-Aliasing

23. **Temporal upscaling**
  Implement temporal upscaling 

### Renderer Branch
1. **Renderer / triangle rasterizer**
  Build a small renderer to generate rasterized triangle output

2. **Basic aliasing demonstration**
  Use the renderer to demonstrate implemented aliasing techniques

3. **MSAA**
   Implement Multisample Anti-Aliasing in the renderer branch

### Bonus features
Bloom and other image filters may be added as additional learning and performance-comparison options.
