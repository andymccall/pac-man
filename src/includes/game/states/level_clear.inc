; GS_LEVEL_CLEAR — Pac just ate the 244th pellet.
;
; Flow (#14 + #40):
;   1. Hide all four ghosts so they don't sit on a cleared maze.
;   2. Flash the walls 4 times by alternately redrawing the maze with
;      a white-tinted tile set (#40 phase machine below). 15 frames per
;      phase × 8 phases (4 white + 4 blue) = 2 s total.
;   3. Hold ~0.5 s after the last flash, then advance current_level,
;      restore the pellet map, transition to GS_READY.

LEVEL_CLEAR_PHASE_FRAMES: equ 8             ; ~0.13 s per phase
LEVEL_CLEAR_NUM_PHASES:   equ 8             ; 4 flashes × (white + blue)
LEVEL_CLEAR_HOLD_FRAMES:  equ 30            ; post-flash pause before READY
                                            ; Total flash ≈ 1.1 s; matches
                                            ; the arcade better than 15 fpp.

state_level_clear_phase:        db 0
state_level_clear_phase_timer:  db LEVEL_CLEAR_PHASE_FRAMES
state_level_clear_hold_timer:   db LEVEL_CLEAR_HOLD_FRAMES

state_level_clear_enter:
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
    call display_1up
    call display_high_score
    call display_2up
    call credit_display
    macro_text_set_color VDU_COL_WHITE
    call display_player_1_score

    ld a, (state_level_clear_phase)
    cp LEVEL_CLEAR_NUM_PHASES
    jp nc, @post_hold

    ld hl, state_level_clear_phase_timer
    dec (hl)
    ret nz

    ld a, LEVEL_CLEAR_PHASE_FRAMES
    ld (hl), a

    ld a, (state_level_clear_phase)
    inc a
    ld (state_level_clear_phase), a
    cp LEVEL_CLEAR_NUM_PHASES
    ret nc

    ; Odd phases show white, even phases show blue (we ended phase 0 on
    ; blue — no redraw needed there since play left it that way).
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
    ld bc, origin_left + 8
    ld de, origin_top + 8
    call vdu_set_gfx_origin
    ld ix, maze_wall
    ld hl, SPRITE_MAZE_TILE_00              ; ignored by draw_maze; struct wins
    call draw_maze
    ld bc, origin_left
    ld de, origin_top
    call vdu_set_gfx_origin
    ret

@post_hold:
    ld hl, state_level_clear_hold_timer
    dec (hl)
    ret nz

    ; Defensive: make sure the maze base is back to blue before READY
    ; redraws (the last flash phase was white).
    ld hl, SPRITE_MAZE_TILE_00
    ld (maze_wall_base_bufferId), hl

    ld a, (current_level)
    inc a
    ld (current_level), a
    call fruit_init
    call level_init

    ld a, GS_READY
    jp game_state_set
