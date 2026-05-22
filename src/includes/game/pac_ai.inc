; Pac AI for the demo (#11, phase 2).
;
; pac_ai_sample is a drop-in replacement for pac_input_sample. Both write
; pac_desired_dir; the rest of the movement pipeline (pac_try_turn,
; pac_step) doesn't care where the desired direction came from.
;
; Phase-2 strategy — naive pellet-seeking, no ghost awareness:
;   * Evaluate each of the four cardinal directions (skip the reverse of
;     pac_dir so we don't oscillate).
;   * For each direction, scan forward up to PAC_AI_LOOKAHEAD tiles,
;     stopping at the first wall.
;   * Score = sum of pellet values along the ray. Dots are worth 1,
;     energisers 2 (matches their values in maze_pellets_map, which keeps
;     the loop body trivial).
;   * Pick the direction with the highest score. Ties resolve by
;     evaluation order (UP, DOWN, LEFT, RIGHT) — refined in phase 3.
;   * If no forward direction scores anything AND no forward direction
;     is even walkable (dead end), force reverse so Pac doesn't stall.
;
; Re-evaluates every frame. Cheap (~20 tile lookups) and avoids any
; alignment-gating complexity. pac_try_turn already ensures the picked
; direction is walkable from the current tile before committing pac_dir.

PAC_AI_LOOKAHEAD: equ 16               ; tiles scanned per direction — needs
                                       ; to be deep enough that long empty
                                       ; corridors still register a pellet at
                                       ; the far end (otherwise the AI sees
                                       ; "0 vs 0" at the dead-end junctions
                                       ; in the bottom row and oscillates)

pac_ai_cur_row:    db 0
pac_ai_cur_col:    db 0
pac_ai_rev_dir:    db 0
pac_ai_try_arg:    db 0
pac_ai_scan_row:   db 0
pac_ai_scan_col:   db 0
pac_ai_scan_count: db 0
pac_ai_score:      db 0
pac_ai_best_score: db 0
pac_ai_best_dir:   db 0
pac_ai_any_open:   db 0                ; any non-reverse walkable found?

; pac_ai_sample — write pac_desired_dir based on the maze + pellet map.
; Destroys: af, bc, de, hl
pac_ai_sample:
    ; Kick-off: pac_dir = NONE at the start of demo (pac_reset sets it).
    ; Arcade demo Pac heads LEFT out of the spawn tile, so do the same.
    ld a, (pac_dir)
    or a
    jr nz, @have_dir
    ld a, DIR_LEFT
    ld (pac_desired_dir), a
    ret
@have_dir:

    ; Pac's current tile.
    ld bc, (sprite_x)
    ld de, (sprite_y)
    call sprite_to_tile
    ret c                              ; off-grid (tunnel mid-warp) — keep going
    ld a, b
    ld (pac_ai_cur_row), a
    ld a, c
    ld (pac_ai_cur_col), a

    ld a, (pac_dir)
    call opposite_dir
    ld (pac_ai_rev_dir), a

    xor a
    ld (pac_ai_best_score), a
    ld (pac_ai_best_dir), a
    ld (pac_ai_any_open), a

    ; Evaluate pac_dir FIRST so it wins all ties — without this, tied
    ; scores (typical when no pellets are in lookahead range) defer to
    ; eval order (UP first), which makes Pac alternate UP/DOWN in any
    ; vertical corridor with no nearby pellets.
    ld a, (pac_dir)
    call @try_dir
    ld a, DIR_UP
    call @try_dir
    ld a, DIR_DOWN
    call @try_dir
    ld a, DIR_LEFT
    call @try_dir
    ld a, DIR_RIGHT
    call @try_dir

    ld a, (pac_ai_best_dir)
    or a
    jr nz, @set_dir

    ; No scoring forward direction. If any non-reverse direction is at
    ; least walkable, pick the first one (evaluation order). Otherwise
    ; force reverse so Pac doesn't stall against a wall.
    ld a, (pac_ai_any_open)
    or a
    jr z, @reverse
    ; (any_open holds the first walkable non-reverse dir we found)
    ld (pac_desired_dir), a
    ret
@reverse:
    ld a, (pac_ai_rev_dir)
@set_dir:
    ld (pac_desired_dir), a
    ret

; @try_dir — evaluate one candidate direction. A = direction.
;   Reverse of pac_dir is skipped.
;   Walkability of the first step is required; otherwise the direction
;   contributes nothing.
;   On a walkable first step, ray-scans up to PAC_AI_LOOKAHEAD tiles in
;   that direction (stopping at the first wall) and sums pellet values.
;   If the total score is strictly better than pac_ai_best_score, the
;   direction becomes the new best.
;   Tracks pac_ai_any_open so the caller can fall back to the first
;   walkable non-reverse direction if no direction scored anything.
@try_dir:
    ld (pac_ai_try_arg), a
    ld b, a
    ld a, (pac_ai_rev_dir)
    cp b
    ret z                              ; skip reverse

    ; Seed scan position at Pac's current tile; @advance_scan will step
    ; into the first neighbour.
    ld a, (pac_ai_cur_row)
    ld (pac_ai_scan_row), a
    ld a, (pac_ai_cur_col)
    ld (pac_ai_scan_col), a

    xor a
    ld (pac_ai_score), a

    ld a, PAC_AI_LOOKAHEAD
    ld (pac_ai_scan_count), a

    ; First step — must be walkable for this direction to count.
    call @advance_scan
    ret c                              ; off-grid in this direction
    call @scan_tile_walkable
    ret nz                             ; first neighbour is a wall — skip dir

    ; Record this as a fallback open direction (first writer wins, since
    ; we early-out as soon as we set it).
    ld a, (pac_ai_any_open)
    or a
    jr nz, @no_record_open
    ld a, (pac_ai_try_arg)
    ld (pac_ai_any_open), a
@no_record_open:

    call @scan_add_pellet
    ld a, (pac_ai_scan_count)
    dec a
    ld (pac_ai_scan_count), a

@scan_loop:
    ld a, (pac_ai_scan_count)
    or a
    jr z, @scan_done

    call @advance_scan
    jr c, @scan_done
    call @scan_tile_walkable
    jr nz, @scan_done                  ; wall blocks the ray

    call @scan_add_pellet
    ld a, (pac_ai_scan_count)
    dec a
    ld (pac_ai_scan_count), a
    jr @scan_loop

@scan_done:
    ld a, (pac_ai_score)
    ld b, a
    ld a, (pac_ai_best_score)
    cp b
    ret nc                             ; existing best >= candidate
    ld a, b
    ld (pac_ai_best_score), a
    ld a, (pac_ai_try_arg)
    ld (pac_ai_best_dir), a
    ret

; @advance_scan — step (pac_ai_scan_row, pac_ai_scan_col) one tile in
; pac_ai_try_arg's direction. Returns carry=1 if stepping off the grid.
; Destroys: af
@advance_scan:
    ld a, (pac_ai_try_arg)
    cp DIR_UP
    jr nz, @as1
    ld a, (pac_ai_scan_row)
    or a
    jr z, @as_off
    dec a
    ld (pac_ai_scan_row), a
    or a
    ret
@as1:
    cp DIR_DOWN
    jr nz, @as2
    ld a, (pac_ai_scan_row)
    cp maze_play_rows - 1
    jr nc, @as_off
    inc a
    ld (pac_ai_scan_row), a
    or a
    ret
@as2:
    cp DIR_LEFT
    jr nz, @as3
    ld a, (pac_ai_scan_col)
    or a
    jr z, @as_off
    dec a
    ld (pac_ai_scan_col), a
    or a
    ret
@as3:
    ; DIR_RIGHT
    ld a, (pac_ai_scan_col)
    cp maze_play_cols - 1
    jr nc, @as_off
    inc a
    ld (pac_ai_scan_col), a
    or a
    ret
@as_off:
    scf
    ret

; @scan_tile_walkable — Z=1 iff (scan_row, scan_col) is a walkable tile.
; Matches the convention used by tile_is_wall in collision.inc.
; Destroys: af, bc, de, hl
@scan_tile_walkable:
    ld a, (pac_ai_scan_row)
    ld b, a
    ld a, (pac_ai_scan_col)
    ld c, a
    call tile_row_col_to_offset
    ld de, maze_wall_map
    add hl, de
    ld a, (hl)
    cp maze_wall_floor
    ret                                ; Z=1 if floor (walkable), NZ if wall

; @scan_add_pellet — read maze_pellets_map at (scan_row, scan_col) and
; add the value (0 / 1 / 2) into pac_ai_score.
; Destroys: af, bc, de, hl
@scan_add_pellet:
    ld a, (pac_ai_scan_row)
    ld b, a
    ld a, (pac_ai_scan_col)
    ld c, a
    call tile_row_col_to_offset
    ld de, maze_pellets_map
    add hl, de
    ld a, (hl)
    or a
    ret z
    ld b, a
    ld a, (pac_ai_score)
    add a, b
    ld (pac_ai_score), a
    ret
