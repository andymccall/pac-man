; GS_ATTRACT_HOLD — title screen with the chase animation (#39).
;
; Sequence:
;   PRE      — title shown, flashing pellet blinks, brief pause.
;   LEFT     — Pac enters from the right and runs left along the chase
;              row at y=196 toward the flashing power pellet at x=160.
;              Shadow / Speedy / Bashful / Pokey trail him in that order.
;   RIGHT    — Pac eats the pellet (it disappears), the four ghosts turn
;              blue and reverse direction. Pac reverses too and chases
;              them right; he's faster than they are, so he catches each
;              one in order. Eaten ghosts vanish and the chain bonus
;              (200 / 400 / 800 / 1600) pops up at the catch position.
;   HOLD1600 — pause showing the 1600 popup.
;   DONE     — transition to GS_PLAYER_SELECT.
;
; Input handling is preserved from the pre-#39 HOLD: C deposits a
; credit and jumps to PLAYER_SELECT immediately; 1 / 2 with banked
; credit starts a game. Either way the chase is cancelled.

; --- Chase layout / timing -------------------------------------------------

ATTRACT_CHASE_PELLET_X:    equ 160
ATTRACT_CHASE_PELLET_Y:    equ 200
ATTRACT_LEGEND_PELLET_X:   equ 207
ATTRACT_LEGEND_PELLET_Y:   equ 264

ATTRACT_CHASE_PELLET_BLINK_PERIOD: equ 8         ; ~3.75 Hz

; Chase-row sprite_y. Sprites are 16x16; the pellet is 8x8 at y=200;
; pellet centre = y=204, sprite centre = sprite_y+8 → sprite_y = 196.
CHASE_SPRITE_Y:            equ 196

CHASE_GAME_AREA_RIGHT:     equ 362           ; right edge of the 224-wide
                                             ; "game area"; sprites east of
                                             ; this stay hidden so they slide
                                             ; on smoothly one at a time
CHASE_RIGHT_BORDER:        equ 16            ; width of the conceptual black
                                             ; border to the right of the
                                             ; game area; sprites spawn beyond
                                             ; it (start_x = 224 + border) so
                                             ; they walk in instead of popping
                                             ; on at the edge
CHASE_PAC_START_X:         equ CHASE_GAME_AREA_RIGHT + CHASE_RIGHT_BORDER
CHASE_PAC_EAT_X:           equ 152           ; Pac aligns with pellet
CHASE_PAC_BLINKY_GAP:      equ 32            ; Pac→Blinky lead at spawn —
                                             ; bigger than the ghost↔ghost
                                             ; gap so Pac has a clear lead
                                             ; entering the screen
CHASE_GHOST_GAP:           equ 16            ; ghost↔ghost spacing; ghosts
                                             ; trail in a tight packed
                                             ; formation behind Pac

; Speeds are Q8.8 fixed-point (256 = 1.0 px/frame). Each frame the
; speed is added to the entity's 16-bit accumulator; the high byte of
; the result is the integer pixel delta applied this frame, the low
; byte is the leftover subpixel carried into the next tick. This is
; the same model the in-game ghost AI uses (ghost_tick uses an 8-bit
; accumulator + 8-bit speed, where 256 = 1 px/frame), just widened to
; 16-bit so we can express speeds above 1 px/frame.
;
; LEFT phase: ghosts run a hair faster than Pac so the gap closes
; gradually — arcade behaviour. Difference is 1/8 px/frame, which
; closes ~14 px over the ~113-frame LEFT phase. Initial Pac→Blinky
; gap (CHASE_GHOST_GAP = 16) shrinks to almost zero by the pellet
; eat, matching "JUST about to catch up".
CHASE_PAC_LEFT_SPEED:      equ 240          ; 2.000 px/frame
CHASE_GHOST_LEFT_SPEED:    equ 256          ; 2.125 px/frame
CHASE_PAC_RIGHT_SPEED:     equ 256          ; 2.000 px/frame
CHASE_GHOST_RIGHT_SPEED:   equ 162          ;1.000 px/frame (fleeing)
CHASE_EAT_REACH:           equ 8            ; smaller than CHASE_GHOST_GAP
                                             ; so Blinky isn't eaten the
                                             ; instant the right-chase
                                             ; starts; Pac visibly closes
                                             ; the gap each time

CHASE_PRE_FRAMES:          equ 60            ; 1 s pause before chase
CHASE_POPUP_FRAMES:        equ 30            ; 0.5 s per chain popup
CHASE_HOLD_1600_FRAMES:    equ 60            ; 1 s hold on the final popup

CHASE_PAC_ANIM_PERIOD:     equ 6
CHASE_GHOST_ANIM_PERIOD:   equ 8

CHASE_PHASE_PRE:           equ 0
CHASE_PHASE_LEFT:          equ 1
CHASE_PHASE_RIGHT:         equ 2
CHASE_PHASE_HOLD_1600:     equ 3
CHASE_PHASE_DONE:          equ 4

; --- State -----------------------------------------------------------------

attract_chase_pellet_blink_counter: db ATTRACT_CHASE_PELLET_BLINK_PERIOD
attract_chase_pellet_visible:       db 1
attract_chase_pellet_alive:         db 1     ; 0 once Pac has eaten it

chase_phase:           db 0
chase_phase_timer:     db 0

; Per-sprite x + "has appeared" flag, laid out so IX can index both via
; ix+0 (3-byte x) and ix+3 (1-byte shown). Once a sprite has appeared
; (x first crosses below CHASE_GAME_AREA_RIGHT), the flag is set and it
; stays visible until explicitly hidden (e.g. by @eat_ghost).
chase_pac_x:           dl 0
chase_pac_shown:       db 0
chase_blinky_x:        dl 0                  ; Shadow
chase_blinky_shown:    db 0
chase_pinky_x:         dl 0                  ; Speedy
chase_pinky_shown:     db 0
chase_inky_x:          dl 0                  ; Bashful
chase_inky_shown:      db 0
chase_clyde_x:         dl 0                  ; Pokey
chase_clyde_shown:     db 0
chase_ghosts_eaten:    db 0                  ; 0..4
chase_pac_anim:        db 0
chase_pac_anim_count:  db CHASE_PAC_ANIM_PERIOD
chase_ghost_anim:      db 0
chase_ghost_anim_count: db CHASE_GHOST_ANIM_PERIOD
chase_popup_visible:   db 0

; Q8.8 subpixel accumulators (16-bit logically). Allocated with `dl`
; (3 bytes) rather than `dw` because the reset code uses `ld (..), hl`
; which in ADL mode writes 3 bytes (L, H, HLU). If these were `dw`, the
; HLU byte would overflow past the variable's allocation and land on
; whatever follows in memory — for `chase_ghost_accum: dw` that next
; byte is the first instruction byte of `state_attract_hold_enter`,
; whose `3E` (start of `ld a, ATTRACT_CHASE_PELLET_BLINK_PERIOD`) gets
; clobbered to `00`. After one HOLD cycle, the function executes as
; `NOP; ex af, af'; ld (counter), a; ...` and the blink counter is
; seeded from a random A' instead of 8 — making the chase pellet stay
; static for an indeterminate stretch on cycle 2. See #54.
chase_pac_accum:       dl 0
chase_ghost_accum:     dl 0

; --- State entry -----------------------------------------------------------

state_attract_hold_enter:
    ld a, ATTRACT_CHASE_PELLET_BLINK_PERIOD
    ld (attract_chase_pellet_blink_counter), a
    ld a, 1
    ld (attract_chase_pellet_visible), a
    ld (attract_chase_pellet_alive), a

    xor a
    ld (chase_phase), a
    ld (chase_ghosts_eaten), a
    ld (chase_popup_visible), a
    ld (chase_pac_anim), a
    ld (chase_ghost_anim), a
    ld (chase_pac_shown), a
    ld (chase_blinky_shown), a
    ld (chase_pinky_shown), a
    ld (chase_inky_shown), a
    ld (chase_clyde_shown), a
    ld a, CHASE_PRE_FRAMES
    ld (chase_phase_timer), a
    ld a, CHASE_PAC_ANIM_PERIOD
    ld (chase_pac_anim_count), a
    ld a, CHASE_GHOST_ANIM_PERIOD
    ld (chase_ghost_anim_count), a

    ld hl, 0
    ld (chase_pac_accum), hl
    ld (chase_ghost_accum), hl

    ; Park Pac and the 4 ghosts off-screen right. Pac→Blinky uses the
    ; bigger PAC_BLINKY_GAP so Pac has a visible lead; the ghosts pack
    ; in tightly behind with the standard GHOST_GAP between each.
    ld hl, CHASE_PAC_START_X
    ld (chase_pac_x), hl
    ld de, CHASE_PAC_BLINKY_GAP
    add hl, de
    ld (chase_blinky_x), hl
    ld de, CHASE_GHOST_GAP
    add hl, de
    ld (chase_pinky_x), hl
    add hl, de
    ld (chase_inky_x), hl
    add hl, de
    ld (chase_clyde_x), hl
    ret

; --- State tick ------------------------------------------------------------

state_attract_hold_tick:
    call @blink_pellets
    call @chase_tick
    call @poll_input
    ret

; @blink_pellets — energiser blink for chase + 50pts legend pellet.
; Once Pac has eaten the chase pellet (alive=0), only the legend
; pellet keeps blinking.
@blink_pellets:
    ld hl, attract_chase_pellet_blink_counter
    dec (hl)
    jr nz, @blink_paint
    ld a, ATTRACT_CHASE_PELLET_BLINK_PERIOD
    ld (hl), a
    ld a, (attract_chase_pellet_visible)
    xor 1
    ld (attract_chase_pellet_visible), a
@blink_paint:
    ld a, (attract_chase_pellet_visible)
    or a
    jr z, @blink_paint_off
    ld hl, SPRITE_POWER_PELLET_00
    jr @blink_paint_now
@blink_paint_off:
    ld hl, SPRITE_ERASE_PELLET_00
@blink_paint_now:
    call vdu_buffer_select
    ld a, (attract_chase_pellet_alive)
    or a
    jr z, @blink_legend_only
    ld bc, ATTRACT_CHASE_PELLET_X
    ld de, ATTRACT_CHASE_PELLET_Y
    call vdu_bitmap_plot
@blink_legend_only:
    ld bc, ATTRACT_LEGEND_PELLET_X
    ld de, ATTRACT_LEGEND_PELLET_Y
    jp vdu_bitmap_plot

; @chase_tick — dispatch on chase_phase.
@chase_tick:
    ld a, (chase_phase)
    or a
    jp z, @phase_pre
    cp CHASE_PHASE_LEFT
    jp z, @phase_left
    cp CHASE_PHASE_RIGHT
    jp z, @phase_right
    cp CHASE_PHASE_HOLD_1600
    jp z, @phase_hold_1600
    ret                                       ; DONE

@phase_pre:
    ld hl, chase_phase_timer
    dec (hl)
    ret nz
    ; Sprites stay hidden — paint helpers reveal them naturally as each
    ; one's x crosses CHASE_GAME_AREA_RIGHT (224) moving left.
    ld a, CHASE_PHASE_LEFT
    ld (chase_phase), a
    ret

@phase_left:
    call @advance_pac_anim
    call @advance_ghost_anim
    ld ix, chase_pac_accum
    ld bc, CHASE_PAC_LEFT_SPEED
    call @advance_accum                       ; A = pixels this frame
    neg                                       ; LEFT phase moves negative
    call @move_pac_x
    ld ix, chase_ghost_accum
    ld bc, CHASE_GHOST_LEFT_SPEED
    call @advance_accum
    neg
    call @move_all_ghosts_x
    call @paint_pac_left
    call @paint_ghosts_left
    ; Pac reached the pellet?
    ld hl, (chase_pac_x)
    ld de, CHASE_PAC_EAT_X
    or a
    sbc hl, de
    ret nc                                    ; pac_x > eat_x → keep moving
    jp @on_pellet_eaten

@on_pellet_eaten:
    xor a
    ld (attract_chase_pellet_alive), a
    ld hl, SPRITE_ERASE_PELLET_00
    call vdu_buffer_select
    ld bc, ATTRACT_CHASE_PELLET_X
    ld de, ATTRACT_CHASE_PELLET_Y
    call vdu_bitmap_plot
    xor a
    ld (chase_ghosts_eaten), a
    ld hl, 0
    ld (chase_pac_accum), hl
    ld (chase_ghost_accum), hl
    ld a, CHASE_PHASE_RIGHT
    ld (chase_phase), a
    ret

@phase_right:
    call @advance_pac_anim
    ; While the chain popup is showing, freeze movement and count down
    ; the popup timer. On expiry, hide the popup and either continue or
    ; (if 4 ghosts eaten) move to HOLD_1600.
    ld a, (chase_popup_visible)
    or a
    jr z, @right_move
    ld hl, chase_phase_timer
    dec (hl)
    ret nz
    ld a, (chase_ghosts_eaten)
    cp 4
    jr z, @popup_to_hold_1600
    ; Ghost 1/2/3 popup expired — hide it and resume the chase.
    xor a
    ld (chase_popup_visible), a
    ld a, SPRITE_SCORE_POPUP_ID
    call vdu_sprite_select
    call vdu_sprite_hide
    jr @right_move
@popup_to_hold_1600:
    ; 4th popup (1600 pts): keep it on screen through HOLD_1600.
    ; player_select_enter will hide it during the state transition.
    ld a, CHASE_PHASE_HOLD_1600
    ld (chase_phase), a
    ld a, CHASE_HOLD_1600_FRAMES
    ld (chase_phase_timer), a
    ret

@right_move:
    ld ix, chase_pac_accum
    ld bc, CHASE_PAC_RIGHT_SPEED
    call @advance_accum                       ; A = +pixels this frame
    call @move_pac_x
    ld ix, chase_ghost_accum
    ld bc, CHASE_GHOST_RIGHT_SPEED
    call @advance_accum
    call @move_all_ghosts_x
    call @paint_pac_right
    call @paint_ghosts_blue
    ld a, (chase_ghosts_eaten)
    cp 4
    ret nc
    call @check_next_eat
    ret

@phase_hold_1600:
    ld hl, chase_phase_timer
    dec (hl)
    ret nz
    ld a, CHASE_PHASE_DONE
    ld (chase_phase), a
    ld a, GS_PLAYER_SELECT
    jp game_state_set

; --- Helpers: ghost x / id lookup ------------------------------------------

; @next_ghost_info — for the next ghost to be eaten (chase_ghosts_eaten),
; load HL = ghost.x and A = sprite id. Caller must check
; chase_ghosts_eaten < 4 before calling.
; Destroys: af, hl
@next_ghost_info:
    push bc
    ld a, (chase_ghosts_eaten)
    or a
    jr nz, @ng_1
    ld hl, (chase_blinky_x)
    ld a, SPRITE_BLINKY_ID
    jr @ng_done
@ng_1:
    cp 1
    jr nz, @ng_2
    ld hl, (chase_pinky_x)
    ld a, SPRITE_PINKY_ID
    jr @ng_done
@ng_2:
    cp 2
    jr nz, @ng_3
    ld hl, (chase_inky_x)
    ld a, SPRITE_INKY_ID
    jr @ng_done
@ng_3:
    ld hl, (chase_clyde_x)
    ld a, SPRITE_CLYDE_ID
@ng_done:
    pop bc
    ret

; @check_next_eat — eat the next ghost if Pac is within CHASE_EAT_REACH.
@check_next_eat:
    call @next_ghost_info                     ; HL = ghost.x, A = sprite id
    push af                                   ; stash sprite id (BC about to die)
    ld de, (chase_pac_x)
    ld bc, CHASE_EAT_REACH
    ex de, hl                                 ; HL = pac.x, DE = ghost.x
    add hl, bc                                ; HL = pac.x + reach
    ex de, hl                                 ; HL = ghost.x, DE = pac.x+reach
    or a
    sbc hl, de                                ; HL = ghost.x − (pac.x+reach)
    jr nc, @cne_no                            ; ghost still too far right
    pop af                                    ; restore sprite id
    jp @eat_ghost
@cne_no:
    pop af
    ret

; @eat_ghost — entered with A = sprite id of the ghost to eat. Hides
; the ghost, plants the chain popup at its x (we just consumed
; chase_ghosts_eaten via @next_ghost_info, so we look up again).
@eat_ghost:
    push af
    call vdu_sprite_select
    call vdu_sprite_hide
    pop af

    call @next_ghost_info                     ; HL = popup x
    ; Show score popup. Frame index = chase_ghosts_eaten (0=200..).
    push hl
    ld a, SPRITE_SCORE_POPUP_ID
    call vdu_sprite_select
    ld a, (chase_ghosts_eaten)
    call vdu_sprite_select_frame
    pop bc                                    ; BC = popup x
    ld de, CHASE_SPRITE_Y
    call vdu_sprite_move_abs
    call vdu_sprite_show

    ; Hide Pac while the score popup is shown — arcade-faithful: the
    ; popup sits where Pac caught the ghost, so Pac himself vanishes
    ; for the half-second display. @paint_pac_right will re-show him
    ; on the next chase frame once the popup expires.
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    call vdu_sprite_hide

    ld a, 1
    ld (chase_popup_visible), a
    ld a, CHASE_POPUP_FRAMES
    ld (chase_phase_timer), a
    ld a, (chase_ghosts_eaten)
    inc a
    ld (chase_ghosts_eaten), a
    ret

; --- Sprite painters -------------------------------------------------------
;
; Each paint helper does an x-position visibility check first:
;   * If sprite_x >= CHASE_GAME_AREA_RIGHT, hide the sprite — it's still
;     off-screen (behind the imaginary right edge "block").
;   * Otherwise, set the frame, move, and show.
; That makes sprites slide on naturally as their x walks down past 224,
; without any partial-pixel clipping (which the Agon sprite plane
; doesn't support — sprites always render as full 16×16 over the bitmap
; plane).

; @check_sprite_visible — should this sprite be painted this frame?
;   Inputs: IX = pointer to chase_*_x (3 bytes), with shown flag at ix+3
;   Outputs: A = shown flag value (post-update); zero = stay hidden
;            (caller should return without painting), non-zero = paint.
;            Updates the shown flag on first entry.
;   Destroys: af, de, hl
@check_sprite_visible:
    ld a, (ix + 3)
    or a
    ret nz                                    ; already shown — paint
    ; Not shown yet — has the sprite crossed into the visible area?
    ld hl, (ix + 0)
    ld de, CHASE_GAME_AREA_RIGHT
    or a
    sbc hl, de                                ; HL = x - 224
    jr nc, @csv_still_hidden
    ; x < 224: first appearance. Set shown=1 and tell the caller to paint.
    ld a, 1
    ld (ix + 3), a
    ret
@csv_still_hidden:
    xor a                                     ; return zero
    ret

@paint_pac_left:
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    ld ix, chase_pac_x
    call @check_sprite_visible
    or a
    ret z
    ld a, (chase_pac_anim)
    add a, 3                                  ; left-facing base
    call vdu_sprite_select_frame
    ld bc, (chase_pac_x)
    ld de, CHASE_SPRITE_Y
    call vdu_sprite_move_abs
    jp vdu_sprite_show

@paint_pac_right:
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    ld ix, chase_pac_x
    call @check_sprite_visible
    or a
    ret z
    ld a, (chase_pac_anim)                    ; right-facing base = 0
    call vdu_sprite_select_frame
    ld bc, (chase_pac_x)
    ld de, CHASE_SPRITE_Y
    call vdu_sprite_move_abs
    jp vdu_sprite_show

@paint_ghosts_left:
    ld ix, chase_blinky_x
    ld a, SPRITE_BLINKY_ID
    call @paint_ghost_left
    ld ix, chase_pinky_x
    ld a, SPRITE_PINKY_ID
    call @paint_ghost_left
    ld ix, chase_inky_x
    ld a, SPRITE_INKY_ID
    call @paint_ghost_left
    ld ix, chase_clyde_x
    ld a, SPRITE_CLYDE_ID
@paint_ghost_left:
    call vdu_sprite_select
    call @check_sprite_visible
    or a
    ret z
    ld a, (chase_ghost_anim)
    add a, 2                                  ; left-facing base
    call vdu_sprite_select_frame
    ld bc, (ix + 0)
    ld de, CHASE_SPRITE_Y
    call vdu_sprite_move_abs
    jp vdu_sprite_show

; @paint_ghosts_blue — during the right-chase, every still-alive ghost
; shows blue fright frames. Eaten ones (those at index <
; chase_ghosts_eaten) are already hidden; skip them.
@paint_ghosts_blue:
    ld a, (chase_ghosts_eaten)
    ld b, a
    or a
    jr nz, @blue_skip_b
    ld ix, chase_blinky_x
    ld a, SPRITE_BLINKY_ID
    call @paint_blue_one
@blue_skip_b:
    ld a, b
    cp 2
    jr nc, @blue_skip_p
    ld ix, chase_pinky_x
    ld a, SPRITE_PINKY_ID
    call @paint_blue_one
@blue_skip_p:
    ld a, b
    cp 3
    jr nc, @blue_skip_i
    ld ix, chase_inky_x
    ld a, SPRITE_INKY_ID
    call @paint_blue_one
@blue_skip_i:
    ld a, b
    cp 4
    ret nc
    ld ix, chase_clyde_x
    ld a, SPRITE_CLYDE_ID
@paint_blue_one:
    call vdu_sprite_select
    call @check_sprite_visible
    or a
    ret z
    ld a, (chase_ghost_anim)
    add a, 8                                  ; fright blue A/B
    call vdu_sprite_select_frame
    ld bc, (ix + 0)
    ld de, CHASE_SPRITE_Y
    call vdu_sprite_move_abs
    jp vdu_sprite_show

; --- Movement helpers ------------------------------------------------------

; @advance_accum — add Q8.8 speed (BC) to the 16-bit accumulator at
; (IX+0/+1), store the leftover subpixel back, and return the integer
; pixel delta for this frame in A. Speeds we use are well under
; 4 px/frame so A is always a small positive byte.
;   Inputs:  IX = pointer to dw accumulator, BC = speed (Q8.8)
;   Outputs: A = unsigned pixel delta this frame
;   Destroys: af, hl
@advance_accum:
    ld l, (ix + 0)
    ld h, (ix + 1)
    add hl, bc
    ld a, h                                   ; integer pixels = high byte
    ld h, 0                                   ; carry low byte forward only
    ld (ix + 0), l
    ld (ix + 1), h
    ret

; @move_pac_x — add signed A (8-bit) to chase_pac_x. Uses subtraction
; for negative deltas to avoid the ADL 24-bit sign-extension trap: just
; setting D = $FF doesn't touch DEU, so add hl,de would corrupt the
; upper byte if DEU happens to be non-zero on entry.
@move_pac_x:
    bit 7, a
    jr nz, @mpx_neg
    ld hl, (chase_pac_x)
    ld de, 0
    ld e, a
    add hl, de
    ld (chase_pac_x), hl
    ret
@mpx_neg:
    neg                                       ; A = |delta|
    ld hl, (chase_pac_x)
    ld de, 0
    ld e, a
    or a
    sbc hl, de
    ld (chase_pac_x), hl
    ret

; @move_all_ghosts_x — same delta applied to all four ghost x.
@move_all_ghosts_x:
    push af
    ld ix, chase_blinky_x
    call @move_ghost_x
    pop af
    push af
    ld ix, chase_pinky_x
    call @move_ghost_x
    pop af
    push af
    ld ix, chase_inky_x
    call @move_ghost_x
    pop af
    ld ix, chase_clyde_x
@move_ghost_x:
    bit 7, a
    jr nz, @mgx_neg
    ld hl, (ix + 0)
    ld de, 0
    ld e, a
    add hl, de
    ld (ix + 0), hl
    ret
@mgx_neg:
    neg
    ld hl, (ix + 0)
    ld de, 0
    ld e, a
    or a
    sbc hl, de
    ld (ix + 0), hl
    ret

; --- Animation helpers -----------------------------------------------------

@advance_pac_anim:
    ld hl, chase_pac_anim_count
    dec (hl)
    ret nz
    ld a, CHASE_PAC_ANIM_PERIOD
    ld (hl), a
    ld a, (chase_pac_anim)
    inc a
    cp 3
    jr c, @apa_store
    xor a
@apa_store:
    ld (chase_pac_anim), a
    ret

@advance_ghost_anim:
    ld hl, chase_ghost_anim_count
    dec (hl)
    ret nz
    ld a, CHASE_GHOST_ANIM_PERIOD
    ld (hl), a
    ld a, (chase_ghost_anim)
    xor 1
    ld (chase_ghost_anim), a
    ret

; --- Input -----------------------------------------------------------------

@poll_input:
    ld a, mos_getkbmap
    rst.lil $08
    ld a, (ix + $0A)
    bit 2, a
    jr z, @input_check_start
    call credit_deposit
    ld a, GS_PLAYER_SELECT
    jp game_state_set
@input_check_start:
    ld a, (credit)
    or a
    ret z
    ld a, (ix + $06)
    bit 0, a
    jp nz, player_start_one_player
    ld a, (credit)
    cp 2
    ret c
    ld a, (ix + $06)
    bit 1, a
    jp nz, player_start_two_player
    ret
