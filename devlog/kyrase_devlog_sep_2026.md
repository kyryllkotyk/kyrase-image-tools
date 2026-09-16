# September 2026

## September 16th, 2026

I decided to start this log today. There hasn't been an update on Kyrase in a bit as I was on vacation with my family! 

During the vacation, I got a very pleasant surprise in my inbox: "Thank you very much for your contribution to IEEE ICPADS 2026. Congratulations on your paper ID 8800 “A Comparison for C++ Cluster-Computing Libraries: MASS C++, HPX and MadMPI” accepted by IEEE ICPADS 2026 as a full paper."

So, it seems I'll be going to Tokyo to given an oral presentation 3 months from now. 

As Kyrase is partially a learning project, I will be making some changes to reflect what I need to understand to prepare for the conference. While I made most of the specifications and specified the features, I did not write anything in MASS or HPX, as that was my partner's job. Therefore, GPU implementation plans will now include the two backends.

### Changes to Kyrase development

**My original plan**:

1. Write naive implementations for a small selection of features (and their respective subfeatures, such as different types of blurring

2. Identify edge cases and pre-requisites of every public operation/subfeature

3. Create tests 
	1. Representative normal configurations for every public operation
	2. Differential tests against existing tools wherever possible (especially OpenCV)
	3. Edge-case tests (tiny images, unusual dimensions, weird radii, large kernels, and subfeature-specific)
	4. Separate edge-case testing for every supported border mode
	5. Invalid / bad input 
	6. Mathematical invariant and property tests (basically, the math itself is correct)
	7. Modifiable vs non-modifiable equivalence (currently they're linked, but just in case)
	8. Cross-implementation equivalence (currently only for Box blur, as it has naive, optimized, and multi)
	9. Pre/post-condition verification based on all functions' assumptions 

4. Automate tests with Catch2 OR GoogleTest

5. Write benchmarks
   	1. Record benchmark context
        	1. CPU / hardware used
        	2. Compiler and version
        	3. Build configuration and optimization flags
        	4. Image dimensions / test input
        	5. Exact algorithm settings for each benchmark
        	6. Number of timed runs
        	7. Number of warmup runs
        	8. Whether the benchmark used the modifying or non-modifying path
    2. Different configurations
    3. Modifying vs non-modifying
    4. Different edge modes
    5. Different pass counts (for subfeatures that have that)
    6. Use warmup runs before timed runs
    7. Reset input between runs
    8. Time the full operation
    9. Average the timed runs
    10. Compare output hashes
    11. Record benchmark times for each subfeature
    12. Keep the benchmark harness in the repository
    13. Compare optimization levels 
    14. Compare implementation types (single vs multi vs GPU)
    15. Record peak memory usage (optional)
    16. Add configuration options for a more reusable harness 

6. Automate benchmarks with Google Benchmark

7. Write optimized implementation & verify with automated tests

8. Write multithreaded implementation & verify with automated tests

9. Write GPU (MPI) implementation & verify with automated tests

10. Run benchmarks for the completed feature 

Repeat steps 2-10 for each feature

11. Release upon completion of the above steps

**New plan**:

I will not be making many changes to the per-feature plan, but my approach on which features to implement and when will reflect the concepts I am practicing.

- MASS and HPX added as backends for GPU implementation alongside MPI 
- Distributed implementations will take priority over non-naive CPU implementations
- GPU implementations will be deprioritized for now
- Benchmarking and heavy testing will be deprioritized for now
