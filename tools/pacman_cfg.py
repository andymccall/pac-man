#!/usr/bin/env python3
"""
pacman_cfg.py — Inspect and edit PACMAN.CFG, the binary settings + high
score file written by the game (see src/includes/game/config.inc).

File layout (16 bytes):
    offset  size  field
    0       2     magic 'P','C'
    2       1     schema version (0x01)
    3       1     lives_start (DIP — 3 or 5)
    4       3     high_score (24-bit LE)
    7       9     reserved (zero — future DIPs)

Subcommands:
    dump PATH                  — decode and print every field.
    edit PATH --lives N        — set lives_start (3 or 5) and rewrite.

Editing the high score is intentionally not supported: the file's
on-disk format is meant for setting machine DIPs, not faking scoreboard
runs. Use the in-game GAME_OVER path to update the high score.

Examples:
    python tools/pacman_cfg.py dump PACMAN.CFG
    python tools/pacman_cfg.py edit PACMAN.CFG --lives 5
"""
from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

CFG_SIZE = 16
CFG_MAGIC = b"PC"
CFG_VERSION = 0x01
ALLOWED_LIVES = (3, 5)


def load(path: Path) -> bytearray:
    data = path.read_bytes()
    if len(data) != CFG_SIZE:
        raise SystemExit(
            f"{path}: expected {CFG_SIZE} bytes, got {len(data)}"
        )
    if data[0:2] != CFG_MAGIC:
        raise SystemExit(
            f"{path}: bad magic {data[0:2]!r} (expected {CFG_MAGIC!r})"
        )
    if data[2] != CFG_VERSION:
        raise SystemExit(
            f"{path}: unsupported schema version 0x{data[2]:02X} "
            f"(this tool knows 0x{CFG_VERSION:02X})"
        )
    return bytearray(data)


def decode(buf: bytes) -> dict:
    high = buf[4] | (buf[5] << 8) | (buf[6] << 16)
    return {
        "magic": buf[0:2].decode("ascii", errors="replace"),
        "version": buf[2],
        "lives_start": buf[3],
        "high_score": high,
        "reserved": bytes(buf[7:16]),
    }


def cmd_dump(args: argparse.Namespace) -> int:
    buf = load(args.path)
    fields = decode(buf)
    print(f"file:        {args.path}")
    print(f"magic:       {fields['magic']!r}")
    print(f"version:     0x{fields['version']:02X}")
    print(f"lives_start: {fields['lives_start']}  (DIP — editable)")
    print(f"high_score:  {fields['high_score']:>7}  (read-only)")
    reserved_hex = " ".join(f"{b:02X}" for b in fields["reserved"])
    print(f"reserved:    {reserved_hex}  (9 bytes, future DIPs)")
    return 0


def cmd_edit(args: argparse.Namespace) -> int:
    buf = load(args.path)
    changed = False

    if args.lives is not None:
        if args.lives not in ALLOWED_LIVES:
            raise SystemExit(
                f"--lives must be one of {ALLOWED_LIVES}, got {args.lives}"
            )
        if buf[3] != args.lives:
            print(f"lives_start: {buf[3]} -> {args.lives}")
            buf[3] = args.lives
            changed = True
        else:
            print(f"lives_start: already {args.lives}, no change")

    if not changed:
        print("nothing to write")
        return 0

    args.path.write_bytes(bytes(buf))
    print(f"wrote {args.path}")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Inspect and edit PACMAN.CFG (DIP settings only — high score is read-only).",
    )
    subs = parser.add_subparsers(dest="cmd", required=True)

    p_dump = subs.add_parser("dump", help="Print the decoded file contents.")
    p_dump.add_argument("path", type=Path)
    p_dump.set_defaults(func=cmd_dump)

    p_edit = subs.add_parser("edit", help="Write back DIP fields.")
    p_edit.add_argument("path", type=Path)
    p_edit.add_argument(
        "--lives",
        type=int,
        choices=ALLOWED_LIVES,
        help="Set starting lives (3 or 5).",
    )
    p_edit.set_defaults(func=cmd_edit)

    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
