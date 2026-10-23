
import csv
from pathlib import Path

import matplotlib.pyplot as plt

ROOT = Path(__file__).resolve().parent

# ==========================================
# GRAPH 1: GPU CACHE CONFIGURATION COMPARISON
# ==========================================

csv_path = ROOT / "build" / "gpu_cache_sweep_results.csv"

if not csv_path.exists():
    csv_path = ROOT / "gpu_cache_sweep_results.csv"

if not csv_path.exists():
    raise FileNotFoundError(
        "GPU cache sweep CSV not found. "
        "Run GPUCacheConfigurationComparison first."
    )

with csv_path.open(newline="", encoding="utf-8") as file:
    rows = list(csv.DictReader(file))

warp_counts = [4, 8, 16, 32]
configurations = [
    "Baseline",
    "Config_A",
    "Config_B",
    "Config_C"
]

results = {
    config: {} for config in configurations
}

for row in rows:
    config = row["Configuration"]
    warps = int(row["Warps"])
    cycles = int(row["CompletionCycles"])

    if config in results:
        results[config][warps] = cycles

plt.figure(figsize=(10, 6))

for config in configurations:
    cycles = [
        results[config][warps]
        for warps in warp_counts
    ]

    plt.plot(
        warp_counts,
        cycles,
        marker="o",
        linewidth=2,
        label=config
    )

plt.title("GPU Cache Configuration Performance")
plt.xlabel("Number of Active Warps")
plt.ylabel("Simulated Completion Cycles")
plt.xticks(warp_counts)
plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()

plt.savefig(
    ROOT / "gpu_cache_configuration_performance.png",
    dpi=200
)

plt.close()

# ==========================================
# GRAPH 2: GPU MEMORY-LEVEL PARALLELISM
# ==========================================

capacities = [2, 4, 8, 16, 32]

# Numerical regression values verified by
# GPUMLPComparison.
completion_cycles = [
    51201,
    25603,
    12807,
    6415,
    3231
]

plt.figure(figsize=(10, 6))

plt.plot(
    capacities,
    completion_cycles,
    marker="o",
    linewidth=2
)

plt.title("GPU Memory-Level Parallelism Scaling")
plt.xlabel("Maximum Outstanding Memory Requests")
plt.ylabel("Simulated Completion Cycles")
plt.xticks(capacities)
plt.grid(True, alpha=0.3)
plt.tight_layout()

plt.savefig(
    ROOT / "gpu_mlp_completion_cycles.png",
    dpi=200
)

plt.close()

# ==========================================
# GRAPH 3: GPU MLP QUEUE STALL ANALYSIS
# ==========================================

queue_stalls = [
    50546,
    24948,
    12152,
    5760,
    0
]

plt.figure(figsize=(10, 6))

plt.bar(
    [str(value) for value in capacities],
    queue_stalls
)

plt.title("GPU Memory Queue Capacity vs Queue Stalls")
plt.xlabel("Maximum Outstanding Memory Requests")
plt.ylabel("Queue Stall Cycles")
plt.grid(axis="y", alpha=0.3)
plt.tight_layout()

plt.savefig(
    ROOT / "gpu_mlp_queue_stalls.png",
    dpi=200
)

plt.close()

print("GPU performance graphs generated successfully!")
print("1. gpu_cache_configuration_performance.png")
print("2. gpu_mlp_completion_cycles.png")
print("3. gpu_mlp_queue_stalls.png")
