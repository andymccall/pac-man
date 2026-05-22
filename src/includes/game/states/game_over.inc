; GS_GAME_OVER — out of lives. Show "GAME OVER" in red where READY was,
; hold ~3 seconds, then return to attract mode.

state_game_over_timer: dl 180     ; ~3 s at 60 fps

state_game_over_enter:
    ; Persist the high score if this run beat the stored top. No-op if
    ; player_1_score <= cfg_high_score; otherwise writes PACMAN.CFG.
    call config_high_score_commit

    ; Hide every play-state sprite so the attract menu we drop back into
    ; doesn't render leftover Pac/cherry on top of the character roster.
    ; Ghosts were already hidden by state_dying_enter; we redo them here
    ; defensively so this state is self-contained.
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    call vdu_sprite_hide
    ld a, SPRITE_CHERRY_0_ID
    call vdu_sprite_select
    call vdu_sprite_hide
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

    ; Plot GAME OVER as a pre-rendered red bitmap so it sits per-pixel
    ; in maze row 17 (same vertical position as READY!).
    ld hl, BANNER_GAME_OVER
    call vdu_buffer_select
    ld bc, BANNER_GAME_OVER_REL_X
    ld de, BANNER_REL_Y
    call vdu_bitmap_plot

    ld hl, 180
    ld (state_game_over_timer), hl
    ret

state_game_over_tick:
    call display_1up
    call display_high_score
    call display_2up
    call credit_display
    macro_text_set_color VDU_COL_WHITE
    call display_player_1_score

    ld hl, (state_game_over_timer)
    ld de, 0
    or a
    sbc hl, de
    ret z                         ; safety: timer already at 0
    dec hl
    ld (state_game_over_timer), hl
    ld de, 0
    or a
    sbc hl, de
    ret nz

    ; After game over we go to the character / credit menu, not the
    ; bitriot splash — the splash is a one-shot boot screen.
    ld a, GS_ATTRACT_CHARS
    jp game_state_set

