#!/usr/bin/env python3
"""
pacman_cfg.py — Inspect and edit PACMAN.CFG, the binary settings + high
score table file written by the game (see src/includes/game/config.inc).

File layout (schema v3, 80 bytes):
    offset  size  field
    0       2     magic 'P','C'
    2       1     schema version (0x03)
    3       1     lives_start (DIP — 3 or 5)
    4       60    score table — 10 entries × 6 bytes:
                    +0..+2  initials (3 ASCII chars)
                    +3..+5  score (24-bit LE)
    64      1     show_score_table (DIP — 0 = off, 1 = on)
    65      15    reserved (zero — future DIPs)

The 1980 arcade cabinet had no top-10 screen — show_score_table defaults
to 0 (off) on a fresh file. The score table itself is still tracked
internally so the persisted top score (entry 0) can drive the HUD
readout regardless of whether the screen is enabled.

Subcommands:
    dump PATH                      — decode and print every field.
    edit PATH --lives N            — set lives_start (3 or 5).
    edit PATH --reset-table        — restore the factory-default top-10.
    edit PATH --show-score-table on|off
                                   — toggle the optional attract screen.

Editing the score table entry-by-entry is intentionally not supported:
the file is meant for setting machine DIPs and resetting the table back
to factory, not faking scoreboard runs. The in-game GAME_OVER path is
how the table changes during play.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

CFG_SIZE = 80
CFG_MAGIC = b"PC"
CFG_VERSION = 0x03
ALLOWED_LIVES = (3, 5)

TABLE_OFFSET = 4
TABLE_ENTRIES = 10
TABLE_ENTRY_SZ = 6
TABLE_BYTES = TABLE_ENTRIES * TABLE_ENTRY_SZ
SHOW_TABLE_OFFSET = TABLE_OFFSET + TABLE_BYTES                # 64
RESERVED_OFFSET = SHOW_TABLE_OFFSET + 1                       # 65
RESERVED_BYTES = CFG_SIZE - RESERVED_OFFSET                   # 15

# Must mirror cfg_factory_table in src/includes/game/config.inc — keep
# in sync if either side changes.
FACTORY_TABLE: list[tuple[str, int]] = [
    ("AEM", 10000),
    ("SIJ",  9000),
    ("RTB",  8000),
    ("SS7",  7000),
    ("CLD",  6000),
    ("747",   500),
    ("FIL",   400),
    ("SMM",   300),
    ("BIL",   200),
    ("MLK",   100),
]


def encode_table(entries: list[tuple[str, int]]) -> bytes:
    out = bytearray()
    for initials, score in entries:
        if len(initials) != 3 or not initials.isascii():
            raise ValueError(f"initials must be 3 ASCII chars: {initials!r}")
        if not (0 <= score < (1 << 24)):
            raise ValueError(f"score out of 24-bit range: {score}")
        out.extend(initials.encode("ascii"))
        out.extend(score.to_bytes(3, "little"))
    assert len(out) == TABLE_BYTES
    return bytes(out)


def decode_table(buf: bytes) -> list[tuple[str, int]]:
    rows = []
    for i in range(TABLE_ENTRIES):
        off = TABLE_OFFSET + i * TABLE_ENTRY_SZ
        initials = buf[off:off + 3].decode("ascii", errors="replace")
        score = int.from_bytes(buf[off + 3:off + 6], "little")
        rows.append((initials, score))
    return rows


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


def cmd_dump(args: argparse.Namespace) -> int:
    buf = load(args.path)
    show_table = bool(buf[SHOW_TABLE_OFFSET])
    print(f"file:             {args.path}")
    print(f"magic:            {buf[0:2].decode('ascii', errors='replace')!r}")
    print(f"version:          0x{buf[2]:02X}")
    print(f"lives_start:      {buf[3]}        (DIP — editable)")
    print(f"show_score_table: {'on' if show_table else 'off':<3}      (DIP — editable; off is arcade-faithful)")
    print()
    print("high-score table (read-only via --reset-table):")
    print("  rank  initials      score")
    for i, (initials, score) in enumerate(decode_table(buf), start=1):
        print(f"  {i:>4}  {initials:<3}        {score:>6}")
    print()
    reserved_hex = " ".join(f"{b:02X}" for b in buf[RESERVED_OFFSET:CFG_SIZE])
    print(f"reserved:         {reserved_hex}")
    return 0


def cmd_edit(args: argparse.Namespace) -> int:
    buf = load(args.path)
    changed = False

    if args.lives is not None:
        if buf[3] != args.lives:
            print(f"lives_start: {buf[3]} -> {args.lives}")
            buf[3] = args.lives
            changed = True
        else:
            print(f"lives_start: already {args.lives}, no change")

    if args.show_score_table is not None:
        new_val = 1 if args.show_score_table == "on" else 0
        current = buf[SHOW_TABLE_OFFSET]
        if current != new_val:
            label = "on" if new_val else "off"
            prev = "on" if current else "off"
            print(f"show_score_table: {prev} -> {label}")
            buf[SHOW_TABLE_OFFSET] = new_val
            changed = True
        else:
            label = "on" if new_val else "off"
            print(f"show_score_table: already {label}, no change")

    if args.reset_table:
        factory = encode_table(FACTORY_TABLE)
        if bytes(buf[TABLE_OFFSET:TABLE_OFFSET + TABLE_BYTES]) != factory:
            print("score table: resetting to factory defaults")
            buf[TABLE_OFFSET:TABLE_OFFSET + TABLE_BYTES] = factory
            changed = True
        else:
            print("score table: already factory defaults, no change")

    if not changed:
        print("nothing to write")
        return 0

    args.path.write_bytes(bytes(buf))
    print(f"wrote {args.path}")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Inspect and edit PACMAN.CFG (DIP + score-table reset only — individual entries are read-only).",
    )
    subs = parser.add_subparsers(dest="cmd", required=True)

    p_dump = subs.add_parser("dump", help="Print the decoded file contents.")
    p_dump.add_argument("path", type=Path)
    p_dump.set_defaults(func=cmd_dump)

    p_edit = subs.add_parser("edit", help="Write back DIP fields or reset the table.")
    p_edit.add_argument("path", type=Path)
    p_edit.add_argument(
        "--lives",
        type=int,
        choices=ALLOWED_LIVES,
        help="Set starting lives (3 or 5).",
    )
    p_edit.add_argument(
        "--reset-table",
        action="store_true",
        help="Restore the factory-default top-10 high-score table.",
    )
    p_edit.add_argument(
        "--show-score-table",
        choices=("on", "off"),
        help="Toggle the GS_ATTRACT_SCORES screen (off = 1980 arcade-faithful).",
    )
    p_edit.set_defaults(func=cmd_edit)

    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
