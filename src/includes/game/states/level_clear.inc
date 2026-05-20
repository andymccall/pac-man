; GS_LEVEL_CLEAR — Pac just ate the 244th pellet.
;
; Minimal first pass (#14): hide the ghosts so they don't sit awkwardly
; on a cleared maze, hold for ~2 s with the static maze on screen, then
; advance the level counter, restore the pellet map, and transition to
; GS_READY for the next level. The maze-flash animation lives in #40.

LEVEL_CLEAR_DURATION:    equ 120          ; ~2 s @ 60 fps
state_level_clear_timer: db LEVEL_CLEAR_DURATION

state_level_clear_enter:
    ; Hide ghosts immediately (matches state_dying_enter). Pac stays
    ; visible on the last tile he was on.
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

    ld a, LEVEL_CLEAR_DURATION
    ld (state_level_clear_timer), a
    ret

state_level_clear_tick:
    ; Keep the HUD ticking so the hold doesn't look frozen.
    call display_1up
    call display_high_score
    call display_2up
    call credit_display
    macro_text_set_color VDU_COL_WHITE
    call display_player_1_score

    ld hl, state_level_clear_timer
    dec (hl)
    ret nz

    ; Timer expired — advance.
    ld a, (current_level)
    inc a
    ld (current_level), a
    call fruit_init                       ; resets shown_count + hides sprite
    call level_init                       ; restores pellet map + counter

    ld a, GS_READY
    jp game_state_set
