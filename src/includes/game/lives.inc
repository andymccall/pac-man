; Lives counter and on-screen display.
;
; lives is initialised to LIVES_START (3) when a new game starts (from
; state_player_select); decremented in state_dying_enter; checked there
; to choose between respawn (READY) and game over (GAME_OVER).
;
; The HUD shows one Pac-Man icon per remaining life along the bottom of
; the maze, just below the playfield.

LIVES_START:     equ 3

; Bonus-life threshold. Arcade default is 10,000 points; arcade also has a
; DIP-switch for 15k/20k/none — deferred (see arcade-fidelity-backlog).
BONUS_LIFE_SCORE: equ 10000

lives:                 db 3
bonus_life_awarded:    db 0     ; 1 once the 10k bonus has been given this game

; Pixel position of the leftmost life icon. Successive icons are placed
; SPRITE_PAC_MAN_LIFE_WIDTH pixels to the right.
lives_display_x: equ 152
lives_display_y: equ 340

lives_x_scratch: dl 0            ; current x while looping through icons

; lives_init — set lives back to LIVES_START and clear the bonus-life
; flag. Called on new game.
; Destroys: af
lives_init:
    ld a, LIVES_START
    ld (lives), a
    xor a
    ld (bonus_life_awarded), a
    ret

; bonus_life_check — if the player's score has crossed BONUS_LIFE_SCORE
; and we haven't already awarded the bonus, increment lives and set the
; flag. Idempotent after the first award (so calling it every frame is
; fine).
; Destroys: af, bc, de, hl
bonus_life_check:
    ld a, (bonus_life_awarded)
    or a
    ret nz

    ld hl, (player_1_score)
    ld de, BONUS_LIFE_SCORE
    or a
    sbc hl, de
    ret c                            ; score < threshold

    ld a, 1
    ld (bonus_life_awarded), a
    ld a, (lives)
    inc a
    ld (lives), a
    ret

; lives_decrement — A = remaining lives after decrement.
; Destroys: af
lives_decrement:
    ld a, (lives)
    or a
    ret z                        ; already 0; nothing to do
    dec a
    ld (lives), a
    ret

; lives_display — draw one Pac icon per remaining life along the bottom
; of the maze. Re-blitted each frame so it survives screen clears.
; Destroys: af, bc, de, hl
lives_display:
    ld a, (lives)
    or a
    ret z
    ld b, a                      ; b = loop counter (remaining lives)

    ld hl, lives_display_x
    ld (lives_x_scratch), hl

@loop:
    push bc                      ; save counter

    ld hl, SPRITE_PAC_MAN_LIFE_0
    call vdu_buffer_select

    ld bc, (lives_x_scratch)
    ld de, lives_display_y
    call vdu_bitmap_plot

    ; advance x by one icon width for the next iteration
    ld hl, (lives_x_scratch)
    ld de, SPRITE_PAC_MAN_LIFE_WIDTH
    add hl, de
    ld (lives_x_scratch), hl

    pop bc
    djnz @loop
    ret
