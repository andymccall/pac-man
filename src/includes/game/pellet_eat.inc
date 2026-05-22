; Pellet eating.
;
; After a successful Pac-Man move, check the pellet map at the tile under the
; sprite's centre. If it's a pellet (1) or power pellet (2), zero the cell,
; redraw that tile as the null pellet, and award points.
;
; Point values follow the original arcade game.

pellet_points_value:        equ 10
power_pellet_points_value:  equ 50

; Post-eat stall lengths. Arcade Pac freezes for these many frames
; after eating, letting ghosts close the gap. Power pellets stall
; longer — the player gets a beat to react before fright kicks in.
pellet_stall_frames:        equ 1
power_pellet_stall_frames:  equ 3

; player_1_score: 24-bit binary score, little-endian (low byte first).
player_1_score:  dl 0
player_2_score:  dl 0

pellet_eat_value: db 0

; ---------------------------------------------------------------------------
; Power-pellet blink. The four energizers blink ~3 Hz throughout play —
; each call to power_pellets_blink_tick decrements the counter and, on
; underflow, toggles the visible flag and re-plots all four positions.
; Eaten power pellets are skipped (cell value != 2 in the pellet map), so
; once consumed they stay gone.
; ---------------------------------------------------------------------------

POWER_PELLET_BLINK_PERIOD:  equ 8         ; ~0.13 s/phase, ~3.75 Hz cycle (matches arcade YouTube)
power_pellet_blink_counter: db POWER_PELLET_BLINK_PERIOD
power_pellet_visible:       db 1          ; 1 = bitmap drawn, 0 = erased

power_pellets_blink_tick:
    ld hl, power_pellet_blink_counter
    dec (hl)
    ret nz

    ld a, POWER_PELLET_BLINK_PERIOD
    ld (hl), a
    ld a, (power_pellet_visible)
    xor 1
    ld (power_pellet_visible), a
    ; fall through to power_pellets_redraw

power_pellets_redraw:
    ld b, 3
    ld c, 1
    call power_pellet_redraw_one
    ld b, 3
    ld c, 26
    call power_pellet_redraw_one
    ld b, 23
    ld c, 1
    call power_pellet_redraw_one
    ld b, 23
    ld c, 26
    jp power_pellet_redraw_one

; power_pellet_redraw_one — plot either the energizer bitmap or the
; erase bitmap at (B=row, C=col), depending on power_pellet_visible.
; Skips the plot entirely if the map cell is no longer a power pellet
; (Pac has eaten it).
; Destroys: af, bc, de, hl
power_pellet_redraw_one:
    push bc
    call tile_row_col_to_offset
    ld de, maze_pellets_map
    add hl, de
    ld a, (hl)
    pop bc
    cp 2
    ret nz                        ; not a power pellet — skip

    ; relative x = 8 + col*8
    ld hl, 0
    ld l, c
    add hl, hl
    add hl, hl
    add hl, hl
    ld de, 8
    add hl, de
    push hl                       ; stash x

    ; relative y = 8 + row*8
    ld hl, 0
    ld l, b
    add hl, hl
    add hl, hl
    add hl, hl
    ld de, 8
    add hl, de
    push hl                       ; stash y

    ld a, (power_pellet_visible)
    or a
    jr z, @erase
    ld hl, SPRITE_POWER_PELLET_00
    jr @plot
@erase:
    ld hl, SPRITE_ERASE_PELLET_00
@plot:
    call vdu_buffer_select
    pop de                        ; relative y
    pop bc                        ; relative x
    jp vdu_bitmap_plot

; pellet_eat
; Inputs:  bc = Pac-Man sprite_x, de = Pac-Man sprite_y (post-move position)
; Destroys: af, bc, de, hl
pellet_eat:
    call sprite_to_tile
    ret c                             ; off-grid -> nothing to do
    push bc                           ; save (b=row, c=col)
    call tile_row_col_to_offset       ; hl = row*28 + col
    pop bc                            ; bc = (row, col) again
    ld de, maze_pellets_map
    add hl, de
    ld a, (hl)
    or a
    ret z                             ; nothing here
    ld (pellet_eat_value), a
    ld (hl), 0                        ; clear the map cell

    ; Bump the level pellet counter — drives ghost-house release timing
    ; and fruit-spawning thresholds (70, 170).
    ld a, (pellets_eaten_level)
    inc a
    ld (pellets_eaten_level), a
    ; fruit_check_threshold may fall through to fruit_activate at pellets
    ; 70 and 170, which calls vdu_sprite_select (clobbers BC). The next
    ; few lines use C as the pellet's column for the erase-bitmap plot,
    ; so save BC across the call. (Symptom otherwise: the 70th and 170th
    ; pellets vanish from the map but the bitmap stays on screen.)
    push bc
    call fruit_check_threshold
    pop bc

    ; Compute screen-relative redraw position.
    ; Origin during game loop is (origin_left, origin_top) = (138, 60).
    ; The maze was drawn with a (+8, +8) extra offset, so cell (col, row) sits
    ; at (8 + col*8, 8 + row*8) in origin-relative coordinates.
    ld h, 0
    ld l, c
    add hl, hl
    add hl, hl
    add hl, hl                        ; col*8
    ld de, 8
    add hl, de                        ; hl = 8 + col*8 (relative x)
    push hl                           ; stash x
    ld h, 0
    ld l, b
    add hl, hl
    add hl, hl
    add hl, hl                        ; row*8
    ld de, 8
    add hl, de                        ; hl = 8 + row*8 (relative y)
    push hl                           ; stash y

    ld hl, SPRITE_ERASE_PELLET_00     ; opaque black tile (null_pellet is fully transparent)
    call vdu_buffer_select
    pop de                            ; de = relative y
    pop bc                            ; bc = relative x
    call vdu_bitmap_plot

    ld a, (pellet_eat_value)
    cp 2
    jr z, @power
    ld hl, pellet_points_value
    ld a, pellet_stall_frames
    ld (pac_stall_frames), a
    jr @add_score
@power:
    call fright_activate
    ld hl, power_pellet_points_value
    ld a, power_pellet_stall_frames
    ld (pac_stall_frames), a
@add_score:
    ld de, (player_1_score)
    add hl, de
    ld (player_1_score), hl
    ret

; score_print
; Print a 24-bit score as 6 zero-padded digits using MOS character output.
; Inputs: hl = score value
; Destroys: af, bc, de, hl
score_print:
    ld bc, 100000
    call score_emit_digit
    ld bc, 10000
    call score_emit_digit
    ld bc, 1000
    call score_emit_digit
    ld bc, 100
    call score_emit_digit
    ld bc, 10
    call score_emit_digit
    ; last digit: hl itself, 0..9
    ld a, l
    add a, '0'
    rst.lil $10
    ret

; score_emit_digit
; Subtract bc from hl repeatedly; print the quotient as a digit; leave the
; remainder in hl. Destroys af, d.
score_emit_digit:
    ld d, 0
@loop:
    push hl
    or a
    sbc hl, bc
    jr c, @done
    pop af                            ; discard saved hl (keep the subtracted one)
    inc d
    jr @loop
@done:
    pop hl                            ; restore hl (last sbc underflowed)
    ld a, d
    add a, '0'
    rst.lil $10
    ret

; display_player_1_score
; Show the score under the 1UP label. Caller should set text colour beforehand.
player_1_score_pos:
    .db     31, 20, 4, 0              ; VDU 31, col, row; NUL to make this a C string

display_player_1_score:
    ld hl, player_1_score_pos
    call vdu_text_print
    ld hl, (player_1_score)
    call score_print
    ret
