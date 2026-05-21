; GS_LEVEL_CLEAR — Pac just ate the 244th pellet.
;
; Flow (#14 + #40):
;   1. Hide all four ghosts so they don't sit awkwardly on a cleared maze.
;   2. Flash the walls 4 times by alternately redrawing the maze with
;      a white-tinted tile set and the normal blue tile set
;      (#40 — phase machine below). 15 frames per phase, 8 phases
;      → 2 s total, matches the arcade.
;   3. Hold for ~0.5 s after the last flash, then advance current_level,
;      restore the pellet map, transition to GS_READY.

LEVEL_CLEAR_PHASE_FRAMES: equ 15            ; 0.25 s per phase
LEVEL_CLEAR_NUM_PHASES:   equ 8             ; 4 flashes × (white + blue)
LEVEL_CLEAR_HOLD_FRAMES:  equ 30            ; pause after last flash, before READY

; Phase 0 starts as "still showing the blue maze that play left behind"
; — we don't redraw on entry. Each subsequent phase boundary swaps the
; maze base buffer and redraws.
state_level_clear_phase:        db 0
state_level_clear_phase_timer:  db LEVEL_CLEAR_PHASE_FRAMES
state_level_clear_hold_timer:   db LEVEL_CLEAR_HOLD_FRAMES

state_level_clear_enter:
    ; Hide ghosts immediately (matches state_dying_enter).
    ld a, SPRITE_BLINKY_ID
    call vdu_sprite_select
    call vdu_sprite_hide
    ld a, SPRITE_PINKY_ID
    call vdu_sprite_select
    call vdu_sprite_hide
    ld a, SPRITE_INKY_ID
    call vdu_sprite_select
    call vdu_sprite_hide
    ld a, SPRITE_CLYDE_ID
    call vdu_sprite_select
    call vdu_sprite_hide

    xor a
    ld (state_level_clear_phase), a
    ld a, LEVEL_CLEAR_PHASE_FRAMES
    ld (state_level_clear_phase_timer), a
    ld a, LEVEL_CLEAR_HOLD_FRAMES
    ld (state_level_clear_hold_timer), a
    ret

state_level_clear_tick:
    ; Keep the HUD ticking so the hold doesn't look frozen.
    call display_1up
    call display_high_score
    call display_2up
    call credit_display
    macro_text_set_color VDU_COL_WHITE
    call display_player_1_score

    ; All flash phases done? — fall through to the post-hold timer.
    ld a, (state_level_clear_phase)
    cp LEVEL_CLEAR_NUM_PHASES
    jp nc, @post_hold

    ; Still flashing. Tick the per-phase timer.
    ld hl, state_level_clear_phase_timer
    dec (hl)
    ret nz

    ; Phase boundary — reset timer, advance phase, redraw with swapped base.
    ld a, LEVEL_CLEAR_PHASE_FRAMES
    ld (hl), a

    ld a, (state_level_clear_phase)
    inc a
    ld (state_level_clear_phase), a
    cp LEVEL_CLEAR_NUM_PHASES
    ret nc                                  ; final phase done — next tick handles post-hold

    ; Odd phases (1, 3, 5, 7) show white. Even phases (0, 2, 4, 6) show
    ; blue. We just advanced, so A holds the new phase.
    bit 0, a
    jr z, @redraw_blue
@redraw_white:
    ld hl, SPRITE_MAZE_TILE_00_WHITE
    ld (maze_wall_base_bufferId), hl
    jr @do_redraw
@redraw_blue:
    ld hl, SPRITE_MAZE_TILE_00
    ld (maze_wall_base_bufferId), hl
@do_redraw:
    ; Same origin / draw_maze pattern as state_ready_enter's @draw_static,
    ; but only the walls — pellets and banner are intentionally skipped.
    ld bc, origin_left + 8
    ld de, origin_top + 8
    call vdu_set_gfx_origin
    ld ix, maze_wall
    ld hl, SPRITE_MAZE_TILE_00              ; ignored by draw_maze; struct field wins
    call draw_maze
    ld bc, origin_left
    ld de, origin_top
    call vdu_set_gfx_origin
    ret

@post_hold:
    ld hl, state_level_clear_hold_timer
    dec (hl)
    ret nz

    ; Done — advance level. (Make sure the maze base is back to blue
    ; before READY redraws — defensive in case the last flash phase
    ; was white.)
    ld hl, SPRITE_MAZE_TILE_00
    ld (maze_wall_base_bufferId), hl

    ld a, (current_level)
    inc a
    ld (current_level), a
    call fruit_init                         ; resets shown_count + hides sprite
    call level_init                         ; restores pellet map + counter

    ld a, GS_READY
    jp game_state_set
