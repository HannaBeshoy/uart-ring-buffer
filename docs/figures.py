"""Draws docs/img/uart_burst.svg from results/uart_burst.csv (run `make bench` first)."""
import csv
import os

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

HERE = os.path.dirname(os.path.abspath(__file__))
rows = list(csv.DictReader(open(os.path.join(HERE, "..", "results", "uart_burst.csv"))))
INK, GRID = "#3b4148", "#d9dde1"
COLORS = ["#d1495b", "#edae49", "#66a182", "#2e86ab", "#00303f"]
plt.rcParams.update({"font.size": 10, "axes.edgecolor": INK, "axes.labelcolor": INK, "xtick.color": INK,
                     "ytick.color": INK, "axes.grid": True, "grid.color": GRID, "grid.linewidth": 0.6,
                     "axes.spines.top": False, "axes.spines.right": False, "legend.frameon": False,
                     "svg.fonttype": "none"})
fig, ax = plt.subplots(figsize=(6.4, 3.4))
caps = sorted({int(r["capacity"]) for r in rows})
for cap, col in zip(caps, COLORS):
    pts = [(float(r["busy_ms"]), 100 * int(r["bytes_dropped"]) / int(r["bytes_sent"])) for r in rows if int(r["capacity"]) == cap]
    ax.plot([p[0] for p in pts], [p[1] for p in pts], marker="o", ms=4, color=col, label=f"{cap} bytes")
ax.axvline(64 / 11.52, color=INK, lw=0.8, ls=":")
ax.text(64 / 11.52 + 0.15, 9.5, "Arduino's 64-byte\nbuffer limit: 5.6 ms", fontsize=8, color=INK)
ax.set_xlabel("main loop busy every 100 ms (ms)")
ax.set_ylabel("bytes lost at 115200 baud (%)")
ax.legend(title="buffer size", fontsize=8, title_fontsize=8, loc="upper left")
fig.tight_layout()
fig.savefig(os.path.join(HERE, "img", "uart_burst.svg"), facecolor="white")
if os.environ.get("CHECK_PNG"):
    fig.savefig(os.path.join(os.environ["CHECK_PNG"], "uart_burst.png"), dpi=90)
print("wrote docs/img/uart_burst.svg")
