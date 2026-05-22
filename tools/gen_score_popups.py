#!/usr/bin/env python3
"""
gen_score_popups.py — Generate Pac-Man score-popup RGBA2 sprites.

The existing ghost-eat popups (200 / 400 / 800 / 1600) live in
src/assets/score/ as 16 × 8 RGBA2 bitmaps coloured cyan (0xFC). The
fruit-eat popups want the same digit shapes in Pinky pink (0xFB).

Pipeline:
  1. Extract digit glyphs 0 / 1 / 2 / 4 / 6 / 8 directly from the
     existing cyan score sprites so we inherit the exact pixel shapes.
  2. Hand-author matching 3 / 5 / 7 glyphs (the user list doesn't need
     9 yet — add if a future value requires it).
  3. Compose each requested value into a 16 × 8 canvas with 1-col
     inter-digit gaps. Pixels go down as Pinky-pink (0xFB).

Output:
  src/assets/score/score_<value>.rgba2

The wider 4-digit values starting with non-"1" digits (2000 / 5000 /
7000) don't fit in 16 × 8 with these digit widths and are intentionally
NOT generated — they need a two-sprite composition (per the arcade
sprite sheet) which is wired separately.
"""

from __future__ import annotations
from pathlib import Path

PINK = 0xFB              # RGBA2222 (A=3, B=3, G=2, R=3) — Pinky body
TRANSPARENT = 0x00

W, H = 16, 8             # output sprite dimensions

# --- Digit glyphs ---------------------------------------------------------
#
# Heights are 7 rows of pixel data + 1 blank row, matching the existing
# score sprites. Variable width per digit, same as the existing font:
# '1' is just a 1-col vertical line; '0' and '6' are 4 wide; '2' / '4'
# / '8' (and our new '3' / '5' / '7') are 5 wide.
DIGITS: dict[str, list[str]] = {
    # Extracted from src/assets/score/score_200.rgba2 cols 0-4.
    "2": [
        ".XXX.",
        "X...X",
        "X...X",
        "...X.",
        "..X..",
        ".X...",
        "XXXXX",
        ".....",
    ],
    # Extracted from src/assets/score/score_200.rgba2 cols 6-9.
    "0": [
        ".XX.",
        "X..X",
        "X..X",
        "X..X",
        "X..X",
        "X..X",
        ".XX.",
        "....",
    ],
    # Extracted from src/assets/score/score_400.rgba2 cols 0-4.
    "4": [
        "...X.",
        "..XX.",
        ".X.X.",
        "X..X.",
        "XXXXX",
        "...X.",
        "...X.",
        ".....",
    ],
    # Extracted from src/assets/score/score_800.rgba2 cols 0-4.
    "8": [
        ".XXX.",
        "X...X",
        "X...X",
        ".XXX.",
        "X...X",
        "X...X",
        ".XXX.",
        ".....",
    ],
    # Extracted from src/assets/score/score_1600.rgba2 col 0.
    "1": [
        "X",
        "X",
        "X",
        "X",
        "X",
        "X",
        "X",
        ".",
    ],
    # Extracted from src/assets/score/score_1600.rgba2 cols 2-5.
    "6": [
        ".XXX",
        "X...",
        "X...",
        "XXXX",
        "X..X",
        "X..X",
        ".XX.",
        "....",
    ],

    # Hand-authored to match the '8' style — same 5-wide envelope and
    # same closed top / bottom / middle horizontals.
    "3": [
        ".XXX.",
        "X...X",
        "....X",
        ".XXX.",
        "....X",
        "X...X",
        ".XXX.",
        ".....",
    ],
    # Hand-authored to match the '6' style flipped — closed bottom curve.
    "5": [
        "XXXXX",
        "X....",
        "X....",
        "XXXX.",
        "....X",
        "X...X",
        ".XXX.",
        ".....",
    ],
    # Hand-authored — flat top + diagonal down-left, matches '2' weight.
    "7": [
        "XXXXX",
        "....X",
        "...X.",
        "..X..",
        "..X..",
        "..X..",
        "..X..",
        ".....",
    ],
}

GAP = 1                  # transparent column between digits


def render(value: str) -> bytes:
    """Render a value string into a 16×8 RGBA2 bitmap."""
    digits = [DIGITS[c] for c in value]
    widths = [len(d[0]) for d in digits]
    total = sum(widths) + GAP * (len(digits) - 1)
    if total > W:
        raise ValueError(
            f"value {value!r} needs {total} cols, won't fit in {W}-wide sprite. "
            f"Consider a two-sprite layout."
        )

    canvas = [[False] * W for _ in range(H)]
    x = 0
    for digit, dw in zip(digits, widths):
        for dy in range(H):
            row = digit[dy]
            for dx in range(dw):
                if row[dx] == "X":
                    canvas[dy][x + dx] = True
        x += dw + GAP

    out = bytearray(W * H)
    for y in range(H):
        for x in range(W):
            out[y * W + x] = PINK if canvas[y][x] else TRANSPARENT
    return bytes(out)


def main() -> None:
    out_dir = Path("src/assets/score")
    out_dir.mkdir(parents=True, exist_ok=True)

    # Values that compose into a single 16×8 sprite. The 4-digit values
    # whose first digit isn't '1' (2000 / 5000 / 7000) are deliberately
    # skipped — they need a two-sprite composition.
    values = ["10", "50", "100", "300", "500", "700", "1000"]

    for v in values:
        data = render(v)
        path = out_dir / f"score_{v}.rgba2"
        path.write_bytes(data)
        print(f"wrote {path}  ({W}x{H}, {len(data)} bytes)")


if __name__ == "__main__":
    main()
