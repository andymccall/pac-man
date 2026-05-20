; GS_ATTRACT_CHARS — character / nickname reveal animation.
;
; Arcade-faithful flow: each ghost is introduced individually with a
; sprite, then their character name (-SHADOW etc.), then their nickname
; (in brackets). Once all four are revealed, the 10pts / 50pts pellet
; legend appears, then the trade-and-copy line, and the state hands off
; to ATTRACT_HOLD.
;
; Phase  Action                                  Wait before next (frames)
; ------ --------------------------------------- ------------------------
;   0    on entry: heading                       60
;   1    Blinky sprite                           40
;   2    Blinky char name (-SHADOW)              40
;   3    Blinky nick name (BLINKY)               40
;   4    Pinky sprite                            40
;   5    Pinky char name (-SPEEDY)               40
;   6    Pinky nick name (PINKY)                 40
;   7    Inky sprite                             40
;   8    Inky char name (-BASHFUL)               40
;   9    Inky nick name (INKY)                   40
;  10    Clyde sprite                            40
;  11    Clyde char name (-POKEY)                40
;  12    Clyde nick name (CLYDE)                 60
;  13    10 pts pellet legend (small pellet)     60
;  14    50 pts pellet legend + trade-and-copy
;        + credit, → HOLD                        —
;
; Arcade behaviour: the © 1980 MIDWAY line appears at the same beat as
; the 50pts power pellet — not as a later step — so phase 14 draws
; both together and then transitions immediately.

CHARS_HEADING_WAIT:    equ 60        ; pause after the heading appears
CHARS_REVEAL_WAIT:     equ 40        ; standard reveal cadence (sprite / text)
CHARS_FINAL_WAIT:      equ 60        ; longer pauses before the pellet legend
                                     ; and the transition to HOLD

vdu_game_data_loaded:        db 0    ; one-shot guard for bitmap upload
state_attract_chars_phase:   db 0
state_attract_chars_timer:   db CHARS_HEADING_WAIT

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
    ld a, CHARS_HEADING_WAIT
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

    ; Dispatch on phase. cp-ladder is fine for ~14 entries and keeps the
    ; per-phase action obvious.
    cp 1
    jp z, @phase_blinky_sprite
    cp 2
    jp z, @phase_blinky_char
    cp 3
    jp z, @phase_blinky_nick
    cp 4
    jp z, @phase_pinky_sprite
    cp 5
    jp z, @phase_pinky_char
    cp 6
    jp z, @phase_pinky_nick
    cp 7
    jp z, @phase_inky_sprite
    cp 8
    jp z, @phase_inky_char
    cp 9
    jp z, @phase_inky_nick
    cp 10
    jp z, @phase_clyde_sprite
    cp 11
    jp z, @phase_clyde_char
    cp 12
    jp z, @phase_clyde_nick
    cp 13
    jp z, @phase_pellet_legend
    cp 14
    jp z, @phase_final
    ret

; ---- Per-phase actions ---------------------------------------------------
; Each one performs its draw, sets the next wait, and returns. The default
; wait is CHARS_REVEAL_WAIT; only the post-final-nick and post-pellet-legend
; phases use CHARS_FINAL_WAIT for a slightly longer hold.

@phase_blinky_sprite:
    ld hl, SPRITE_BLINKY_MENU_00
    call vdu_buffer_select
    ld bc, 155
    ld de, 91
    call vdu_bitmap_plot
    jp @set_reveal_wait

@phase_blinky_char:
    macro_text_set_color VDU_COL_BRIGHT_RED
    ld hl, blinky_char_name
    call vdu_text_print
    jp @set_reveal_wait

@phase_blinky_nick:
    macro_text_set_color VDU_COL_BRIGHT_RED
    ld hl, blinky_nick_name
    call vdu_text_print
    jp @set_reveal_wait

@phase_pinky_sprite:
    ld hl, SPRITE_PINKY_MENU_00
    call vdu_buffer_select
    ld bc, 155
    ld de, 115
    call vdu_bitmap_plot
    jp @set_reveal_wait

@phase_pinky_char:
    macro_text_set_color 0x3D
    ld hl, pinky_char_name
    call vdu_text_print
    jp @set_reveal_wait

@phase_pinky_nick:
    macro_text_set_color 0x3D
    ld hl, pinky_nick_name
    call vdu_text_print
    jp @set_reveal_wait

@phase_inky_sprite:
    ld hl, SPRITE_INKY_MENU_00
    call vdu_buffer_select
    ld bc, 155
    ld de, 139
    call vdu_bitmap_plot
    jp @set_reveal_wait

@phase_inky_char:
    macro_text_set_color 0x16
    ld hl, inky_char_name
    call vdu_text_print
    jp @set_reveal_wait

@phase_inky_nick:
    macro_text_set_color 0x16
    ld hl, inky_nick_name
    call vdu_text_print
    jp @set_reveal_wait

@phase_clyde_sprite:
    ld hl, SPRITE_CLYDE_MENU_00
    call vdu_buffer_select
    ld bc, 155
    ld de, 163
    call vdu_bitmap_plot
    jp @set_reveal_wait

@phase_clyde_char:
    macro_text_set_color 0x36
    ld hl, clyde_char_name
    call vdu_text_print
    jp @set_reveal_wait

@phase_clyde_nick:
    macro_text_set_color 0x36
    ld hl, clyde_nick_name
    call vdu_text_print
    ld a, CHARS_FINAL_WAIT
    ld (state_attract_chars_timer), a
    ret

@phase_pellet_legend:
    ld hl, SPRITE_PELLET_00
    call vdu_buffer_select
    ld bc, 207
    ld de, 247
    call vdu_bitmap_plot
    macro_text_set_color VDU_COL_WHITE
    ld hl, pellet_points
    call vdu_text_print
    ld a, CHARS_FINAL_WAIT
    ld (state_attract_chars_timer), a
    ret

@phase_final:
    ; 50 pts power pellet, the © 1980 / AGON-port credit line, and the
    ; flashing "chase row" power pellet on the left of the screen all
    ; land together — matches the arcade title-screen beat. ATTRACT_HOLD
    ; takes over the chase-pellet blink from here.
    ld hl, SPRITE_POWER_PELLET_00
    call vdu_buffer_select
    ld bc, 207
    ld de, 264
    call vdu_bitmap_plot
    macro_text_set_color VDU_COL_WHITE
    ld hl, power_pellet_points
    call vdu_text_print
    ld hl, trade_and_copy_message
    call vdu_text_print
    call credit_display

    ld hl, SPRITE_POWER_PELLET_00
    call vdu_buffer_select
    ld bc, ATTRACT_CHASE_PELLET_X
    ld de, ATTRACT_CHASE_PELLET_Y
    call vdu_bitmap_plot

    ld a, GS_ATTRACT_HOLD
    jp game_state_set

@set_reveal_wait:
    ld a, CHARS_REVEAL_WAIT
    ld (state_attract_chars_timer), a
    ret
