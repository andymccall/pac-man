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
    ; Reset graphics origin to (0, 0). state_ready_enter offsets it to
    ; (origin_left, origin_top) = (138, 60) so maze plots can use cell-
    ; relative coords, and never resets it. Without this, the menu
    ; ghost-reveal bitmap plots in the attract phases land at
    ; (138+155, 60+91) = (293, 151) etc. instead of (155, 91) — which is
    ; exactly what showed up on attract loops after the first game.
    ld bc, 0
    ld de, 0
    call vdu_set_gfx_origin

    call vdu_screen_graphics_clear
    call attract_ensure_game_data_loaded

    ; Defensive: hide every sprite slot. Has to run AFTER
    ; attract_ensure_game_data_loaded — the first time through it does
    ; the upload + macro_sprite_activate 16; before that, "hide sprite N"
    ; is a no-op because the sprite doesn't exist yet. On subsequent
    ; attract entries (after GAME OVER) the upload is skipped and the
    ; hides catch anything left visible by the previous game.
    call attract_hide_game_sprites

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
    ; Fast-forward (#41): C at any point in the reveal deposits a credit
    ; and jumps straight to PLAYER_SELECT. Non-arcade — quality-of-life
    ; departure so dev/play doesn't have to sit through the full reveal.
    ld a, mos_getkbmap
    rst.lil $08
    ld a, (ix + $0A)
    bit 2, a
    jp nz, @fast_forward

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

@fast_forward:
    call credit_deposit
    ld a, GS_PLAYER_SELECT
    jp game_state_set

; attract_ensure_game_data_loaded — uploads the bitmap/sprite-frame data
; block to the VDP once per session. Safe to call multiple times; the
; vdu_game_data_loaded flag short-circuits subsequent calls. Used by:
;   * state_attract_chars_enter (normal first-time entry)
;   * state_attract_splash_tick (#41 fast-forward — skips CHARS)
; If this runs from the SPLASH fast-forward path, the sprites are
; available immediately when PLAYER_SELECT → READY → PLAY starts the
; game without the reveal animation ever running.
; Destroys: af, bc, hl
attract_ensure_game_data_loaded:
    ld a, (vdu_game_data_loaded)
    or a
    ret nz
    ld hl, vdu_game_data
    ld bc, vdu_game_data_end - vdu_game_data
    rst.lil VDU_OUTPUT_TO_VDP
    ld a, 1
    ld (vdu_game_data_loaded), a
    ret

; attract_hide_game_sprites — defensively re-hide every play-state
; sprite (Pac, cherry, the four ghosts) at the start of every attract
; loop. state_game_over_enter already hides them on the way out, but
; re-issuing here is cheap insurance against any path that bypasses
; that state (e.g. a future "exit to attract" debug shortcut).
; Destroys: af, bc, hl
attract_hide_game_sprites:
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
    jp vdu_sprite_hide
