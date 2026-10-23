
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

# =========================================
# CACHE COMPARISON
# =========================================

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

# =========================================
# WORKLOAD GENERATOR
# =========================================

add_executable(WorkloadGenerator
    src/workload_generator.cpp
)

# =========================================
# CACHE POLICY COMPARISON
# =========================================

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

# =========================================
# CACHE REPLACEMENT TEST
# =========================================

add_executable(CacheReplacementTest
    src/test_replacement.cpp
    src/Cache.cpp
    src/CacheSet.cpp
    src/CacheLine.cpp
    src/CacheStats.cpp
    src/Memory.cpp
    src/AddressDecoder.cpp
)

# =========================================
# GPU WORKLOAD GENERATOR
# =========================================

add_executable(GPUWorkloadGenerator
    src/gpu_workload_generator.cpp
)

# =========================================
# CACHE ASSOCIATIVITY COMPARISON
# =========================================

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

# =========================================
# MATRIX WORKLOAD GENERATOR
# =========================================

add_executable(MatrixWorkloadGenerator
    src/matrix_workload_generator.cpp
)

# =========================================
# GPU WARP COALESCING
# =========================================

add_executable(GPUWarpCoalescing
    src/warp_coalescing.cpp
)

# =========================================
# GPU SHARED MEMORY BANK CONFLICTS
# =========================================

add_executable(GPUSharedMemoryBanks
    src/shared_memory_banks.cpp
)

# =========================================
# GPU MATRIX TRANSPOSE
# =========================================

add_executable(GPUMatrixTranspose
    src/gpu_matrix_transpose.cpp
)

# =========================================
# ENABLE TESTING
# =========================================

enable_testing()

# =========================================
# GPU MEMORY MODEL REGRESSION TESTS
# =========================================

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
# GPU MEMORY COMPARISON
# =========================================

add_executable(GPUMemoryComparison
    src/gpu_memory_comparison.cpp
)

target_link_libraries(GPUMemoryComparison PRIVATE
    GPUMemoryModel
)

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
# GPU WARP SCHEDULER
# =========================================

add_executable(GPUWarpScheduler
    src/gpu_warp_scheduler.cpp
)

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

# =========================================
# GPU VARIABLE LATENCY MODEL
# =========================================

add_executable(GPUVariableLatency
    src/gpu_variable_latency.cpp
)

add_test(
    NAME GPUVariableLatencyTest
    COMMAND GPUVariableLatency
)

set_tests_properties(
    GPUVariableLatencyTest
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "32[ ]+692[ ]+256[ ]+436[ ]+36[.]99%"
)

# =========================================
# GPU SCHEDULER STALL ANALYSIS
# =========================================

add_executable(GPUStallAnalysis
    src/gpu_stall_analysis.cpp
)

add_test(
    NAME GPUStallAnalysisTest
    COMMAND GPUStallAnalysis
)

set_tests_properties(
    GPUStallAnalysisTest
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "32[ ]+692[ ]+436[ ]+0[ ]+0[ ]+68[ ]+368[ ]+36[.]99%"
)

# =========================================
# GPU CACHE LATENCY ADAPTER
# =========================================

add_library(GPULatencyAdapter STATIC
    src/GPULatencyAdapter.cpp
)

target_include_directories(GPULatencyAdapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# =========================================
# GPU LATENCY ADAPTER INTEGRATION TEST
# =========================================

add_executable(GPULatencyAdapterTest
    src/test_gpu_latency_adapter.cpp
    src/CacheHierarchy.cpp
    src/Cache.cpp
    src/CacheSet.cpp
    src/CacheLine.cpp
    src/CacheStats.cpp
    src/Memory.cpp
    src/AddressDecoder.cpp
)

target_link_libraries(GPULatencyAdapterTest PRIVATE
    GPULatencyAdapter
)

add_test(
    NAME GPULatencyAdapterIntegrationTest
    COMMAND GPULatencyAdapterTest
)

# =========================================
# GPU WARP MEMORY ACCESS LIBRARY
# =========================================

add_library(GPUWarpMemoryAccess STATIC
    src/GPUWarpMemoryAccess.cpp
)

target_include_directories(GPUWarpMemoryAccess PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(GPUWarpMemoryAccess PUBLIC
    GPULatencyAdapter
    GPUMemoryModel
)

# =========================================
# INTEGRATED GPU PERFORMANCE SIMULATOR
# =========================================

add_executable(GPUIntegratedSimulator
    src/gpu_integrated_scheduler.cpp
    src/CacheHierarchy.cpp
    src/Cache.cpp
    src/CacheSet.cpp
    src/CacheLine.cpp
    src/CacheStats.cpp
    src/Memory.cpp
    src/AddressDecoder.cpp
)

target_link_libraries(GPUIntegratedSimulator PRIVATE
    GPUWarpMemoryAccess
)

# =========================================
# INTEGRATED GPU SIMULATOR REGRESSION TEST
# =========================================

add_test(
    NAME GPUIntegratedSimulatorTest
    COMMAND GPUIntegratedSimulator
)




set_tests_properties(
    GPUIntegratedSimulatorTest
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "32[ ]+2832[ ]+3231[ ]+256[ ]+2576[ ]+0[ ]+0[ ]+512[ ]+1024[ ]+512[ ]+9[.]04[ ]+7[.]92%"
)

# =========================================
# GPU PERFORMANCE BOTTLENECK ANALYZER
# =========================================

add_library(GPUBottleneckAnalyzer STATIC
    src/GPUBottleneckAnalyzer.cpp
)

target_include_directories(GPUBottleneckAnalyzer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(GPUIntegratedSimulator PRIVATE
    GPUBottleneckAnalyzer
)

# =========================================
# GPU BOTTLENECK ANALYZER REGRESSION TEST
# =========================================

add_executable(GPUBottleneckAnalyzerTest
    src/test_gpu_bottleneck_analyzer.cpp
)

target_link_libraries(GPUBottleneckAnalyzerTest PRIVATE
    GPUBottleneckAnalyzer
)

add_test(
    NAME GPUBottleneckAnalyzerRegressionTest
    COMMAND GPUBottleneckAnalyzerTest
)

set_tests_properties(
    GPUBottleneckAnalyzerRegressionTest
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "All GPU bottleneck analyzer tests passed!"
)

# =========================================
# GPU CACHE CONFIGURATION COMPARISON
# =========================================

add_executable(GPUCacheConfigurationComparison
    src/gpu_cache_configuration_comparison.cpp
    src/CacheHierarchy.cpp
    src/Cache.cpp
    src/CacheSet.cpp
    src/CacheLine.cpp
    src/CacheStats.cpp
    src/Memory.cpp
    src/AddressDecoder.cpp
)

target_link_libraries(
    GPUCacheConfigurationComparison
    PRIVATE
    GPUWarpMemoryAccess
    GPUBottleneckAnalyzer
)

# GPU Cache Sweep Regression Test

find_program(POWERSHELL_EXECUTABLE
    NAMES pwsh powershell
)

if(POWERSHELL_EXECUTABLE)
    add_test(
        NAME GPUCacheSweepRegressionTest
        COMMAND ${POWERSHELL_EXECUTABLE}
            -NoProfile
            -ExecutionPolicy Bypass
            -File
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/verify_gpu_cache_sweep.ps1"
            -CsvPath
            "${CMAKE_CURRENT_BINARY_DIR}/gpu_cache_sweep_results.csv"
    )

    set_tests_properties(
        GPUCacheSweepRegressionTest
        PROPERTIES
        DEPENDS GPUCacheSweepGenerateTest
    )

    add_test(
        NAME GPUCacheSweepGenerateTest
        COMMAND GPUCacheConfigurationComparison
    )

    set_tests_properties(
        GPUCacheSweepGenerateTest
        PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
endif()

# =========================================
# GPU MEMORY-LEVEL PARALLELISM
# =========================================

add_library(GPUMemoryQueue STATIC
    src/GPUMemoryQueue.cpp
)

target_include_directories(GPUMemoryQueue PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(GPUMemoryQueueTest
    src/test_gpu_memory_queue.cpp
)

target_link_libraries(GPUMemoryQueueTest PRIVATE
    GPUMemoryQueue
)

add_test(
    NAME GPUMemoryQueueRegressionTest
    COMMAND GPUMemoryQueueTest
)

set_tests_properties(
    GPUMemoryQueueRegressionTest
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "All GPU memory queue tests passed!"
)
