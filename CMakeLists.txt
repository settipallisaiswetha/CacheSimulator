cmake_minimum_required(VERSION 3.20)

project(CacheSimulator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================================
# MAIN CACHE SIMULATOR
# =========================================

add_executable(CacheSimulator
    src/main.cpp
    src/CacheLine.cpp
    src/CacheSet.cpp
    src/Cache.cpp
    src/AddressDecoder.cpp
    src/CacheStats.cpp
    src/Memory.cpp
    src/CacheHierarchy.cpp
)

# =========================================
# L1 / L2 CACHE HIERARCHY TEST
# =========================================

add_executable(CacheHierarchyTest
    src/test_hierarchy.cpp
    src/CacheHierarchy.cpp
    src/Cache.cpp
    src/CacheSet.cpp
    src/CacheLine.cpp
    src/CacheStats.cpp
    src/Memory.cpp
    src/AddressDecoder.cpp
)



add_executable(CacheComparison
    src/compare_main.cpp
    src/TraceReader.cpp
    src/CacheHierarchy.cpp
    src/Cache.cpp
    src/CacheSet.cpp
    src/CacheLine.cpp
    src/CacheStats.cpp
    src/Memory.cpp
    src/AddressDecoder.cpp
)

add_executable(WorkloadGenerator
    src/workload_generator.cpp
)

add_executable(CachePolicyComparison
    src/policy_compare.cpp
    src/TraceReader.cpp
    src/CacheHierarchy.cpp
    src/Cache.cpp
    src/CacheSet.cpp
    src/CacheLine.cpp
    src/CacheStats.cpp
    src/Memory.cpp
    src/AddressDecoder.cpp
)

add_executable(CacheReplacementTest
    src/test_replacement.cpp
    src/Cache.cpp
    src/CacheSet.cpp
    src/CacheLine.cpp
    src/CacheStats.cpp
    src/Memory.cpp
    src/AddressDecoder.cpp
)

add_executable(GPUWorkloadGenerator
    src/gpu_workload_generator.cpp
)

add_executable(CacheAssociativityComparison
    src/associativity_compare.cpp
    src/TraceReader.cpp
    src/CacheHierarchy.cpp
    src/Cache.cpp
    src/CacheSet.cpp
    src/CacheLine.cpp
    src/CacheStats.cpp
    src/Memory.cpp
    src/AddressDecoder.cpp
)

add_executable(MatrixWorkloadGenerator
    src/matrix_workload_generator.cpp
)

add_executable(GPUWarpCoalescing
    src/warp_coalescing.cpp
)

add_executable(GPUSharedMemoryBanks
    src/shared_memory_banks.cpp
)

add_executable(GPUMatrixTranspose
    src/gpu_matrix_transpose.cpp
)

# =========================================
# GPU MEMORY MODEL REGRESSION TESTS
# =========================================

enable_testing()

add_test(
    NAME GPUWarpCoalescingTest
    COMMAND GPUWarpCoalescing
)

add_test(
    NAME GPUSharedMemoryBanksTest
    COMMAND GPUSharedMemoryBanks
)

add_test(
    NAME GPUMatrixTransposeTest
    COMMAND GPUMatrixTranspose
)


# =========================================
# GPU NUMERICAL REGRESSION CHECKS
# =========================================

set_tests_properties(
    GPUWarpCoalescingTest
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Misaligned[ ]+5[ ]+128[ ]+160[ ]+80[.]00%"
)

set_tests_properties(
    GPUSharedMemoryBanksTest
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Stride 32[ ]+32[ ]+1[ ]+1"
)

set_tests_properties(
    GPUMatrixTransposeTest
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Tiled [(]padding[)][ ]+4[ ]+1[ ]+100[.]00%"
)

# =========================================
# REUSABLE GPU MEMORY MODEL LIBRARY
# =========================================

add_library(GPUMemoryModel STATIC
    src/GPUMemoryModel.cpp
)

target_include_directories(GPUMemoryModel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# =========================================
# INTEGRATED GPU MEMORY COMPARISON
# =========================================

add_executable(GPUMemoryComparison
    src/gpu_memory_comparison.cpp
)

target_link_libraries(GPUMemoryComparison PRIVATE
    GPUMemoryModel
)

# =========================================
# INTEGRATED GPU MODEL TEST
# =========================================

add_test(
    NAME GPUMemoryComparisonTest
    COMMAND GPUMemoryComparison
)

set_tests_properties(
    GPUMemoryComparisonTest
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Stride 32[ ]+32[ ]+12[.]50[ ]+32"
)

# =========================================
# CACHE HIERARCHY REGRESSION TEST
# =========================================

add_test(
    NAME CacheHierarchyRegressionTest
    COMMAND CacheHierarchyTest
)

set_tests_properties(
    CacheHierarchyRegressionTest
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Passed: 15"
    FAIL_REGULAR_EXPRESSION
    "Failed: [1-9][0-9]*"
)


# =========================================
# CACHE REPLACEMENT REGRESSION TEST
# =========================================

add_test(
    NAME CacheReplacementRegressionTest
    COMMAND CacheReplacementTest
)

set_tests_properties(
    CacheReplacementRegressionTest
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Passed: 10"
    FAIL_REGULAR_EXPRESSION
    "Failed: [1-9][0-9]*"
)

# =========================================
# GPU WARP SCHEDULING MODEL
# =========================================

add_executable(GPUWarpScheduler
    src/gpu_warp_scheduler.cpp
)

# =========================================
# GPU WARP SCHEDULER REGRESSION TEST
# =========================================

add_test(
    NAME GPUWarpSchedulerTest
    COMMAND GPUWarpScheduler
)

set_tests_properties(
    GPUWarpSchedulerTest
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "8[ ]+80[ ]+80[ ]+0[ ]+100[.]00%"
)
