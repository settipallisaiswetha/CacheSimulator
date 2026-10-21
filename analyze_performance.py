
import csv
from pathlib import Path

import matplotlib.pyplot as plt

BASE = Path(__file__).resolve().parent

FILES = {
    "Naive": "matrix_naive_results.csv",
    "Tiled 4": "matrix_tiled_4_results.csv",
    "Tiled 8": "matrix_tiled_8_results.csv",
    "Tiled 16": "matrix_tiled_16_results.csv",
}

CONFIGS = ["Small", "Medium", "Large"]
WORKLOADS = list(FILES.keys())

results = {}

# Read existing C++ simulator results.
for workload, filename in FILES.items():
    path = BASE / filename

    if not path.exists():
        raise FileNotFoundError(
            f"Missing {filename}. Run CacheComparison first."
        )

    with path.open(newline="") as file:
        reader = csv.DictReader(file)

        for row in reader:
            config = row["Configuration"]
            accesses = int(row["Accesses"])
            l1_hits = int(row["L1Hits"])
            l2_hits = int(row["L2Hits"])
            l2_misses = int(row["L2Misses"])

            # Same simplified latency model as C++:
            # L1 lookup = 1 cycle
            # L2 lookup = 10 cycles on an L1 miss
            # RAM = 100 cycles on an L2 miss
            total_cycles = (
                accesses * 1
                + (accesses - l1_hits) * 10
                + l2_misses * 100
            )

            amat = total_cycles / accesses

            results[(workload, config)] = {
                "accesses": accesses,
                "l1_hit_rate": 100 * l1_hits / accesses,
                "l2_hit_rate": (
                    100 * l2_hits / (l2_hits + l2_misses)
                    if l2_hits + l2_misses else 0
                ),
                "amat": amat,
                "total_cycles": total_cycles,
            }

# Calculate improvement against naive for each cache size.
output_rows = []

for config in CONFIGS:
    baseline = results[("Naive", config)]["total_cycles"]

    for workload in WORKLOADS:
        result = results[(workload, config)]

        improvement = (
            100 * (baseline - result["total_cycles"])
            / baseline
        )

        output_rows.append({
            "Configuration": config,
            "Workload": workload,
            "Accesses": result["accesses"],
            "L1HitRatePercent": round(
                result["l1_hit_rate"], 2
            ),
            "L2HitRatePercent": round(
                result["l2_hit_rate"], 2
            ),
            "AMATCycles": round(result["amat"], 2),
            "TotalEstimatedCycles": result["total_cycles"],
            "ImprovementPercent": round(improvement, 2),
        })

# Save combined report.
output_csv = BASE / "matrix_performance_summary.csv"

with output_csv.open("w", newline="") as file:
    writer = csv.DictWriter(
        file,
        fieldnames=list(output_rows[0].keys())
    )
    writer.writeheader()
    writer.writerows(output_rows)

print("\nMATRIX PERFORMANCE SUMMARY")
print("-" * 75)

for row in output_rows:
    print(
        f"{row['Configuration']:8} "
        f"{row['Workload']:10} "
        f"AMAT={row['AMATCycles']:7.2f} "
        f"Cycles={row['TotalEstimatedCycles']:10} "
        f"Improvement={row['ImprovementPercent']:7.2f}%"
    )

# Plot 1: AMAT by workload.
x = range(len(WORKLOADS))
width = 0.25

fig, ax = plt.subplots(figsize=(10, 6))

for index, config in enumerate(CONFIGS):
    values = [
        results[(workload, config)]["amat"]
        for workload in WORKLOADS
    ]
    positions = [
        position + (index - 1) * width
        for position in x
    ]

    ax.bar(positions, values, width, label=config)

ax.set_xticks(list(x))
ax.set_xticklabels(WORKLOADS)
ax.set_ylabel("Estimated AMAT (cycles)")
ax.set_title("Matrix Multiplication: Cache AMAT Comparison")
ax.legend()
ax.grid(axis="y", alpha=0.25)

fig.tight_layout()
fig.savefig(BASE / "matrix_amat.png", dpi=200)
plt.close(fig)

# Plot 2: Total estimated memory-access cycles.
fig, ax = plt.subplots(figsize=(10, 6))

for index, config in enumerate(CONFIGS):
    values = [
        results[(workload, config)]["total_cycles"]
        / 1_000_000
        for workload in WORKLOADS
    ]
    positions = [
        position + (index - 1) * width
        for position in x
    ]

    ax.bar(positions, values, width, label=config)

ax.set_xticks(list(x))
ax.set_xticklabels(WORKLOADS)
ax.set_ylabel("Estimated memory cycles (millions)")
ax.set_title("Matrix Multiplication: Total Memory Cost")
ax.legend()
ax.grid(axis="y", alpha=0.25)

fig.tight_layout()
fig.savefig(BASE / "matrix_total_cycles.png", dpi=200)
plt.close(fig)

print("\nGenerated files:")
print("matrix_performance_summary.csv")
print("matrix_amat.png")
print("matrix_total_cycles.png")
