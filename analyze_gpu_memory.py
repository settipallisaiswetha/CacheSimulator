
import csv
from pathlib import Path
import matplotlib.pyplot as plt

BASE_DIR = Path(__file__).resolve().parent
CSV_FILE = BASE_DIR / "gpu_memory_results.csv"

patterns = []
sectors = []
efficiencies = []
conflicts = []

with CSV_FILE.open("r", newline="") as file:
    reader = csv.DictReader(file)

    for row in reader:
        patterns.append(row["Pattern"])
        sectors.append(int(row["Sectors"]))
        efficiencies.append(float(row["EfficiencyPercent"]))
        conflicts.append(int(row["BankConflict"]))

# Graph 1: Global memory efficiency
plt.figure(figsize=(9, 5))
plt.plot(
    patterns,
    efficiencies,
    marker="o",
    linewidth=2
)
plt.title("GPU Global Memory Coalescing Efficiency")
plt.xlabel("Memory Access Pattern")
plt.ylabel("Efficiency (%)")
plt.ylim(0, 110)
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig(
    BASE_DIR / "gpu_coalescing_efficiency.png",
    dpi=300
)
plt.close()

# Graph 2: Shared memory bank conflicts
plt.figure(figsize=(9, 5))
plt.bar(patterns, conflicts)
plt.title("GPU Shared Memory Bank Conflicts")
plt.xlabel("Memory Access Pattern")
plt.ylabel("Maximum Bank Conflict Degree")
plt.grid(axis="y", alpha=0.3)
plt.tight_layout()
plt.savefig(
    BASE_DIR / "gpu_bank_conflicts.png",
    dpi=300
)
plt.close()

print("GPU performance graphs generated successfully!")
print("1. gpu_coalescing_efficiency.png")
print("2. gpu_bank_conflicts.png")
