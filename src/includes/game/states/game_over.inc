; GS_GAME_OVER — out of lives. Show "GAME OVER" in red where READY was,
; hold ~3 seconds, then return to attract mode.

state_game_over_timer: dl 180     ; ~3 s at 60 fps

state_game_over_enter:
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

    macro_text_set_color VDU_COL_BRIGHT_RED
    ld hl, game_over_txt
    call vdu_text_print
    macro_text_set_color VDU_COL_WHITE

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

game_over_txt:
    .db     31, 27, 26
    .db     "GAME OVER", 0
game_over_txt_end:
