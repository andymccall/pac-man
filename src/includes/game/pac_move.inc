; Pac-Man movement: tile-grid decision logic with queued direction and
; arcade-style 3-frame mouth animation.
;
; State:
;   pac_dir         — direction Pac is actively moving (NONE if stopped)
;   pac_desired_dir — direction the player most recently pressed; Pac turns
;                     onto it as soon as the tile in that direction is open
;   sprite_x, sprite_y — Pac sprite top-left in screen pixels (24-bit)
;   pac_anim_frame  — current mouth state (0=closed, 1=half, 2=open)
;   pac_anim_counter — countdown until next mouth state
;
; Per-frame flow (called from state_play_tick after mos_getkbmap):
;   pac_input_sample → update pac_desired_dir from held keys
;   pac_try_turn     → if tile in pac_desired_dir is walkable, set pac_dir
;   pac_step         → advance 1 pixel in pac_dir; on success advance the
;                      animation, on block snap to closed mouth
;
; Sprite frame layout (from vdu_game_data):
;   0..2 right  (closed, half, open)
;   3..5 left
;   6..8 up
;   9..11 down

DIR_NONE:   equ 0
DIR_UP:     equ 1
DIR_DOWN:   equ 2
DIR_LEFT:   equ 3
DIR_RIGHT:  equ 4

pac_anim_period:   equ 4         ; game frames per mouth state

pac_dir:           db DIR_NONE
pac_desired_dir:   db DIR_NONE
pac_anim_frame:    db 0
pac_anim_counter:  db pac_anim_period

; 24-bit storage so ADL-mode loads are clean; values logically 16-bit.
sprite_x:   dl 250
sprite_y:   dl 248

; Horizontal tunnel: row 14 of the wall map is the open corridor.
pac_tunnel_row:        equ 14
pac_tunnel_center_min: equ 180   ; maze_play_origin_y + pac_tunnel_row * maze_play_tile
pac_tunnel_center_max: equ 188

pac_right_edge_sprite_x: equ 358  ; maze_play_origin_x + 27*8 - 8
pac_left_edge_sprite_x:  equ 138  ; maze_play_origin_x - 8

; pac_reset — clear movement + animation state, show closed-mouth right.
; Called by state_ready_enter so Pac starts each life stationary.
pac_reset:
    xor a
    ld (pac_dir), a
    ld (pac_desired_dir), a
    ld (pac_anim_frame), a
    ld a, pac_anim_period
    ld (pac_anim_counter), a
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    ld a, 0                  ; closed-mouth right (looks like a yellow disc)
    jp vdu_sprite_select_frame

; pac_on_tunnel_row — Z=1 if sprite_y in DE places Pac's centre on the
; tunnel row.
; Destroys: af, hl
pac_on_tunnel_row:
    ld hl, 8
    add hl, de
    ld de, pac_tunnel_center_min
    or a
    sbc hl, de
    jr c, @not_row
    ld a, h
    or a
    jr nz, @not_row
    ld a, l
    cp pac_tunnel_center_max - pac_tunnel_center_min
    jr nc, @not_row
    cp a
    ret
@not_row:
    or 1
    ret

; pac_try_move — BC = candidate sprite_x, DE = candidate sprite_y.
; On success: commits, moves the sprite, eats any pellet, returns Z=1.
; On blocked: leaves state unchanged, returns Z=0.
; Destroys: af, bc, de, hl
pac_try_move:
    push bc
    push de
    call tile_is_wall
    pop de
    pop bc
    jr z, @ok
    or 1                          ; Z=0 → blocked
    ret
@ok:
    ld (sprite_x), bc
    ld (sprite_y), de
    ; Defensive re-select: pac_select_frame_for_dir already selected Pac
    ; earlier in pac_step, but something between here and there is
    ; apparently leaving Clyde (sprite 4) selected when the move command
    ; fires — symptom: Clyde teleports onto Pac's position as Pac moves.
    ; Force-select Pac immediately before the move to make this airtight.
    push bc
    push de
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    pop de
    pop bc
    push bc
    push de
    call vdu_sprite_move_abs
    pop de
    pop bc
    call pellet_eat
    xor a                         ; Z=1 → moved
    ret

; pac_input_sample — IX must already point at the keymap (caller did
; mos_getkbmap). Updates pac_desired_dir if any direction key is held.
; Order S/W/D/A means A wins ties (priority: left > right > up > down).
; Destroys: af
pac_input_sample:
    ld a, (ix + $0A)         ; S row
    bit 1, a
    jr z, @no_s
    ld a, DIR_DOWN
    ld (pac_desired_dir), a
@no_s:
    ld a, (ix + $04)         ; W row
    bit 1, a
    jr z, @no_w
    ld a, DIR_UP
    ld (pac_desired_dir), a
@no_w:
    ld a, (ix + $06)         ; D row
    bit 2, a
    jr z, @no_d
    ld a, DIR_RIGHT
    ld (pac_desired_dir), a
@no_d:
    ld a, (ix + $08)         ; A row
    bit 1, a
    jr z, @no_a
    ld a, DIR_LEFT
    ld (pac_desired_dir), a
@no_a:
    ret

; pac_try_turn — if the tile one step in pac_desired_dir from Pac's
; current tile is walkable, set pac_dir <- pac_desired_dir. No alignment
; check: arcade Pac-Man allows turning from any sub-tile position; the
; cornering routine (pac_nudge) then drifts the perpendicular axis back
; to the tile-centre line over the next few frames. Mirrors the arcade
; ROM behaviour at #1985.
;
; Destroys: af, bc, de, hl
pac_try_turn:
    ld a, (pac_desired_dir)
    or a
    ret z

    ; Same direction as current? Nothing to do.
    ld b, a
    ld a, (pac_dir)
    cp b
    ret z

    ld bc, (sprite_x)
    ld de, (sprite_y)
    call sprite_to_tile          ; out: c=col, b=row, carry=off-grid
    ret c
    ; b = row, c = col

    ld a, (pac_desired_dir)
    cp DIR_UP
    jr nz, @nu
    ld a, b
    or a
    ret z                        ; row 0 — can't go up
    dec b
    jr @lookup
@nu:
    cp DIR_DOWN
    jr nz, @nd
    ld a, b
    cp maze_play_rows - 1
    ret nc                       ; bottom row — can't go down
    inc b
    jr @lookup
@nd:
    cp DIR_LEFT
    jr nz, @nl
    ld a, c
    or a
    ret z                        ; col 0 — tunnel handled by pac_step
    dec c
    jr @lookup
@nl:
    ; DIR_RIGHT
    ld a, c
    cp maze_play_cols - 1
    ret nc                       ; col 27 — tunnel handled by pac_step
    inc c

@lookup:
    call tile_row_col_to_offset
    ld de, maze_wall_map
    add hl, de
    ld a, (hl)
    cp maze_wall_floor
    ret nz                       ; target tile is a wall — no turn
    ld a, (pac_desired_dir)
    ld (pac_dir), a
    ret

; pac_select_frame_for_dir — sets Pac's sprite frame from (pac_dir,
; pac_anim_frame). Frame base per direction: right=0, left=3, up=6, down=9;
; anim offset = pac_anim_frame (0=closed, 1=half, 2=open).
; Destroys: af
pac_select_frame_for_dir:
    push bc
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    ld a, (pac_anim_frame)
    ld b, a                       ; b = anim offset
    ld a, (pac_dir)
    cp DIR_RIGHT
    jr nz, @nr
    ld a, b
    pop bc
    jp vdu_sprite_select_frame
@nr:
    cp DIR_LEFT
    jr nz, @nl
    ld a, b
    add a, 3
    pop bc
    jp vdu_sprite_select_frame
@nl:
    cp DIR_UP
    jr nz, @nu
    ld a, b
    add a, 6
    pop bc
    jp vdu_sprite_select_frame
@nu:
    ; DIR_DOWN (or NONE — treat as default right closed)
    cp DIR_DOWN
    jr nz, @none
    ld a, b
    add a, 9
    pop bc
    jp vdu_sprite_select_frame
@none:
    ld a, 0
    pop bc
    jp vdu_sprite_select_frame

; pac_anim_advance — countdown pac_anim_counter; on expiry, cycle
; pac_anim_frame 0→1→2→0.
; Destroys: af, hl
pac_anim_advance:
    ld hl, pac_anim_counter
    dec (hl)
    ret nz
    ld (hl), pac_anim_period
    ld a, (pac_anim_frame)
    inc a
    cp 3
    jr c, @save
    xor a
@save:
    ld (pac_anim_frame), a
    ret

; pac_anim_reset_closed — snap mouth to closed (frame 0) and reload the
; counter. Called when blocked by a wall.
; Destroys: af
pac_anim_reset_closed:
    xor a
    ld (pac_anim_frame), a
    ld a, pac_anim_period
    ld (pac_anim_counter), a
    ret

; pac_at_axis_centre — Z=1 if Pac's pixel position is exactly on the tile
; centre line for his current motion axis. For horizontal motion (LEFT/RIGHT)
; that means sprite_x mod 8 == 6 (col centre, since 142 + col*8 mod 8 == 6).
; For vertical motion (UP/DOWN) that means sprite_y mod 8 == 0 (row centre,
; since 64 + row*8 mod 8 == 0).
; Destroys: af
pac_at_axis_centre:
    ld a, (pac_dir)
    cp DIR_LEFT
    jr c, @vertical
    ld a, (sprite_x)
    and 7
    cp 6
    ret
@vertical:
    ld a, (sprite_y)
    and 7
    ret

; pac_can_advance — Z=1 if the tile one step in pac_dir from Pac's current
; tile is walkable. Treats off-grid LEFT/RIGHT on the tunnel row as
; "walkable" so the wrap can fire when Pac walks off the edge.
; Destroys: af, bc, de, hl
pac_can_advance:
    ld bc, (sprite_x)
    ld de, (sprite_y)
    call sprite_to_tile          ; b=row, c=col, carry=off-grid
    jr c, @blocked

    ld a, (pac_dir)
    cp DIR_UP
    jr nz, @nu
    ld a, b
    or a
    jr z, @blocked
    dec b
    jr @lookup
@nu:
    cp DIR_DOWN
    jr nz, @nd
    ld a, b
    cp maze_play_rows - 1
    jr nc, @blocked
    inc b
    jr @lookup
@nd:
    cp DIR_LEFT
    jr nz, @nl
    ld a, c
    or a
    jr z, @tunnel
    dec c
    jr @lookup
@nl:
    ld a, c
    cp maze_play_cols - 1
    jr nc, @tunnel
    inc c

@lookup:
    call tile_row_col_to_offset
    ld de, maze_wall_map
    add hl, de
    ld a, (hl)
    cp maze_wall_floor
    ret

@tunnel:
    ld de, (sprite_y)
    jp pac_on_tunnel_row         ; tail call: Z=1 iff on the tunnel row

@blocked:
    or 1
    ret

; pac_nudge — push the perpendicular axis one pixel toward the centre
; line of Pac's current tile. Called once per frame at the tail of
; pac_step. Mirrors the arcade auto-centre routine at #195e in the ROM.
;
; Horizontal motion (LEFT/RIGHT): perpendicular axis is Y → target
; sprite_y = row*8 + 64. Vertical motion: perpendicular axis is X →
; target sprite_x = col*8 + 142. Move 1 pixel toward target per frame,
; or zero if already on the line. Combined with the alignment-free turn,
; this lets Pac pre-turn slightly into a corner and then glide into the
; new lane's centre over a few frames.
;
; Destroys: af, bc, de, hl
pac_nudge:
    ld a, (pac_dir)
    or a
    ret z

    cp DIR_LEFT
    jp c, @nudge_perp_x

    ; Horizontal motion — nudge sprite_y toward current row's centre.
    ld bc, (sprite_x)
    ld de, (sprite_y)
    call sprite_to_tile          ; b=row, c=col
    ret c

    ; target_y = row*8 + 64
    ld h, 0
    ld l, b
    add hl, hl
    add hl, hl
    add hl, hl
    ld de, 64
    add hl, de

    ld de, (sprite_y)
    or a
    sbc hl, de
    ret z                        ; already on row centre
    jr c, @y_dec

    ld hl, (sprite_y)
    inc hl
    ld (sprite_y), hl
    jp @commit
@y_dec:
    ld hl, (sprite_y)
    dec hl
    ld (sprite_y), hl
    jp @commit

@nudge_perp_x:
    ; Vertical motion — nudge sprite_x toward current col's centre.
    ld bc, (sprite_x)
    ld de, (sprite_y)
    call sprite_to_tile
    ret c

    ; target_x = col*8 + 142
    ld h, 0
    ld l, c
    add hl, hl
    add hl, hl
    add hl, hl
    ld de, 142
    add hl, de

    ld de, (sprite_x)
    or a
    sbc hl, de
    ret z                        ; already on col centre
    jr c, @x_dec

    ld hl, (sprite_x)
    inc hl
    ld (sprite_x), hl
    jp @commit
@x_dec:
    ld hl, (sprite_x)
    dec hl
    ld (sprite_x), hl

@commit:
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    ld bc, (sprite_x)
    ld de, (sprite_y)
    jp vdu_sprite_move_abs

; pac_step — advance Pac one pixel in pac_dir, walls block, tunnel wraps.
;
; Arcade-model wall handling: at the tile centre on Pac's motion axis we
; check whether the next tile in pac_dir is walkable. If not, Pac stays
; on that tile centre — never overshoots into the wall. Between centres
; he's locked into the corridor and just steps one pixel.
;
; After any step (or a no-op stop) pac_nudge runs to drift the
; perpendicular axis toward its tile-centre line — the arcade's cornering
; behaviour.
;
; Drives the mouth animation: cycle on successful step, reset to closed
; when blocked. No-op if pac_dir is NONE.
; Destroys: af, bc, de, hl
pac_step:
    ld a, (pac_dir)
    or a
    ret z

    ; Decision-point check
    call pac_at_axis_centre
    jr nz, @do_step
    call pac_can_advance
    jr z, @do_step

    ; Blocked at tile centre — stay put, keep facing direction, close mouth.
    call pac_anim_reset_closed
    call pac_select_frame_for_dir
    jp pac_nudge

@do_step:
    call pac_select_frame_for_dir
    ld a, (pac_dir)
    ld bc, (sprite_x)
    ld de, (sprite_y)

    cp DIR_UP
    jp z, @do_up
    cp DIR_DOWN
    jp z, @do_down
    cp DIR_LEFT
    jp z, @do_left
    ; DIR_RIGHT — handle tunnel-wrap before the step
    ld hl, pac_right_edge_sprite_x
    or a
    sbc hl, bc
    jr nz, @inc
    push bc
    push de
    call pac_on_tunnel_row
    pop de
    pop bc
    jr nz, @inc
    ld bc, pac_left_edge_sprite_x
    ld (sprite_x), bc
    push bc
    push de
    call vdu_sprite_move_abs
    pop de
    pop bc
    jr @moved
@inc:
    inc bc
    call pac_try_move
    jr nz, @blocked
    jr @moved

@do_left:
    ld hl, pac_left_edge_sprite_x
    or a
    sbc hl, bc
    jr nz, @dec
    push bc
    push de
    call pac_on_tunnel_row
    pop de
    pop bc
    jr nz, @dec
    ld bc, pac_right_edge_sprite_x
    ld (sprite_x), bc
    push bc
    push de
    call vdu_sprite_move_abs
    pop de
    pop bc
    jr @moved
@dec:
    dec bc
    call pac_try_move
    jr nz, @blocked
    jr @moved

@do_up:
    dec de
    call pac_try_move
    jr nz, @blocked
    jr @moved

@do_down:
    inc de
    call pac_try_move
    jr nz, @blocked
    ; fall through to @moved

@moved:
    call pac_anim_advance
    call pac_select_frame_for_dir
    jp pac_nudge

@blocked:
    call pac_anim_reset_closed
    call pac_select_frame_for_dir
    jp pac_nudge
