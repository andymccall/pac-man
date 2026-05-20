; GS_ATTRACT_SPLASH — bitriot logo splash, timed or skipped with space.

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
    ; Space key skips
    ld a, mos_getkbmap
    rst.lil $08
    ld a, (ix + $0C)
    bit 2, a
    jr nz, @advance
    ld hl, state_attract_splash_counter
    dec (hl)
    ret nz
@advance:
    ld a, GS_ATTRACT_CHARS
    jp game_state_set
