# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Breno Cunha Queiroz
"""Generate an animated SVG of a honeybee performing the waggle dance.

Anatomy of the dance (Karl von Frisch):

    ┌── return loop ─────╮         ╭───── return loop ──┐
    │                     ╲       ╱                     │
    │                      ╲     ╱                      │
    │           waggle run  ║ ║ ║                       │
    │                       ║ ║ ║                       │
    │                       ║ ║ ║   ← rapid abdomen     │
    │                       ║ ║ ║      wiggling here    │
    │                      ╱     ╲                      │
    │                     ╱       ╲                     │
    └────────────────────╯         ╰────────────────────┘

Direction encoding: the waggle-run angle relative to vertical (gravity on the
comb) equals the angle from the sun to the target. Distance encoding: longer
waggle run = farther target. Quality encoding: more repeats / more vigor.

We tilt the waggle by DANCE_ANGLE_DEG just to make the directionality visible
in the figure. Run:

    python bee_dance.py            # writes bee_dance.svg next to the script
    python bee_dance.py -o foo.svg
"""

import argparse
import math
from pathlib import Path

# ── Canvas ────────────────────────────────────────────────────────────────────
W, H = 520, 340

# ── Dance geometry ────────────────────────────────────────────────────────────
DANCE_ANGLE_DEG = 28              # waggle angle from vertical (clockwise)
WAGGLE_LENGTH = 140               # straight-run length (px); ~= distance to target
RETURN_RADIUS = 44                # how far the return loops bulge out (px)
DANCE_CENTER = (W // 2, H // 2 + 15)

# Side-to-side abdomen wiggle layered on top of the straight waggle motion.
WAGGLE_JITTER_AMP = 5.0           # px perpendicular to the waggle direction
WAGGLE_JITTER_FREQ = 5            # full wiggle cycles per traversal of the run

# ── Sampling resolution ───────────────────────────────────────────────────────
N_WAGGLE_SAMPLES = 80             # high so the abdomen wiggle reads smoothly
N_RETURN_SAMPLES = 36

# Fraction of each cycle spent in each phase (must sum to 1.0).
WAGGLE_FRAC = 0.30                # each waggle run
RETURN_FRAC = 0.20                # each return loop

# ── Animation ─────────────────────────────────────────────────────────────────
PERIOD = 4.0                      # seconds per full figure-eight cycle

# ── Colors (GitHub light/dark + honey amber) ──────────────────────────────────
TEXT_LIGHT = "#1f2328"
TEXT_DARK = "#d1d7e0"
BEE_BODY_LIGHT = "#B7791F"
BEE_BODY_DARK = "#FBBF24"
BEE_STRIPE_LIGHT = "#1f2328"
BEE_STRIPE_DARK = "#101218"
TRAIL_LIGHT = "#9aa4ad"
TRAIL_DARK = "#4d5862"


def local_to_world(dx: float, dy: float) -> tuple[float, float]:
    """Dance-local (dx along perpendicular, dy along waggle direction) → screen coords.

    Dance "up" tilts clockwise from screen up by DANCE_ANGLE_DEG. Screen y grows
    downward, so screen-up = -y.
    """
    a = math.radians(DANCE_ANGLE_DEG)
    up_x, up_y = math.sin(a), -math.cos(a)
    right_x, right_y = math.cos(a), math.sin(a)
    cx, cy = DANCE_CENTER
    return (cx + dx * right_x + dy * up_x, cy + dx * right_y + dy * up_y)


def compute_dance_path() -> list[tuple[float, float]]:
    """One full figure-eight cycle, dense enough for animateMotion's tangent
    rotation to capture the abdomen wiggle as visible body rocking."""
    pts: list[tuple[float, float]] = []
    L = WAGGLE_LENGTH
    R = RETURN_RADIUS

    def waggle(samples: int, phase: float, include_start: bool) -> None:
        start = 0 if include_start else 1
        for i in range(start, samples):
            u = i / (samples - 1)
            dy = -L / 2 + L * u
            dx = WAGGLE_JITTER_AMP * math.sin(2 * math.pi * WAGGLE_JITTER_FREQ * u + phase)
            pts.append(local_to_world(dx, dy))

    def return_loop(samples: int, side: int) -> None:
        # side = +1 for right loop, -1 for left loop. We come in from the top
        # of the waggle (0, +L/2), arc out via (side*R, 0), and rejoin the
        # bottom (0, -L/2) — ready for the next waggle to start.
        for i in range(1, samples):
            u = i / (samples - 1)
            theta = math.pi * u
            dx = side * R * math.sin(theta)
            dy = (L / 2) * math.cos(theta)
            pts.append(local_to_world(dx, dy))

    waggle(N_WAGGLE_SAMPLES, phase=0.0, include_start=True)
    return_loop(N_RETURN_SAMPLES, side=+1)
    waggle(N_WAGGLE_SAMPLES, phase=math.pi, include_start=False)  # phase-shift second run for variety
    return_loop(N_RETURN_SAMPLES, side=-1)
    return pts


def path_d_attr(pts: list[tuple[float, float]]) -> str:
    parts: list[str] = []
    for i, (x, y) in enumerate(pts):
        parts.append(f"{'M' if i == 0 else 'L'}{x:.2f},{y:.2f}")
    parts.append("Z")
    return " ".join(parts)


def keytimes_for_phases(n_pts: int) -> str:
    """Build a keyTimes string so the four phases occupy their intended fractions.

    The list has n_pts entries from 0 to 1. Phase boundaries are at
    cumulative fractions [W, W+R, 2W+R, 2W+2R].
    """
    # Reconstruct how many samples each phase contributed in compute_dance_path.
    counts = [N_WAGGLE_SAMPLES, N_RETURN_SAMPLES - 1, N_WAGGLE_SAMPLES - 1, N_RETURN_SAMPLES - 1]
    assert sum(counts) == n_pts
    fracs = [WAGGLE_FRAC, RETURN_FRAC, WAGGLE_FRAC, RETURN_FRAC]
    starts = [0.0, WAGGLE_FRAC, WAGGLE_FRAC + RETURN_FRAC, 2 * WAGGLE_FRAC + RETURN_FRAC]

    times: list[str] = []
    idx = 0
    for count, frac, start in zip(counts, fracs, starts):
        for j in range(count):
            # Map [0..count-1] within the phase. The first sample of the very
            # first phase is t=0; for later phases the "first sample" we kept
            # is the one *after* the boundary, so it shouldn't land exactly on
            # the boundary — but with this scheme it does (effectively a held
            # frame at the boundary). That's fine for visual purposes.
            denom = count if idx != 0 else count - 1
            u = j / denom if denom > 0 else 0.0
            times.append(f"{start + frac * u:.4f}")
            idx += 1
    return ";".join(times)


def bee_def() -> str:
    """A small bee drawn with its head pointing along +x so animateMotion's
    rotate='auto' aligns it with the path tangent (so the body rocks side-to-
    side during the waggle run, mimicking the abdomen wiggle)."""
    return (
        '<g id="bee">'
        # Wings (light fill, semi-transparent so they tint over the body)
        '<ellipse cx="-3" cy="-5" rx="6" ry="3" fill="white" opacity="0.55"/>'
        '<ellipse cx="-3" cy="5" rx="6" ry="3" fill="white" opacity="0.55"/>'
        # Body
        '<ellipse cx="0" cy="0" rx="11" ry="5.5" class="bee-body"/>'
        # Three stripes across the abdomen
        '<rect x="-7" y="-5.5" width="2" height="11" class="bee-stripe"/>'
        '<rect x="-3" y="-5.5" width="2" height="11" class="bee-stripe"/>'
        '<rect x="1" y="-5.5" width="2" height="11" class="bee-stripe"/>'
        # Head
        '<circle cx="10" cy="0" r="3" class="bee-stripe"/>'
        "</g>"
    )


def annotations() -> str:
    """Static labels and reference lines around the dance."""
    cx, cy = DANCE_CENTER
    a = math.radians(DANCE_ANGLE_DEG)
    L = WAGGLE_LENGTH
    R = RETURN_RADIUS

    # Vertical "gravity / sun direction" reference line.
    vert_top = (cx, 28)
    vert_bot = (cx, cy + L / 2 + 6)

    # Tip of the waggle run (= where the dance points if extrapolated).
    waggle_tip = local_to_world(0, L / 2 + 24)

    # Angle arc between vertical and waggle direction (drawn at radius 36
    # around the dance center, swept from -90° to -90°+DANCE_ANGLE_DEG
    # in screen-rotation degrees).
    arc_r = 38
    arc_start = (cx, cy - arc_r)
    arc_end = (cx + arc_r * math.sin(a), cy - arc_r * math.cos(a))
    arc_label_x, arc_label_y = (cx + 8, cy - arc_r - 6)

    # A label-lead from the right return loop out into the right margin.
    rl_anchor = local_to_world(R, 0)
    rl_label = (rl_anchor[0] + 36, rl_anchor[1] + 6)

    # A label-lead from the waggle run out into the left margin.
    wg_anchor = local_to_world(0, 0)
    wg_label = (wg_anchor[0] - 130, wg_anchor[1] + 5)

    return "".join([
        # Vertical reference, dashed.
        f'<line x1="{vert_top[0]}" y1="{vert_top[1]}" x2="{vert_bot[0]}" y2="{vert_bot[1]}" '
        'class="ref" stroke-dasharray="4,4"/>',
        # Small "↑ sun" label above the vertical line.
        f'<text x="{vert_top[0]}" y="{vert_top[1] - 8}" class="anno" text-anchor="middle">'
        '↑ sun direction (= gravity on the comb)</text>',
        # Solid line pointing along the waggle direction out past the figure.
        f'<line x1="{cx}" y1="{cy}" x2="{waggle_tip[0]:.2f}" y2="{waggle_tip[1]:.2f}" '
        'class="ref-solid"/>',
        # Tiny arrowhead at the waggle tip.
        f'<circle cx="{waggle_tip[0]:.2f}" cy="{waggle_tip[1]:.2f}" r="3" class="ref-dot"/>',
        # Angle arc.
        f'<path d="M{arc_start[0]:.2f},{arc_start[1]:.2f} '
        f'A{arc_r},{arc_r} 0 0 1 {arc_end[0]:.2f},{arc_end[1]:.2f}" '
        'class="ref" fill="none"/>',
        f'<text x="{arc_label_x:.2f}" y="{arc_label_y:.2f}" class="anno">'
        f'{DANCE_ANGLE_DEG}° = angle to target</text>',
        # Waggle-run label on the left.
        f'<text x="{wg_label[0]:.2f}" y="{wg_label[1]:.2f}" class="anno" text-anchor="end">'
        'waggle run</text>',
        f'<line x1="{wg_label[0] + 6:.2f}" y1="{wg_label[1] - 4:.2f}" '
        f'x2="{wg_anchor[0] - 4:.2f}" y2="{wg_anchor[1]:.2f}" class="lead"/>',
        # Return-loop label on the right.
        f'<text x="{rl_label[0]:.2f}" y="{rl_label[1]:.2f}" class="anno">'
        'return loop</text>',
        f'<line x1="{rl_label[0] - 4:.2f}" y1="{rl_label[1] - 4:.2f}" '
        f'x2="{rl_anchor[0] + 4:.2f}" y2="{rl_anchor[1]:.2f}" class="lead"/>',
    ])


def build_svg() -> str:
    pts = compute_dance_path()
    d = path_d_attr(pts)
    times = keytimes_for_phases(len(pts))

    parts: list[str] = []
    parts.append(
        f'<svg xmlns="http://www.w3.org/2000/svg" '
        f'viewBox="0 0 {W} {H}" width="{W}" height="{H}" role="img" '
        f'aria-label="Honeybee waggle dance">'
    )
    parts.append(
        "<style>"
        # Defaults (light mode)
        f".bee-body {{ fill: {BEE_BODY_LIGHT}; }}"
        f".bee-stripe {{ fill: {BEE_STRIPE_LIGHT}; }}"
        f".trail {{ stroke: {TRAIL_LIGHT}; fill: none; stroke-width: 1.4; }}"
        f".ref {{ stroke: {TRAIL_LIGHT}; stroke-width: 1.2; fill: none; }}"
        f".ref-solid {{ stroke: {BEE_BODY_LIGHT}; stroke-width: 1.6; }}"
        f".ref-dot {{ fill: {BEE_BODY_LIGHT}; }}"
        f".lead {{ stroke: {TRAIL_LIGHT}; stroke-width: 1.0; }}"
        f".anno {{ fill: {TEXT_LIGHT}; font: 13px/1 system-ui, sans-serif; }}"
        # Dark mode overrides
        "@media (prefers-color-scheme: dark) {"
        f"  .bee-body {{ fill: {BEE_BODY_DARK}; }}"
        f"  .bee-stripe {{ fill: {BEE_STRIPE_DARK}; }}"
        f"  .trail {{ stroke: {TRAIL_DARK}; }}"
        f"  .ref {{ stroke: {TRAIL_DARK}; }}"
        f"  .ref-solid {{ stroke: {BEE_BODY_DARK}; }}"
        f"  .ref-dot {{ fill: {BEE_BODY_DARK}; }}"
        f"  .lead {{ stroke: {TRAIL_DARK}; }}"
        f"  .anno {{ fill: {TEXT_DARK}; }}"
        "}"
        "</style>"
    )
    parts.append("<defs>")
    parts.append(f'<path id="dance_path" d="{d}"/>')
    parts.append(bee_def())
    parts.append("</defs>")

    # Faint dashed trail of the figure-eight (the path the bee will trace).
    parts.append(f'<path d="{d}" class="trail" stroke-dasharray="3,3" opacity="0.55"/>')

    # Static annotations (vertical reference, angle arc, labels).
    parts.append(annotations())

    # The bee itself, following the dance path with auto-rotation along the tangent.
    parts.append(
        '<use href="#bee">'
        f'<animateMotion dur="{PERIOD:.2f}s" repeatCount="indefinite" rotate="auto" '
        f'keyTimes="{times}" keyPoints="{";".join(f"{i/(len(pts)-1):.4f}" for i in range(len(pts)))}">'
        '<mpath href="#dance_path"/>'
        "</animateMotion>"
        "</use>"
    )

    parts.append("</svg>")
    return "".join(parts)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    default_out = Path(__file__).parent / "bee_dance.svg"
    ap.add_argument("-o", "--output", type=Path, default=default_out)
    args = ap.parse_args()
    args.output.write_text(build_svg())
    print(f"wrote {args.output}")


if __name__ == "__main__":
    main()
