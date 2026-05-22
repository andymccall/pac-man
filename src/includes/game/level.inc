; Level state and per-level reset.
;
; level_init restores the mutable pellet map from its read-only template
; (so a 2nd game / new level doesn't inherit half-eaten state from the
; previous one) and zeroes the per-level pellet counter that drives
; ghost-house release timing and fruit spawns. Called from:
;   * state_player_select on a new game (alongside lives_init / fruit_init)
;   * state_level_clear when advancing to the next level

MAZE_PELLETS_CELLS:  equ 28 * 31          ; 868 bytes
MAZE_TOTAL_PELLETS:  equ 244              ; 240 dots + 4 energisers — used
                                          ; by state_play_tick to detect a
                                          ; cleared maze. Matches the arcade.

; current_level — 1-indexed. Reset to 1 by state_player_select on a new
; game; incremented by state_level_clear when the maze is cleared. Will
; eventually drive per-level scatter/chase timings (#16), Pac speed
; (#22), fruit type (#24), and cruise Elroy (#17).
current_level: db 1

; level_init — copy maze_pellets_template → maze_pellets_map and reset
; pellets_eaten_level. Also picks Pac's per-level sub-pixel speed (#22).
; Destroys: af, bc, de, hl
level_init:
    ld hl, maze_pellets_template
    ld de, maze_pellets_map
    ld bc, MAZE_PELLETS_CELLS
    ldir

    xor a
    ld (pellets_eaten_level), a

    ; Pick Pac's normal / fright speed for current_level — see
    ; pac_move.inc. Resets the accumulator so the new level starts on
    ; a whole-pixel tick.
    jp pac_speed_init_for_level
