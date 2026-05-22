display_1up:
    ;Print the 1UP text
    ld hl, up1_txt
    ld bc, up1_txt_end - up1_txt
    rst.lil VDU_OUTPUT_TO_VDP
    ret

; --- Active-player blink (#71) ----------------------------------------------
;
; The active player's 1UP label blinks while their turn is being played.
; Has its own counter / visible flag — independent of the power-pellet
; blink, slower (~1 Hz cycle, period 30 = half a second per phase) per
; user preference. The blink ticks inside display_1up_blink itself so
; the caller doesn't have to tick it separately.
DISPLAY_1UP_BLINK_PERIOD: equ 30          ; frames per visible / hidden phase

display_1up_blink_counter: db DISPLAY_1UP_BLINK_PERIOD
display_1up_visible:       db 1           ; 1 = white, 0 = black (overwrites)

; display_1up_blink — paint the 1UP label, ticking its own blink
; counter so the cell alternates white ↔ black at ~1 Hz. Always exits
; with the text colour set back to white so subsequent display_*
; calls paint normally.
;
; Use this in PLAY only. In states where there's no active turn
; (READY / DYING / LEVEL_CLEAR / attract) keep calling display_1up
; for a static label, OR call display_1up_blink_reset on entry to
; the next PLAY so the label starts visible.
; Destroys: af, bc, hl
display_1up_blink:
    ld hl, display_1up_blink_counter
    dec (hl)
    jr nz, @paint
    ld (hl), DISPLAY_1UP_BLINK_PERIOD
    ld a, (display_1up_visible)
    xor 1
    ld (display_1up_visible), a
@paint:
    ld a, (display_1up_visible)
    or a
    jr z, @hidden
    macro_text_set_color VDU_COL_WHITE
    jp display_1up
@hidden:
    macro_text_set_color VDU_COL_BLACK
    call display_1up
    macro_text_set_color VDU_COL_WHITE
    ret

; display_1up_blink_reset — restore the blink to "visible, full
; period". Call on PLAY enter so each life / level starts with the
; label visible and a full half-second before the first toggle.
; Destroys: af
display_1up_blink_reset:
    ld a, DISPLAY_1UP_BLINK_PERIOD
    ld (display_1up_blink_counter), a
    ld a, 1
    ld (display_1up_visible), a
    ret

display_2up:
    ; Print the 2UP text
    ld hl, up2_txt
    ld bc, up2_txt_end - up2_txt
    rst.lil VDU_OUTPUT_TO_VDP
    ret

display_high_score:
    ; Label.
    ld hl, high_score_data
    ld bc, high_score_data_end - high_score_data
    rst.lil VDU_OUTPUT_TO_VDP

    ; Numeric value below the label. Painted as the live max of
    ; cfg_high_score (loaded at boot, persisted at GAME_OVER) and the
    ; current player_1_score — so when the active player passes the
    ; stored top, the on-screen value tracks them in real time.
    macro_text_set_color VDU_COL_WHITE
    ld hl, high_score_pos
    call vdu_text_print

    ld hl, (player_1_score)
    ld de, (cfg_high_score)
    or a
    sbc hl, de
    jr c, @show_stored                  ; player < stored
    ld hl, (player_1_score)
    jp score_print
@show_stored:
    ld hl, (cfg_high_score)
    jp score_print

high_score_pos:
    ; Cursor for the numeric value — col 29, row 4 (centred under
    ; "HIGH SCORE" at col 27, which spans cols 27..36).
    .db     31, 29, 4, 0