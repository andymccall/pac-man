# Pac-Man Arcade Attract-Mode Demo Route

This document records the AI-driven Pac route that plays during the
arcade Pac-Man attract sequence. It's the path used by [#11 — Demo
mode in the attract sequence](https://github.com/andymccall/pac-man/issues/11)
in this port, encoded in [`src/includes/game/pac_script.inc`](../src/includes/game/pac_script.inc).

I could not find this route published anywhere online when I went
looking for it (May 2026). The Pac-Man Dossier mentions only *which*
ghost catches Pac in normal-difficulty attract mode (Bashful / Inky,
in the lower-left), and Don Hodges' site focuses on ghost-AI fixes
rather than the demo path. Armin Reichert's clone uses a runtime
autopilot, not a recorded route. So this transcription may well be
the most complete public record of the demo's tile-by-tile path.

If you arrived here looking for the route — welcome. The full path
is at the bottom under [Canonical waypoint table](#canonical-waypoint-table).

## Background

The arcade Pac-Man attract sequence cycles:

```
CHARACTER REVEAL  →  CHASE ANIMATION  →  PUSH START  →  DEMO  →  (loop)
```

The DEMO segment is roughly 30 seconds of AI-driven Pac playing the
maze, walking a deterministic path, until a ghost catches him in the
lower-left area. The path is the same every cycle — the arcade has
no randomness driving Pac's AI in demo mode.

## How we got here

### Attempt 1 — extract from the ROM via simulation

The first approach was algorithmic. `reference/pac-man.asm` is a
disassembly of the original Pac-Man Z80 arcade ROM. Within the ROM,
Pac's demo movement uses the same target-seeking routine the ghosts
use (`#2966`): for each candidate direction at a tile centre, pick
the one minimising squared straight-line distance to a target tile,
with reverse forbidden and ties resolved by the order
LEFT > DOWN > RIGHT > UP.

In attract mode the demo's target is computed as
`target = 2 × Pac − Pinky`, with Pinky parked inactive in the ghost
house. That gives Pac a deterministic destination each tile, and the
route is the sequence of decisions the algorithm produces.

[`reference/simulate_attract.py`](./simulate_attract.py) implements
this. It parses the wall data straight out of the ROM disassembly
into a 32×32 VRAM grid, then walks the AI step by step until a
state-loop is detected. The output went to
[`reference/attract_mode_route.md`](./attract_mode_route.md) — a
112-step trajectory in *cabinet* and *VRAM* coordinates with each
turn marked.

That worked exactly as designed. The output was even partially
self-consistent. **It didn't translate to our maze, though**, for
two reasons:

1. **Off-grid wraparound.** The simulator's `is_wall` check reads
   directly out of the rotated arcade VRAM and doesn't bound-check
   the cabinet column. Cells outside the playable maze happen to
   contain non-wall tile IDs (they're the score/HUD area in the
   arcade), so the AI cheerfully walks Pac to cabinet columns −1 and
   below, treating the off-screen region as a tunnel. The route
   table records eight of those "off-grid" steps — they're a
   simulator artefact, not arcade behaviour.

2. **Junction position drift.** When we transposed the simulator's
   wall map into our `(col, row)` coordinate system and compared it
   to `maze_wall_map.inc`, 324 of the 840 playable tiles disagreed.
   Some of that is benign (the ROM marks the pen interior as "wall"
   for collision purposes while we mark it as floor for sprite
   compositing), but the rest is real: our maze geometry's
   down-passages from the open lanes sit at slightly different
   columns than the arcade ROM's. So the simulator's first turn
   ("DOWN at cabinet col 7 row 26") lands on a wall in our maze
   instead of an opening.

The conclusion: the ROM disassembly gives us the arcade's exact
demo path *for the arcade's exact maze*, but I'd need to either
align our maze to the arcade byte-for-byte (a substantial migration
that would ripple through `#14`, sprite alignment, the chase
animation, etc.) or re-derive the path for our maze. I chose the
latter.

### Attempt 2 — manual transcription against our maze

I rendered our wall + pellet map as a text grid, marked Pac's spawn
position, and walked the arcade demo by eye, replacing each tile he
visited with a direction letter (`L` / `R` / `U` / `D`) for the
direction Pac was heading through that tile. Because Pac traverses
several intersections more than once during the ~30 s demo, the
route was split into four annotated maze copies — one per "segment"
of the path.

Marker conventions:

| Glyph | Meaning |
|-------|---------|
| `#`   | Wall (don't touch) |
| `.`   | Floor with dot pellet |
| `o`   | Floor with energiser (power pellet) |
| (space) | Floor without pellet (tunnel row, pen, junctions) |
| `S`   | Start of this segment |
| `E`   | End of this segment |
| `X`   | Pac dies here (last map only) |
| `L` `R` `U` `D` | Pac was heading in this direction when he passed through this tile |

## The four maps

In each map, Pac walks from `S` to `E` (or to `X` on the final map).
The maps share tiles where the path retraces itself; each map
captures one traversal of those tiles.

### Map 1 — spawn to upper-left, energiser at (1, 23)

Pac heads LEFT out of the spawn at (14, 23). He walks the bottom
lane, down to the bottom row, across, up the right side, traverses
the upper-right inner loop counter-clockwise, drops to the tunnel
row, ducks left through the lower middle, and climbs col 6 to the
top-left corner where the segment ends at (5, 5).

```
                111111111122222222
      0123456789012345678901234567
     +----------------------------+
r00  |############################|
r01  |#LLLLLU......##............#|
r02  |#D####U#####.##.#####.####.#|
r03  |#D#  #U#   #.##.#   #.#  #o#|
r04  |#D####U#####.##.#####.####.#|
r05  |#DRRREU....................#|
r06  |#.####U##.########.##.####.#|
r07  |#.####U##.########.##.####.#|
r08  |#.....U##....##....##......#|
r09  |######U##### ## #####.######|
r10  |     #U##### ## #####.#     |
r11  |     #U##LLLLLLLLLU##.#     |
r12  |     #U##D########U##.#     |
r13  |######U##D#      #U##.######|
r14  |      LLLD#      #U  .      |
r15  |######.## #      #U##.######|
r16  |     #.## ########U##.#     |
r17  |     #.##         U##.#     |
r18  |     #.## ########U##.#     |
r19  |######.## ########U##.######|
r20  |#............##...LLLLLLLLU#|
r21  |#.####.#####.##.#####.####U#|
r22  |#.####.#####.##.#####.####U#|
r23  |#o..##...LLLLLS.......##URR#|
r24  |###.##.##D########.##.##U###|
r25  |###.##.##D########.##.##U###|
r26  |#......##DRRR##....##...LLU#|
r27  |#.##########D##.##########U#|
r28  |#.##########D##.##########U#|
r29  |#...........DRRRRRRRRRRRRRR#|
r30  |############################|
     +----------------------------+
```

### Map 2 — upper-left loop

Pac leaves (4, 5) heading RIGHT, climbs to row 1, runs left along
the top, drops back down col 1, and finishes at the E marker at
(3, 5).

```
                111111111122222222
      0123456789012345678901234567
     +----------------------------+
r00  |############################|
r01  |#LLLLLLLLLLLU##............#|
r02  |#D####.#####U##.#####.####.#|
r03  |#D#  #.#   #U##.#   #.#  #o#|
r04  |#D####.#####U##.#####.####.#|
r05  |#DRESRRRRRRRR..............#|
r06  |#.####.##.########.##.####.#|
r07  |#.####.##.########.##.####.#|
r08  |#......##....##....##......#|
r09  |######.##### ## #####.######|
r10  |     #.##### ## #####.#     |
r11  |     #.##..........##.#     |
r12  |     #.##.########.##.#     |
r13  |######.##.#      #.##.######|
r14  |      ....#      #.  .      |
r15  |######.## #      #.##.######|
r16  |     #.## ########.##.#     |
r17  |     #.##         .##.#     |
r18  |     #.## ########.##.#     |
r19  |######.## ########.##.######|
r20  |#............##............#|
r21  |#.####.#####.##.#####.####.#|
r22  |#.####.#####.##.#####.####.#|
r23  |#o..##................##...#|
r24  |###.##.##.########.##.##.###|
r25  |###.##.##.########.##.##.###|
r26  |#......##....##....##......#|
r27  |#.##########.##.##########.#|
r28  |#.##########.##.##########.#|
r29  |#..........................#|
r30  |############################|
     +----------------------------+
```

### Map 3 — across, tunnel wrap, bottom loop

The longest segment. From (4, 5) Pac goes right and down through
the central region, descends to row 14, walks left through the
tunnel (the only row where col 0 and col 27 are walkable), wraps to
the right side, comes down col 21, loops through the bottom-left
energiser corner, climbs back to row 20, and exits at the E marker
at (20, 20).

```
                111111111122222222
      0123456789012345678901234567
     +----------------------------+
r00  |############################|
r01  |#............##............#|
r02  |#.####.#####.##.#####.####.#|
r03  |#.#  #.#   #.##.#   #.#  #o#|
r04  |#.####.#####.##.#####.####.#|
r05  |#...SRRRRR.................#|
r06  |#.####.##D########.##.####.#|
r07  |#.####.##D########.##.####.#|
r08  |#......##DRRR##....##......#|
r09  |######.#####D## #####.######|
r10  |     #.#####D## #####.#     |
r11  |     #.##LLLD......##.#     |
r12  |     #.##D########.##.#     |
r13  |######.##D#      #.##.######|
r14  |LLLLLLLLLD#      #.  LLLLLLL|
r15  |######.## #      #.##D######|
r16  |     #.## ########.##D#     |
r17  |     #.##         .##D#     |
r18  |     #.## ########.##D#     |
r19  |######.## ########.##D######|
r20  |#URRRRRRRRRRR##URRRRED.....#|
r21  |#U####.#####D##U#####D####.#|
r22  |#U####.#####D##U#####D####.#|
r23  |#LLU##......DRRR.....D##...#|
r24  |###U##.##.########.##D##.###|
r25  |###U##.##.########.##D##.###|
r26  |#URR...##....##....##DRRRRR#|
r27  |#U##########.##.##########D#|
r28  |#U##########.##.##########D#|
r29  |#LLLLLLLLLLLLLLLLLLLLLLLLLD#|
r30  |############################|
     +----------------------------+
```

### Map 4 — final approach, caught at (3, 20)

Pac enters the segment at (20, 20) heading RIGHT, runs the bottom
loop once more, and is caught by Bashful (Inky) in the lower-left
at (3, 20).

```
                111111111122222222
      0123456789012345678901234567
     +----------------------------+
r00  |############################|
r01  |#............##............#|
r02  |#.####.#####.##.#####.####.#|
r03  |#.#  #.#   #.##.#   #.#  #o#|
r04  |#.####.#####.##.#####.####.#|
r05  |#..........................#|
r06  |#.####.##.########.##.####.#|
r07  |#.####.##.########.##.####.#|
r08  |#......##....##....##......#|
r09  |######.##### ## #####.######|
r10  |     #.##### ## #####.#     |
r11  |     #.##..........##.#     |
r12  |     #.##.########.##.#     |
r13  |######.##.#      #.##.######|
r14  |      ....#      #.  .      |
r15  |######.## #      #.##.######|
r16  |     #.## ########.##.#     |
r17  |     #.##         .##.#     |
r18  |     #.## ########.##.#     |
r19  |######.## ########.##.######|
r20  |#URX.........##.....SRRRRRR#|
r21  |#U####.#####.##.#####.####D#|
r22  |#U####.#####.##.#####.####D#|
r23  |#LLU##................##LLD#|
r24  |###U##.##.########.##.##D###|
r25  |###U##.##.########.##.##D###|
r26  |#URR...##....##....##...DRR#|
r27  |#U##########.##.##########D#|
r28  |#U##########.##.##########D#|
r29  |#LLLLLLLLLLLLLLLLLLLLLLLLLD#|
r30  |############################|
     +----------------------------+
```

## Canonical waypoint table

Each row is a *turn waypoint*: when Pac's tile coordinates match,
`pac_desired_dir` is set to the listed direction and the cursor
advances. Tiles between waypoints are straight-line traversals and
need no entries.

The kickoff at (14, 23) is the spawn tile; `pac_reset` leaves
`pac_dir = NONE`, so the script seeds the initial LEFT explicitly.

| #  | Col | Row | Dir   | Source / notes |
|----|----:|----:|-------|----------------|
| 1  | 14  | 23  | LEFT  | spawn kickoff |
| 2  |  9  | 23  | DOWN  | map 1 |
| 3  |  9  | 26  | RIGHT | map 1 |
| 4  | 12  | 26  | DOWN  | map 1 |
| 5  | 12  | 29  | RIGHT | map 1 |
| 6  | 26  | 29  | UP    | map 1 |
| 7  | 26  | 26  | LEFT  | map 1 |
| 8  | 24  | 26  | UP    | map 1 |
| 9  | 24  | 23  | RIGHT | map 1 |
| 10 | 26  | 23  | UP    | map 1 |
| 11 | 26  | 20  | LEFT  | map 1 |
| 12 | 18  | 20  | UP    | map 1 |
| 13 | 18  | 11  | LEFT  | map 1 |
| 14 |  9  | 11  | DOWN  | map 1 |
| 15 |  9  | 14  | LEFT  | map 1 |
| 16 |  6  | 14  | UP    | map 1 |
| 17 |  6  |  1  | LEFT  | map 1 |
| 18 |  1  |  1  | DOWN  | map 1 |
| 19 |  1  |  5  | RIGHT | map 1 |
| 20 | 12  |  5  | UP    | map 2 |
| 21 | 12  |  1  | LEFT  | map 2 |
| 22 |  1  |  1  | DOWN  | map 2 |
| 23 |  1  |  5  | RIGHT | map 2 |
| 24 |  9  |  5  | DOWN  | map 3 |
| 25 |  9  |  8  | RIGHT | map 3 |
| 26 | 12  |  8  | DOWN  | map 3 |
| 27 | 12  | 11  | LEFT  | map 3 |
| 28 |  9  | 11  | DOWN  | map 3 |
| 29 |  9  | 14  | LEFT  | map 3 (Pac enters the tunnel here) |
| 30 | 21  | 14  | DOWN  | map 3 (turn after the tunnel wrap) |
| 31 | 21  | 26  | RIGHT | map 3 |
| 32 | 26  | 26  | DOWN  | map 3 |
| 33 | 26  | 29  | LEFT  | map 3 |
| 34 |  1  | 29  | UP    | map 3 |
| 35 |  1  | 26  | RIGHT | map 3 |
| 36 |  3  | 26  | UP    | map 3 |
| 37 |  3  | 23  | LEFT  | map 3 |
| 38 |  1  | 23  | UP    | map 3 |
| 39 |  1  | 20  | RIGHT | map 3 |
| 40 | 12  | 20  | DOWN  | map 3 |
| 41 | 12  | 23  | RIGHT | map 3 |
| 42 | 15  | 23  | UP    | map 3 |
| 43 | 15  | 20  | RIGHT | map 3 |
| 44 | 26  | 20  | DOWN  | map 4 |
| 45 | 26  | 23  | LEFT  | map 4 |
| 46 | 24  | 23  | DOWN  | map 4 |
| 47 | 24  | 26  | RIGHT | map 4 |
| 48 | 26  | 26  | DOWN  | map 4 |
| 49 | 26  | 29  | LEFT  | map 4 |
| 50 |  1  | 29  | UP    | map 4 |
| 51 |  1  | 26  | RIGHT | map 4 |
| 52 |  3  | 26  | UP    | map 4 |
| 53 |  3  | 23  | LEFT  | map 4 |
| 54 |  1  | 23  | UP    | map 4 |
| 55 |  1  | 20  | RIGHT | map 4 (Pac is caught at (3, 20) shortly after this) |

## Implementation in this port

[`src/includes/game/pac_script.inc`](../src/includes/game/pac_script.inc)
encodes the table as a packed `db col, row, dir` stream and provides
`pac_script_sample`. The demo state ([`states/attract_demo.inc`](../src/includes/game/states/attract_demo.inc))
calls the script first; if the cursor is still active, the heuristic
AI in [`pac_ai.inc`](../src/includes/game/pac_ai.inc) is skipped so
it can't second-guess the recorded path. When the cursor exhausts
(or if Pac diverges and never reaches the next waypoint), the
heuristic takes over as a safety net.

In the current build of this port, our ghost AI catches Pac in the
bottom row before the script completes — typically around (col 11,
row 29). When the ghost AI is tuned closer to arcade behaviour (see
the open issues for Pinky targeting, Inky targeting, and per-level
scatter/chase timings) Pac should survive longer and trace more of
the recorded route before being caught.

## Coordinate notes

Our `(col, row)` system has:

- Column 0 on the left, column 27 on the right
- Row 0 at the top of the playable maze (the row of the HUD-adjacent
  outer wall), row 30 at the bottom
- Pac's spawn at `(14, 23)` (the same logical position as cabinet
  (14, 26) in the Pittman dossier / arcade-ROM convention — the
  3-row offset is the HUD band above row 0 of the playable maze)

The tunnel sits on row 14: it is the only row where cells `(0, 14)`
and `(27, 14)` are walkable. Pac stepping LEFT off `(0, 14)`
reappears at `(27, 14)` still heading LEFT (the tunnel preserves
direction).

## Provenance / further reading

- [`reference/simulate_attract.py`](./simulate_attract.py) — the
  ROM-based simulator described under "Attempt 1". Implements
  arcade routine `#2966` and walks the demo from VRAM data.
- [`reference/attract_mode_route.md`](./attract_mode_route.md) —
  output of the simulator; trajectory in cabinet/VRAM coords,
  including the off-grid wraparound steps.
- [`reference/attract_mode_route_template.md`](./attract_mode_route_template.md)
  — the four manually-annotated maze maps as the author drew them.
- [`reference/pac-man.asm`](./pac-man.asm) — Z80 ROM disassembly,
  the data source for the simulator.
- *The Pac-Man Dossier* by Jamey Pittman: https://pacman.holenet.info/
- Don Hodges' Pac-Man bug analyses: http://donhodges.com/
