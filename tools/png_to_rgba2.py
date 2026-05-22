#!/usr/bin/env python3
"""
png_to_rgba2.py — Convert a PNG into a raw Agon VDP RGBA2222 binary.

Pac-Man's .rgba2 assets are 1 byte per pixel packed as
    bits 7-6 = A (2 bits)
    bits 5-4 = B
    bits 3-2 = G
    bits 1-0 = R
Output is row-major from top-left, no header. Image dimensions live
in the bitmap-buffer registration (vdu_game_data.inc), not in the file.

Usage:
    python tools/png_to_rgba2.py INPUT.png OUTPUT.rgba2
    python tools/png_to_rgba2.py INPUT.png        # writes alongside as INPUT.rgba2

Based on from-the-dead/scripts/convert_sprite.py's encode_frame_agon().
That script does palette quantization + Agon RGBA2222 packing; for
pac-man the PNGs are already authored in the target palette so we
skip the quantization step and pack directly from the per-pixel RGBA.

Run with --force to overwrite an existing .rgba2.
"""
from __future__ import annotations
import argparse
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    sys.exit("Pillow required: pip install --user pillow")


def rgba2222(r: int, g: int, b: int, a: int) -> int:
    """Pack a 32-bit RGBA pixel into Agon's 1-byte RGBA2222 format."""
    return (((a >> 6) & 3) << 6) | (((b >> 6) & 3) << 4) | \
           (((g >> 6) & 3) << 2) | ((r >> 6) & 3)


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("input", type=Path, help="Source PNG")
    ap.add_argument("output", type=Path, nargs="?",
                    help="Destination .rgba2 (defaults to <input>.rgba2)")
    ap.add_argument("--force", action="store_true",
                    help="Overwrite output if it exists")
    args = ap.parse_args(argv)

    out_path: Path = args.output or args.input.with_suffix(".rgba2")
    if out_path.exists() and not args.force:
        sys.exit(f"{out_path} exists — pass --force to overwrite")

    img = Image.open(args.input).convert("RGBA")
    w, h = img.size

    buf = bytearray(w * h)
    for i, (r, g, b, a) in enumerate(img.getdata()):
        buf[i] = rgba2222(r, g, b, a)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(bytes(buf))
    print(f"{args.input}  ->  {out_path}  ({w}x{h}, {len(buf)} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
