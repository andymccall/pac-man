; GS_ATTRACT_SPLASH — bitriot logo splash. Inputs:
;   SPACE — skip to the character reveal (CHARS).
;   C     — fast-forward (#41): upload sprite data, deposit a credit,
;           and jump straight to PLAYER_SELECT, bypassing the full
;           reveal animation. Non-arcade — quality-of-life departure.

state_attract_splash_counter: db 180

state_attract_splash_enter:
    call vdu_buffer_clear_all
    ld hl, vdu_splash_data
    ld bc, vdu_splash_data_end - vdu_splash_data
    rst.lil VDU_OUTPUT_TO_VDP
    call small_screen
    ld a, 180
    ld (state_attract_splash_counter), a
    ret

state_attract_splash_tick:
    ld a, mos_getkbmap
    rst.lil $08

    ; C — fast-forward straight to PLAYER_SELECT. Have to upload the
    ; sprite data here too, since we're skipping CHARS_enter which would
    ; normally do it. Sprites are needed once PLAYER_SELECT → READY runs.
    ld a, (ix + $0A)
    bit 2, a
    jr nz, @fast_forward

    ; Space — advance to the character reveal.
    ld a, (ix + $0C)
    bit 2, a
    jr nz, @advance

    ld hl, state_attract_splash_counter
    dec (hl)
    ret nz

@advance:
    ld a, GS_ATTRACT_CHARS
    jp game_state_set

@fast_forward:
    call attract_ensure_game_data_loaded
    call credit_deposit
    ld a, GS_PLAYER_SELECT
    jp game_state_set
