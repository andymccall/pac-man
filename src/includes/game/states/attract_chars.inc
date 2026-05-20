; GS_ATTRACT_CHARS — character / nickname reveal animation.
;
; Phases, each preceded by a wait of the given number of frames:
;   on entry: heading shown (UI labels + "CHARACTER / NICKNAME")
;   60f -> phase 1: draw Blinky sprite
;   40f -> phase 2: print "-SHADOW" (Blinky char name)
;   40f -> phase 3: print "BLINKY"  (Blinky nick name)
;   40f -> phase 4: empty pause (mirrors original pinky_wait)
;   40f -> phase 5: draw Pinky/Inky/Clyde + names, pellet labels,
;                   trade/copy text, credit display; transition to HOLD.

vdu_game_data_loaded:        db 0      ; one-shot guard for bitmap upload
state_attract_chars_phase:   db 0
state_attract_chars_timer:   db 60

state_attract_chars_enter:
    call vdu_screen_graphics_clear

    ld a, (vdu_game_data_loaded)
    or a
    jr nz, @skip_upload
    ld hl, vdu_game_data
    ld bc, vdu_game_data_end - vdu_game_data
    rst.lil VDU_OUTPUT_TO_VDP
    ld a, 1
    ld (vdu_game_data_loaded), a
@skip_upload:

    xor a
    ld (state_attract_chars_phase), a
    ld a, 60
    ld (state_attract_chars_timer), a

    macro_text_set_color VDU_COL_WHITE
    call display_1up
    call display_high_score
    call display_2up
    ld hl, character_nickname_heading
    call vdu_text_print
    ret

state_attract_chars_tick:
    ld hl, state_attract_chars_timer
    dec (hl)
    ret nz
    ld a, (state_attract_chars_phase)
    inc a
    ld (state_attract_chars_phase), a
    cp 1
    jp z, @phase_1_blinky
    cp 2
    jp z, @phase_2_shadow
    cp 3
    jp z, @phase_3_blinky_nick
    cp 4
    jp z, @phase_4_pause
    cp 5
    jp z, @phase_5_final
    ret

@phase_1_blinky:
    ld hl, SPRITE_BLINKY_MENU_00
    call vdu_buffer_select
    ld bc, 155
    ld de, 91
    call vdu_bitmap_plot
    ld a, 40
    ld (state_attract_chars_timer), a
    ret

@phase_2_shadow:
    macro_text_set_color VDU_COL_BRIGHT_RED
    ld hl, blinky_char_name
    call vdu_text_print
    ld a, 40
    ld (state_attract_chars_timer), a
    ret

@phase_3_blinky_nick:
    macro_text_set_color VDU_COL_BRIGHT_RED
    ld hl, blinky_nick_name
    call vdu_text_print
    ld a, 40
    ld (state_attract_chars_timer), a
    ret

@phase_4_pause:
    ld a, 40
    ld (state_attract_chars_timer), a
    ret

@phase_5_final:
    ld hl, SPRITE_PINKY_MENU_00
    call vdu_buffer_select
    ld bc, 155
    ld de, 115
    call vdu_bitmap_plot
    macro_text_set_color 0x3D
    ld hl, pinky_name
    call vdu_text_print

    ld hl, SPRITE_INKY_MENU_00
    call vdu_buffer_select
    ld bc, 155
    ld de, 139
    call vdu_bitmap_plot
    macro_text_set_color 0x16
    ld hl, inky_name
    call vdu_text_print

    ld hl, SPRITE_CLYDE_MENU_00
    call vdu_buffer_select
    ld bc, 155
    ld de, 163
    call vdu_bitmap_plot
    macro_text_set_color 0x36
    ld hl, clyde_name
    call vdu_text_print

    ld hl, SPRITE_PELLET_00
    call vdu_buffer_select
    ld bc, 207
    ld de, 247
    call vdu_bitmap_plot
    macro_text_set_color VDU_COL_WHITE
    ld hl, pellet_points
    call vdu_text_print

    ld hl, SPRITE_POWER_PELLET_00
    call vdu_buffer_select
    ld bc, 207
    ld de, 264
    call vdu_bitmap_plot
    ld hl, power_pellet_points
    call vdu_text_print

    ld hl, trade_and_copy_message
    call vdu_text_print
    call credit_display

    ld a, GS_ATTRACT_HOLD
    jp game_state_set
