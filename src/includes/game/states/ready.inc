; GS_READY — draw maze, place sprites, hold for ~2 seconds, then to PLAY.

state_ready_timer: db 120     ; ~2 sec at 60 fps

state_ready_enter:
    call @draw_static

    ; Reset Pac-Man.
    ; Order matters: vdu_sprite_select clobbers BC (and rst.lil $18 likely
    ; clobbers DE), so select first, then load coordinates fresh into BC/DE
    ; for vdu_sprite_move_abs.
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    ld bc, 250
    ld (sprite_x), bc
    ld de, 248
    ld (sprite_y), de
    call vdu_sprite_move_abs
    call vdu_sprite_show
    call pac_reset

    ; Hide any fruit left over from the previous life (the appearance
    ; counter is preserved so we don't re-trigger 70/170 thresholds).
    call fruit_hide_now

    ; Ghosts: ghost_ai_init resets the AI state, moves each sprite to
    ; its start position, AND tail-calls ghosts_animate which already
    ; sets each ghost to a per-ghost-staggered frame (Blinky +0, Pinky
    ; +1, Inky +2, Clyde +3) inside the normal 0-7 range. So we just
    ; need to make each sprite visible.
    ;
    ; (Earlier versions called vdu_sprite_next_frame here for staggering,
    ; but those wrapped at 16 not 8 — and 16 is the full frame list
    ; including blue/white-flash/eyes — so dying without a power pellet
    ; could push Inky/Clyde into the blue or eyes range. See #4.)
    call ghost_ai_init

    ld a, SPRITE_BLINKY_ID
    call vdu_sprite_select
    call vdu_sprite_show

    ld a, SPRITE_PINKY_ID
    call vdu_sprite_select
    call vdu_sprite_show

    ld a, SPRITE_INKY_ID
    call vdu_sprite_select
    call vdu_sprite_show

    ld a, SPRITE_CLYDE_ID
    call vdu_sprite_select
    call vdu_sprite_show

    ld a, 120
    ld (state_ready_timer), a
    ret

; @draw_static — paint the maze + pellets + READY banner + lives HUD onto
; whichever buffer is currently the back. Called twice from
; state_ready_enter (once per buffer) so both stay in sync.
; Destroys: af, bc, de, hl, ix
@draw_static:
    call vdu_screen_graphics_clear

    ld bc, origin_left + 8
    ld de, origin_top + 8
    call vdu_set_gfx_origin

    ld ix, maze_wall
    ld hl, SPRITE_MAZE_TILE_00
    call draw_maze
    ld ix, maze_pellets
    ld hl, SPRITE_NULL_PELLET_00
    call draw_maze

    ld bc, origin_left
    ld de, origin_top
    call vdu_set_gfx_origin

    ; Plot the READY! banner as a pre-rendered yellow bitmap. Lets us
    ; position it per-pixel so it sits cleanly in maze row 17 (the open
    ; lane below the pen) rather than overlapping a wall row.
    ld hl, BANNER_READY
    call vdu_buffer_select
    ld bc, BANNER_READY_REL_X
    ld de, BANNER_REL_Y
    call vdu_bitmap_plot

    ; Lives HUD: re-blit so post-death decrements show.
    jp lives_display

state_ready_tick:
    call display_1up
    call display_high_score
    call display_2up
    call credit_display
    macro_text_set_color VDU_COL_WHITE
    call display_player_1_score

    ld hl, state_ready_timer
    dec (hl)
    ret nz
    ld a, GS_PLAY
    jp game_state_set
