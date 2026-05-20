; GS_ATTRACT_HOLD — title screen. The characters, names, pellet legend
; and Midway/Bitriot copyright are already on screen from ATTRACT_CHARS,
; plus a power pellet on the left of the screen at the "chase row"
; (planted by CHARS phase 14). HOLD's job:
;
;   * Blink the chase-row power pellet (~3.75 Hz, same cadence as the
;     in-maze energiser blink) so it draws the eye while the player
;     decides to insert credit.
;   * Watch for input:
;       C — deposits a credit. Once credit ≥ 1, transition to
;           GS_PLAYER_SELECT (the second screen, which shows the
;           1 PLAYER / 2 PLAYER options, BONUS line, and blinking
;           PUSH START).
;
; The arcade's first title screen has no PUSH START or coin prompt at
; all — only the credit count and the flashing pellet. Those prompts
; appear on PLAYER_SELECT.

; Chase-row power pellet — aligned with the ghost reveal column at
; x=155 but nudged right so it doesn't sit directly under Clyde. The
; row is between Clyde (y=163) and the 10pts legend (y=247).
ATTRACT_CHASE_PELLET_X:    equ 160
ATTRACT_CHASE_PELLET_Y:    equ 200

; 50pts legend power pellet position (must match @phase_pellet_legend /
; @phase_final in attract_chars.inc). HOLD redraws this so it blinks in
; sync with the chase pellet — gives the title screen the same flicker
; cadence as the in-maze energisers during play.
ATTRACT_LEGEND_PELLET_X:   equ 207
ATTRACT_LEGEND_PELLET_Y:   equ 264

; ~3.75 Hz blink — matches the in-maze energiser cadence
; (POWER_PELLET_BLINK_PERIOD in pellet_eat.inc).
ATTRACT_CHASE_PELLET_BLINK_PERIOD: equ 8
attract_chase_pellet_blink_counter: db ATTRACT_CHASE_PELLET_BLINK_PERIOD
attract_chase_pellet_visible:       db 1

state_attract_hold_enter:
    ld a, ATTRACT_CHASE_PELLET_BLINK_PERIOD
    ld (attract_chase_pellet_blink_counter), a
    ld a, 1
    ld (attract_chase_pellet_visible), a
    ret

state_attract_hold_tick:
    ; Update the blink phase, then redraw the pellet every frame —
    ; either the energiser bitmap or the opaque-black erase bitmap.
    ld hl, attract_chase_pellet_blink_counter
    dec (hl)
    jr nz, @paint_pellet

    ld a, ATTRACT_CHASE_PELLET_BLINK_PERIOD
    ld (hl), a
    ld a, (attract_chase_pellet_visible)
    xor 1
    ld (attract_chase_pellet_visible), a

@paint_pellet:
    ld a, (attract_chase_pellet_visible)
    or a
    jr z, @paint_off
    ld hl, SPRITE_POWER_PELLET_00
    jr @paint_now
@paint_off:
    ld hl, SPRITE_ERASE_PELLET_00
@paint_now:
    ; Same buffer select covers both plots — vdu_buffer_select latches
    ; the chosen bitmap until another select is issued.
    call vdu_buffer_select
    ld bc, ATTRACT_CHASE_PELLET_X
    ld de, ATTRACT_CHASE_PELLET_Y
    call vdu_bitmap_plot
    ld bc, ATTRACT_LEGEND_PELLET_X
    ld de, ATTRACT_LEGEND_PELLET_Y
    call vdu_bitmap_plot

    ld a, mos_getkbmap
    rst.lil $08

    ; C — deposit credit.
    ld a, (ix + $0A)
    bit 2, a
    call nz, credit_deposit

    ; Any credit available? → switch to PLAYER_SELECT (the second screen).
    ld a, (credit)
    or a
    ret z
    ld a, GS_PLAYER_SELECT
    jp game_state_set
