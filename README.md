# C++ Cache Hierarchy and GPU Performance Simulator

A C++20, trace-driven cache hierarchy and **GPU-inspired memory performance modelling** project. It combines a configurable two-level cache hierarchy with analytical GPU memory-access models, a simplified warp scheduler, latency classification, bottleneck analysis, and controlled memory-level parallelism (MLP) experiments.

> **Scope:** This is an educational, GPU-inspired performance model—not a cycle-accurate simulator of any NVIDIA, AMD, or Apple GPU. Reported cycles and speedups are simulated estimates, not hardware measurements.

## Features

### Cache hierarchy and workloads
- Configurable L1/L2 cache sets and associativity; inclusive hierarchy with L1 invalidation on L2 eviction
- FIFO, LRU, and RANDOM replacement policies
- Write-back and write-through policies; write allocation and dirty eviction handling
- Trace-driven reads and writes, sequential/random/repeated/strided workloads
- Cache configuration, associativity, and replacement-policy comparisons
- Naive and tiled matrix multiplication memory traces
- CSV reports and Python/Matplotlib visualization

### GPU-inspired modelling
- **Warp coalescing:** 32-lane accesses grouped into 32-byte memory sectors
- **Shared-memory bank conflicts:** 32 banks with 4-byte bank words; broadcast and conflict examples
- **Matrix transpose:** comparison of naive, tiled, and padded shared-memory access patterns
- **Warp scheduling:** round-robin, single-issue scheduling and dependent instruction latencies
- **Cache-aware memory accesses:** warp addresses mapped to cache lines and classified as L1, L2, or DRAM accesses
- **GPU latency adapter:** illustrative L1/L2/DRAM latencies of 20/100/400 cycles
- **Bottleneck analysis:** issue-slot utilization, scheduler idle time, completion tail, and cache-level access fractions
- **Cache sweep:** 4 configurations × 4 warp counts, exporting 16 experiments to CSV
- **Memory request queue:** configurable outstanding-request capacity and completion tracking
- **MLP comparison:** capacities of 2, 4, 8, 16, and 32 with numerical regression checks

## Architecture

```text
                         Workload / Warp Address Generator
                                      |
                        +-------------+-------------+
                        |                           |
                 Trace-driven path              GPU-inspired path
                        |                           |
                  L1 Cache                  Warp Scheduler
                        |                           |
                  L2 Cache                Warp Memory Access
                        |                           |
                 Main Memory              Coalescing / Cache Lines
                                                    |
                                             GPU Latency Adapter
                                                    |
                                               L1 -> L2 -> DRAM

        Separate MLP experiment: Warps -> Finite Request Queue -> Completion Events

                   Statistics -> Bottleneck Analysis -> CSV / Console
```

The GPU integrated simulator and the MLP comparison are **separate experiments**. The MLP queue is not yet wired into the integrated cache-aware scheduler.

## Technology

C++20, CMake, GCC/MSYS2, PowerShell, Git/GitHub, Python 3, and Matplotlib (for visualization).

## Build and test

Requirements: C++20-compatible compiler and CMake. Python/Matplotlib are only needed for the plotting scripts.

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The latest local verification reported **16/16 CTest tests passing**. These include cache, GPU memory, scheduler, latency adapter, bottleneck analyzer, cache sweep, and MLP comparison tests. Numerical regression checks cover five MLP queue capacities. Tests exercise selected scenarios rather than exhaustively proving correctness.

### Useful executables

```powershell
# Integrated GPU performance experiment
.\build\GPUIntegratedSimulator.exe

# Cache configuration sweep (writes gpu_cache_sweep_results.csv)
.\build\GPUCacheConfigurationComparison.exe

# Outstanding-memory-request capacity experiment
.\build\GPUMLPComparison.exe

# Other cache and GPU experiments
.\build\CacheHierarchyTest.exe
.\build\CacheReplacementTest.exe
.\build\GPUMemoryComparison.exe
```

Some trace-driven comparisons expect a workload path:

```powershell
.\build\WorkloadGenerator.exe
.\build\GPUWorkloadGenerator.exe
.\build\MatrixWorkloadGenerator.exe
.\build\CacheComparison.exe traces\sample_trace.txt
.\build\CachePolicyComparison.exe traces\repeated.txt
.\build\CacheAssociativityComparison.exe traces\stride_8.txt
```

Optional plotting:

```powershell
python -m pip install matplotlib
python analyze_performance.py
python analyze_gpu_memory.py
```

## Experimental results

### 1. Matrix multiplication locality

Estimated total memory-access cycles for 32×32 matrix multiplication traces:

| Workload | Small cache | Medium cache | Large cache |
|---|---:|---:|---:|
| Naive | 4,265,920 | 3,986,080 | 3,914,240 |
| Tiled 4×4 | 5,462,784 | 971,044 | 419,824 |
| Tiled 8×8 | 4,701,952 | 2,676,572 | 446,332 |
| Tiled 16×16 | 4,431,616 | 4,164,416 | 1,603,156 |

For the large-cache configuration, the 4×4 tiled trace reduces estimated memory-access cost by **approximately 89.27%** versus the naive trace. The naive and tiled traces use different output-matrix access patterns; this is **not** a controlled end-to-end kernel speedup.

![Matrix AMAT comparison](matrix_amat.png)
![Matrix estimated memory cycles](matrix_total_cycles.png)

The original trace-driven AMAT model uses illustrative latencies of **L1 = 1**, **L2 = 10**, and **main memory = 100** cycles:

```text
AMAT = T_L1 + MR_L1 × (T_L2 + MR_L2 × T_MEM)
```

Here `MR_L2` is the conditional L2 miss rate among accesses reaching L2. This model is distinct from the GPU latency adapter's 20/100/400-cycle assumptions.

### 2. Warp coalescing and shared memory

| Transpose access pattern | Global memory sectors | Maximum bank conflict |
|---|---:|---:|
| Naive transpose | 32 | 1 |
| Tiled transpose | 4 | 32 |
| Tiled transpose with padding | 4 | 1 |

These are analytical access-pattern results, not measured GPU execution times. The reusable functions include `gpu::analyzeCoalescing()` and `gpu::analyzeBankConflicts()`.

![GPU coalescing efficiency](gpu_coalescing_efficiency.png)
![GPU shared-memory bank conflicts](gpu_bank_conflicts.png)

### 3. GPU cache configuration sweep

The sweep compares **Baseline, Config_A, Config_B, and Config_C** for 4, 8, 16, and 32 active warps, producing **16 CSV rows**.

| Warps | Configuration | L1 hits | L2 hits | DRAM accesses | Completion cycles |
|---:|---|---:|---:|---:|---:|
| 4 | Baseline | 0 | 32 | 32 | 2,003 |
| 4 | Config_A | 32 | 0 | 32 | 1,683 |
| 8 | Baseline | 0 | 0 | 128 | 3,207 |
| 8 | Config_B | 64 | 0 | 64 | 1,687 |
| 16 | Baseline | 0 | 0 | 256 | 3,215 |
| 16 | Config_C | 128 | 0 | 128 | 1,695 |
| 32 | Baseline | 0 | 0 | 512 | 3,231 |
| 32 | Config_C | 0 | 0 | 512 | 3,231 |

At 16 warps, Config_C reduces simulated completion cycles from 3,215 to 1,695 (**~47.3% reduction; ~1.90× speedup**). At 32 warps, the compared configurations produce identical completion cycles for this workload.

**Interpretation caveat:** The sweep increases associativity while keeping set counts fixed, so total cache capacity also changes. It does not isolate associativity from capacity.

### 4. Memory-level parallelism (MLP)

Experiment: 32 warps, 8 dependent requests per warp (256 total), fixed 400-cycle request latency, one request issued per cycle, finite outstanding-request queue.

| Queue capacity | Issue cycles | Completion cycles | Queue stall cycles | Peak outstanding |
|---:|---:|---:|---:|---:|
| 2 | 50,802 | 51,201 | 50,546 | 2 |
| 4 | 25,204 | 25,603 | 24,948 | 4 |
| 8 | 12,408 | 12,807 | 12,152 | 8 |
| 16 | 6,016 | 6,415 | 5,760 | 16 |
| 32 | 2,832 | 3,231 | 0 | 32 |

Increasing queue capacity from 2 to 32 reduces simulated completion cycles by **~93.7%** (a **~15.85×** speedup). At capacity 32, there are no queue-capacity stalls, but warps still experience dependency-related idle cycles.

## Modelling assumptions and limitations

- GPU-inspired functional/analytical model; **not** cycle-accurate and not calibrated to a specific commercial GPU.
- Integrated scheduler issues one instruction per cycle using round-robin selection; issue utilization measures the **modelled issue slot**, not physical SM occupancy or hardware utilization.
- Integrated warp memory access assumes perfect overlap of requests to distinct cache lines, using the maximum access latency as warp latency; it does not model DRAM queueing or bandwidth contention.
- Cache state is updated when accesses are modelled rather than upon asynchronous completion.
- The separate MLP comparison models finite outstanding requests and dependency delays, but uses a fixed memory latency without DRAM service contention, cache hierarchy, or bandwidth limits.
- Memory coalescing and bank conflict calculations use simplified 32-lane, 32-byte-sector and 32-bank assumptions.
- Cache sweep configurations vary both associativity and total cache capacity.
- Matrix multiplication results are trace-based estimated costs, not controlled wall-clock GPU benchmarks.

## Future improvements

- Integrate outstanding-request limits into the cache-aware warp scheduler
- Add finite DRAM bandwidth, queueing, and service contention
- Hold cache capacity constant during associativity sensitivity studies
- Add more workloads, parameterized configuration, and independent reference tests
- Add CI build/test automation and reproducible plots from CSV outputs

## Project objective

Develop practical skills in **computer architecture, GPU memory systems, C++ systems programming, experimental methodology, and performance modelling**. The project is intended as an educational and portfolio foundation for GPU architecture and performance engineering work.
