# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Breno Cunha Queiroz
"""Generate the animated Honeybee Democracy README banner.

The banner shows two centered lines, "Honeybee" and "Democracy". Instead of
plain text, each glyph is sampled into a cloud of bee-dots. Forwards in time
those dots do a correlated random walk away from the text (heading-persistent,
boundary-reflecting). The SVG animation replays that walk in *reverse* for the
first half of each cycle and forwards again for the second half, so the bees
appear to coalesce into the text, then scatter, then coalesce again.

Run:
    python honeybee_title.py            # writes honeybee_title.svg next to the script
    python honeybee_title.py -o foo.svg
"""

import argparse
import math
import random
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

# ── Canvas ────────────────────────────────────────────────────────────────────
W, H = 1200, 340

# ── Text ──────────────────────────────────────────────────────────────────────
LINES = ["Honeybee", "Democracy"]
FONT_SIZE = 120
LINE_GAP = 8

# ── Bee swarm ─────────────────────────────────────────────────────────────────
N_BEES = 2500          # bees sampled from text pixels (capped by available pixels)
N_FRAMES = 200         # forward simulation length
N_KEYFRAMES_PER_PHASE = 14  # SVG keyframes used during the converge and during the diverge
DOT_RADIUS = 2.4

# Random walk per step
STEP_SIZE = 4.5
HEADING_DRIFT_DEG = 18.0  # heading change stddev — small = persistent flight, large = jittery

# Animation cycle: scattered → text (converge), hold at text, text → scattered (diverge), loop.
CONVERGE_DURATION = 3.0
HOLD_DURATION = 5.0
DIVERGE_DURATION = 3.0
PERIOD = CONVERGE_DURATION + HOLD_DURATION + DIVERGE_DURATION  # seconds
SEED = 42

# Colors:
#   class "b" → GitHub light/dark text defaults (same convention as EventTrack)
#   class "y" → honey gold; deeper on light backgrounds, brighter on dark ones for contrast
BEE_FILL_LIGHT = "#1f2328"
BEE_FILL_DARK = "#d1d7e0"
BEE_YELLOW_LIGHT = "#B7791F"
BEE_YELLOW_DARK = "#FBBF24"

FONT_CANDIDATES = [
    "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    "/usr/share/fonts/dejavu/DejaVuSans-Bold.ttf",
    "/Library/Fonts/Arial Bold.ttf",
    "C:/Windows/Fonts/arialbd.ttf",
]


def find_font() -> str:
    for p in FONT_CANDIDATES:
        if Path(p).exists():
            return p
    raise RuntimeError("No bold font found; add yours to FONT_CANDIDATES")


def render_text_mask(font: ImageFont.FreeTypeFont) -> np.ndarray:
    """Rasterize the two centered lines; return a binary mask (uint8) of text pixels."""
    img = Image.new("L", (W, H), 0)
    draw = ImageDraw.Draw(img)

    metrics = []
    for line in LINES:
        bbox = draw.textbbox((0, 0), line, font=font)
        metrics.append({
            "bbox": bbox,
            "width": bbox[2] - bbox[0],
            "height": bbox[3] - bbox[1],
        })

    total_h = sum(m["height"] for m in metrics) + LINE_GAP * (len(LINES) - 1)
    y_top = (H - total_h) / 2

    cursor_y = y_top
    for line, m in zip(LINES, metrics):
        bbox = m["bbox"]
        # textbbox is relative to the position passed to draw.text; offset so the
        # bbox's top-left lands at (line_x_left, cursor_y).
        line_x_left = (W - m["width"]) / 2
        px = line_x_left - bbox[0]
        py = cursor_y - bbox[1]
        draw.text((px, py), line, fill=255, font=font)
        cursor_y += m["height"] + LINE_GAP

    return np.asarray(img)


def sample_bee_targets(mask: np.ndarray, n: int, rng: random.Random) -> list[tuple[float, float]]:
    """Pick `n` (x, y) positions sampled uniformly from the lit text pixels."""
    ys, xs = np.nonzero(mask > 128)
    if xs.size == 0:
        raise RuntimeError("Text mask is empty — check font/size/canvas")
    n = min(n, xs.size)
    indices = rng.sample(range(xs.size), n)
    return [(float(xs[i]), float(ys[i])) for i in indices]


def simulate_random_walks(targets: list[tuple[float, float]], rng: random.Random) -> list[list[tuple[float, float]]]:
    """For each target text pixel, run a heading-persistent random walk for
    `N_FRAMES` steps, reflecting off the canvas borders. Returns a list of
    trajectories of length N_FRAMES+1 — trajectory[0] is the text pixel,
    trajectory[-1] is the scattered final position."""
    heading_drift_rad = math.radians(HEADING_DRIFT_DEG)
    trajectories: list[list[tuple[float, float]]] = []

    for tx, ty in targets:
        heading = rng.uniform(0.0, 2 * math.pi)
        x, y = tx, ty
        traj: list[tuple[float, float]] = [(x, y)]
        for _ in range(N_FRAMES):
            heading += rng.gauss(0.0, heading_drift_rad)
            x += STEP_SIZE * math.cos(heading)
            y += STEP_SIZE * math.sin(heading)

            # Mirror-reflect off the canvas borders so bees stay in-frame.
            if x < 0:
                x = -x
                heading = math.pi - heading
            elif x > W:
                x = 2 * W - x
                heading = math.pi - heading
            if y < 0:
                y = -y
                heading = -heading
            elif y > H:
                y = 2 * H - y
                heading = -heading

            traj.append((x, y))

        trajectories.append(traj)

    return trajectories


def keyframe_indices(n: int) -> list[int]:
    """Evenly-spaced indices into a trajectory of length N_FRAMES+1, including endpoints."""
    return [int(round(i * N_FRAMES / (n - 1))) for i in range(n)]


def build_bee_circle(traj: list[tuple[float, float]], sample_idx: list[int], css_class: str) -> str:
    """Render one bee as <circle cx=0 cy=0> + an animateTransform that:
      • [0, t_converge_end]              : scattered → text (reverse of the forward walk)
      • [t_converge_end, t_diverge_start]: held at the text position (5s hold)
      • [t_diverge_start, PERIOD]        : text → scattered (forward walk replay)
    Loops with the same scattered endpoint at t=0 and t=PERIOD so the seam is invisible."""
    t_converge_end = CONVERGE_DURATION / PERIOD
    t_diverge_start = (CONVERGE_DURATION + HOLD_DURATION) / PERIOD

    coords: list[str] = []
    times: list[str] = []
    n = len(sample_idx)

    # Converge: scattered → text. sample_idx[0] = 0 → traj[-1] (scattered);
    # sample_idx[-1] = N_FRAMES → traj[0] (text).
    for k, idx in enumerate(sample_idx):
        pt = traj[-1 - idx]
        coords.append(f"{pt[0]:.2f},{pt[1]:.2f}")
        times.append(f"{t_converge_end * k / (n - 1):.4f}")

    # Hold: add an explicit keyframe at the text position at t_diverge_start.
    # Linear interpolation between this and the previous text-position keyframe
    # is a no-op, so the bee stays put for HOLD_DURATION.
    coords.append(f"{traj[0][0]:.2f},{traj[0][1]:.2f}")
    times.append(f"{t_diverge_start:.4f}")

    # Diverge: text → scattered. Skip k=0 since we just emitted the text position.
    for k in range(1, n):
        idx = sample_idx[k]
        pt = traj[idx]
        coords.append(f"{pt[0]:.2f},{pt[1]:.2f}")
        t = t_diverge_start + (1.0 - t_diverge_start) * k / (n - 1)
        times.append(f"{t:.4f}")

    return (
        f'<circle class="{css_class}" cx="0" cy="0" r="{DOT_RADIUS}">'
        f'<animateTransform attributeName="transform" type="translate" '
        f'values="{";".join(coords)}" keyTimes="{";".join(times)}" '
        f'dur="{PERIOD:.2f}s" repeatCount="indefinite"/>'
        f"</circle>"
    )


def build_svg() -> str:
    rng = random.Random(SEED)
    font = ImageFont.truetype(find_font(), FONT_SIZE)
    mask = render_text_mask(font)
    targets = sample_bee_targets(mask, N_BEES, rng)
    trajectories = simulate_random_walks(targets, rng)

    sample_idx = keyframe_indices(N_KEYFRAMES_PER_PHASE)

    out: list[str] = []
    out.append(
        f'<svg xmlns="http://www.w3.org/2000/svg" '
        f'viewBox="0 0 {W} {H}" width="{W}" height="{H}" role="img" '
        f'aria-label="Honeybee Democracy">'
    )
    out.append(
        "<style>"
        f".b {{ fill: {BEE_FILL_LIGHT}; }}"
        f".y {{ fill: {BEE_YELLOW_LIGHT}; }}"
        "@media (prefers-color-scheme: dark) {"
        f"  .b {{ fill: {BEE_FILL_DARK}; }}"
        f"  .y {{ fill: {BEE_YELLOW_DARK}; }}"
        "}"
        "</style>"
    )
    # Bee targets are already in random spatial order (rng.sample), so even/odd
    # indices give a roughly uniform spatial split between the two color classes.
    for i, traj in enumerate(trajectories):
        css_class = "y" if i % 2 == 0 else "b"
        out.append(build_bee_circle(traj, sample_idx, css_class))
    out.append("</svg>")
    return "\n".join(out)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    default_out = Path(__file__).parent / "honeybee_title.svg"
    ap.add_argument("-o", "--output", type=Path, default=default_out)
    args = ap.parse_args()
    args.output.write_text(build_svg())
    print(f"wrote {args.output}")


if __name__ == "__main__":
    main()
