; Per-level reset. Restores the mutable pellet map from its read-only
; template (so a 2nd game / new level doesn't inherit half-eaten state
; from the previous one) and zeroes the per-level pellet counter that
; drives ghost-house release timing and fruit spawns.
;
; Called from state_player_select_tick when a new game starts. The
; existing lives_init / fruit_init / score-reset stay where they are —
; level_init covers only the maze-cell state that pellet_eat mutates.
;
; Eventually a level-progression flow will call this too when advancing
; to the next level.

MAZE_PELLETS_CELLS: equ 28 * 31         ; 868 bytes

; level_init — copy maze_pellets_template → maze_pellets_map and reset
; pellets_eaten_level.
; Destroys: af, bc, de, hl
level_init:
    ld hl, maze_pellets_template
    ld de, maze_pellets_map
    ld bc, MAZE_PELLETS_CELLS
    ldir

    xor a
    ld (pellets_eaten_level), a
    ret
