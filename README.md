# C++ Cache Hierarchy and Performance Simulator

A trace-driven C++ simulator for exploring cache hierarchy behavior, replacement policies, memory locality, associativity, and matrix multiplication tiling.

The project implements a configurable two-level cache hierarchy and evaluates memory access patterns using hit/miss statistics and a simplified Average Memory Access Time (AMAT) model.

## Features

- Configurable L1 and L2 cache sizes and associativity
- FIFO, LRU, and RANDOM replacement policies
- Write-back and write-through policies
- Write allocation and dirty-block eviction handling
- Inclusive L1/L2 hierarchy with L1 invalidation on L2 eviction
- Trace-driven read and write simulation
- Sequential, random, repeated, and strided memory workloads
- Cache configuration and associativity comparisons
- Naive and tiled matrix multiplication memory traces
- CSV-based performance reports and Python visualization

## Architecture

```text
Memory Trace
     |
     v
Trace Reader
     |
     v
L1 Cache
     |
     v
L2 Cache
     |
     v
Main Memory

Statistics:
L1/L2 Hits, Misses, Write-backs
            |
            v
Performance Analysis
            |
            v
CSV Reports and Graphs
```

The simulator models cache blocks, sets, address decoding, replacement policies, write policies, and a backing-memory abstraction.

## Technology Stack

- C++20
- CMake
- GCC / MSYS2
- Python
- Matplotlib
- Git and GitHub

## Build Instructions

### Requirements

- C++20-compatible compiler
- CMake
- Python 3
- Matplotlib for visualization

### Build

From the project root:

```bash
cmake -S . -B build
cmake --build build
```

### Run Cache Hierarchy Tests

```powershell
.\build\CacheHierarchyTest.exe
```

### Run Replacement Policy Tests

```powershell
.\build\CacheReplacementTest.exe
```

### Generate Memory Workloads

```powershell
.\build\WorkloadGenerator.exe
.\build\GPUWorkloadGenerator.exe
.\build\MatrixWorkloadGenerator.exe
```

### Run Cache Configuration Comparison

```powershell
.\build\CacheComparison.exe traces\sample_trace.txt
```

### Run Replacement Policy Comparison

```powershell
.\build\CachePolicyComparison.exe traces\repeated.txt
```

### Run Associativity Comparison

```powershell
.\build\CacheAssociativityComparison.exe traces\stride_8.txt
```

### Generate Matrix Performance Reports

```powershell
python -m pip install matplotlib
python analyze_performance.py
```

## Performance Evaluation

The simulator was evaluated using sequential, random, repeated, strided, and matrix multiplication memory traces.

### Matrix Multiplication Results

The following table reports estimated total memory-access cycles for a 32×32 matrix multiplication workload.

| Workload | Small Cache | Medium Cache | Large Cache |
|---|---:|---:|---:|
| Naive | 4,265,920 | 3,986,080 | 3,914,240 |
| Tiled 4×4 | 5,462,784 | 971,044 | 419,824 |
| Tiled 8×8 | 4,701,952 | 2,676,572 | 446,332 |
| Tiled 16×16 | 4,431,616 | 4,164,416 | 1,603,156 |

For the Large cache configuration, 4×4 tiling reduced estimated memory-access cost by **89.27%** relative to the naive trace.

This result illustrates the effect of improved memory locality and cache reuse under the simulated configuration.

### Performance Graphs

![Matrix AMAT Comparison](matrix_amat.png)

![Matrix Total Memory Cycles](matrix_total_cycles.png)

## Performance Model

The simulator estimates AMAT using:

\[
AMAT = T_{L1} + MR_{L1}(T_{L2} + MR_{L2}T_{MEM})
\]

Where:

- L1 latency = 1 cycle
- L2 latency = 10 cycles
- Main memory latency = 100 cycles
- MR represents the corresponding cache miss rate

The total estimated memory-access cost is calculated from the number of L1 accesses, L1 misses, and L2 misses.

## Testing

The project has passed:

- 15 cache hierarchy regression assertions
- 10 FIFO/LRU replacement-policy assertions

These tests cover selected cache behavior, data preservation, dirty-block handling, and eviction order.

## Limitations

- This is a functional cache simulator with a simplified latency model, not a cycle-accurate processor or GPU simulator.
- The model does not include memory-level parallelism, bandwidth limitations, detailed pipeline timing, or complete write-back latency.
- The naive and tiled matrix traces use different output-matrix access patterns, so results represent their respective generated traces rather than a fully controlled end-to-end execution benchmark.
- GPU-specific features such as warp coalescing, shared memory, and GPU scheduling are not yet implemented.
- The tests cover selected scenarios and do not constitute exhaustive correctness verification.

## Future Improvements

- GPU warp-level memory coalescing
- Shared-memory bank conflict modelling
- Configurable GPU-inspired memory hierarchy
- Cache sensitivity studies across workloads
- Controlled matrix multiplication benchmarks
- More comprehensive automated regression testing

## Project Objective

This project was developed to strengthen practical understanding of Computer Architecture, memory hierarchy design, C++ systems programming, and performance modelling.

It provides a foundation for future work in GPU architecture and performance analysis.


## GPU Memory Performance Modelling

This project includes experimental GPU memory models implemented in C++20.

### 1. Warp Memory Coalescing

Models memory access patterns across a 32-thread warp using
32-byte memory sectors.

Supported patterns:
- Contiguous access
- Strided access
- Misaligned access

### 2. Shared Memory Bank Conflicts

Models 32 shared-memory banks with 4-byte bank words.

Demonstrates:
- Conflict-free accesses
- 2-way to 32-way bank conflicts
- Shared-memory broadcast behavior

### 3. GPU Matrix Transpose Optimization

Compares three memory-access scenarios:

| Method | Global Sectors | Bank Conflict |
|---|---:|---:|
| Naive transpose | 32 | 1 |
| Tiled transpose | 4 | 32 |
| Tiled transpose with padding | 4 | 1 |

These are analytical memory-access results, not measured
GPU execution times.

### 4. Integrated GPU Memory Model

Reusable C++ functions:
- `gpu::analyzeCoalescing()`
- `gpu::analyzeBankConflicts()`

The integrated model exports results to
`gpu_memory_results.csv`.

### 5. Performance Visualization

**Global memory efficiency**

![GPU Coalescing Efficiency](gpu_coalescing_efficiency.png)

**Shared-memory bank conflicts**

![GPU Bank Conflicts](gpu_bank_conflicts.png)

### Build and Run

```powershell
cmake -S . -B build
cmake --build build
.\build\GPUMemoryComparison.exe
python analyze_gpu_memory.py
```

### Testing

```powershell
ctest --test-dir build --output-on-failure
```

The GPU modules have four CTest regression checks.
