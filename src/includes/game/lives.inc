; Lives counter and on-screen display.
;
; lives is initialised to cfg_lives_start (3 by default, 5 via DIP) when a
; new game starts (from state_player_select); decremented in
; state_dying_enter; checked there to choose between respawn (READY) and
; game over (GAME_OVER).
;
; The HUD shows one Pac-Man icon per remaining life along the bottom of
; the maze, just below the playfield.

; Maximum starting lives — sets the width of the on-screen reserves strip
; and the clear rect in lives_pop. The runtime cfg_lives_start can be 3
; or 5; using the max here means the strip always has room for either.
LIVES_MAX:       equ 5

; Bonus-life threshold. Arcade default is 10,000 points; arcade also has a
; DIP-switch for 15k/20k/none — deferred (see arcade-fidelity-backlog).
BONUS_LIFE_SCORE: equ 10000

lives:                 db 3
bonus_life_awarded:    db 0     ; 1 once the 10k bonus has been given this game

; 1 while a READY phase is pending a HUD deduction (set by game-start
; and by dying.inc; consumed by state_ready_tick at the +1 s mark).
; Cleared by lives_pop after it fires. Level-clear → READY transitions
; deliberately do NOT set this — those re-enter READY without burning
; a life.
lives_pop_pending:     db 0

; Pixel position of the leftmost life icon. Successive icons are placed
; SPRITE_PAC_MAN_LIFE_WIDTH pixels to the right.
lives_display_x: equ 162
lives_display_y: equ 320

lives_x_scratch: dl 0            ; current x while looping through icons

; lives_init — set lives back to cfg_lives_start (from PACMAN.CFG; see
; config.inc) and clear the bonus-life flag. Called on new game.
; Destroys: af
lives_init:
    ld a, (cfg_lives_start)
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

    ; 5-note ascending fanfare. Fires once per game (the flag above
    ; gates it).
    jp melody_start_bonus_life

; lives_pop — if a deduction is pending, decrement lives by 1 and
; visually remove the rightmost icon from the HUD strip. Called every
; tick at the +1 s mark of READY; the lives_pop_pending gate makes the
; call a no-op for level-clear → READY (which mustn't burn a life).
;
; Why this lives in READY rather than DYING: arcade-style "HUD =
; reserves" semantics. The brief flash of the full count at the start
; of a game (or after a death) is the intro; the deduction at +1 s
; settles into the reserves-only display that holds until the next
; death. On the last life that "reserves-only" display is empty.
; Destroys: af, bc, de, hl, ix, iy
lives_pop:
    ld a, (lives_pop_pending)
    or a
    ret z
    xor a
    ld (lives_pop_pending), a

    ld a, (lives)
    or a
    ret z
    dec a
    ld (lives), a

    ; Erase the strip with a black rect, then let lives_display
    ; repaint the remaining icons. The rect coords are RELATIVE to
    ; the current graphics origin (origin_left, origin_top) — we do
    ; NOT set origin to (0,0) here, because doing so puts two
    ; back-to-back VDU 29 calls right next to each other (this one
    ; plus the one lives_display does at its top), and that combo
    ; consistently corrupts subsequent rendering on the VDP
    ; (confirmed via blind A/B/C tests). Drawing at relative coords
    ; avoids the duplicate VDU 29 entirely.
    ld a, VDU_COL_BLACK
    call vdu_plot_set_fill

    ; Relative coords for the strip — same math as the absolute
    ; version, just minus the origin. Multiply-first to dodge
    ; ez80asm's left-to-right precedence (see
    ; project_ez80asm_no_precedence).
    ld bc, lives_display_x - origin_left
    ld de, lives_display_y - origin_top
    ld ix, LIVES_MAX * SPRITE_PAC_MAN_LIFE_WIDTH + lives_display_x - origin_left - 1
    ld iy, lives_display_y + SPRITE_PAC_MAN_LIFE_HEIGHT - origin_top - 1
    call vdu_plot_filled_rect

    jp lives_display             ; tail-call: repaints and restores origin

; lives_display — draw one Pac icon per remaining life along the bottom
; of the maze. Re-blitted by @draw_static so it survives screen clears.
;
; Plots in screen-absolute coords — the maze setup leaves the gfx
; origin at (origin_left, origin_top) = (138, 60), which would push the
; HUD off the bottom of the screen. Reset origin to (0, 0) for the
; plot, then restore. (Without this, lives_display_y=340 + 60 = 400,
; 16 px past the 384-pixel screen bottom — invisible.)
; Destroys: af, bc, de, hl
lives_display:
    ld a, (lives)
    or a
    ret z
    ld b, a                      ; b = loop counter (remaining lives)

    push bc
    ld bc, 0
    ld de, 0
    call vdu_set_gfx_origin
    pop bc

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

    ; Restore gfx origin so subsequent gameplay bitmap plots (e.g.
    ; pellet_eat erasing a dot) land at the right place.
    ld bc, origin_left
    ld de, origin_top
    jp vdu_set_gfx_origin
