; Fruit (bonus prize) spawning.
;
; The arcade spawns a fruit at fixed maze position twice per level — once
; when 70 pellets have been eaten this level and again at 170. Each
; appearance lasts ~9 seconds, or until Pac walks over it. The fruit type
; and points are level-dependent (Cherry 100 / Strawberry 300 / Orange
; 500 / Apple 700 / Melon 1000 / Galaxian 2000 / Bell 3000 / Key 5000).
; Level progression isn't wired up yet, so v1 always shows a cherry.
;
; Position: (250, 200) — half-column between cols 13/14, row 17, the open
; corridor just below the pen. Same column as the door, two rows below
; the pen floor.

FRUIT_FIRST_THRESHOLD:  equ 70
FRUIT_SECOND_THRESHOLD: equ 170
FRUIT_DURATION:         equ 540        ; ~9 s @ 60 fps
FRUIT_X:                equ 250
FRUIT_Y:                equ 200
FRUIT_TILE_ROW:         equ 17         ; sprite_y=200 → row 17
FRUIT_TILE_COL_A:       equ 13         ; sprite spans cols 13-14
FRUIT_TILE_COL_B:       equ 14

CHERRY_POINTS:          equ 100

; Per-level appearance bookkeeping. fruit_timer counts the visible
; duration down to 0; fruit_shown_count tracks how many appearances
; we've triggered this level (0, 1, or 2) so the same threshold doesn't
; re-trigger.
fruit_timer:       dl 0
fruit_shown_count: db 0
fruit_active:      db 0                ; 1 while the sprite is on screen

; fruit_init — full reset for a new game / new level. Clears the
; per-level appearance counter so the 70- and 170-pellet thresholds can
; trigger fresh. Hides the sprite.
; Destroys: af
fruit_init:
    xor a
    ld (fruit_shown_count), a
    ; fall through to fruit_hide_now

; fruit_hide_now — clear the active flag and timer and hide the sprite,
; without touching shown_count. Called from state_ready_enter so that
; dying mid-fruit doesn't leave the cherry on screen (but if Pac was
; between the 70- and 170-pellet appearances, the 170 trigger still
; counts as appearance #2 after respawn).
; Destroys: af
fruit_hide_now:
    xor a
    ld (fruit_active), a
    ld hl, 0
    ld (fruit_timer), hl
    ld a, SPRITE_CHERRY_0_ID
    call vdu_sprite_select
    jp vdu_sprite_hide

; fruit_check_threshold — called from pellet_eat after the pellet counter
; has incremented. If the new count crosses 70 or 170 (and we haven't
; already shown that appearance), activate the fruit.
; Destroys: af
fruit_check_threshold:
    ld a, (pellets_eaten_level)
    cp FRUIT_FIRST_THRESHOLD
    jr z, @first
    cp FRUIT_SECOND_THRESHOLD
    ret nz                              ; not a trigger pellet

    ; Second appearance only if we've already done the first.
    ld a, (fruit_shown_count)
    cp 1
    ret nz                              ; either 0 (skipped first?) or 2 (done)
    jr fruit_activate

@first:
    ld a, (fruit_shown_count)
    or a
    ret nz                              ; already shown — ignore

fruit_activate:
    ld a, 1
    ld (fruit_active), a
    ld a, (fruit_shown_count)
    inc a
    ld (fruit_shown_count), a
    ld hl, FRUIT_DURATION
    ld (fruit_timer), hl
    ld a, SPRITE_CHERRY_0_ID
    call vdu_sprite_select
    ld bc, FRUIT_X
    ld de, FRUIT_Y
    call vdu_sprite_move_abs
    jp vdu_sprite_show

; fruit_tick — decrement timer; on expiry hide the fruit. Called once
; per frame from state_play_tick.
; Destroys: af, hl, de
fruit_tick:
    ld a, (fruit_active)
    or a
    ret z
    ld hl, (fruit_timer)
    dec hl
    ld (fruit_timer), hl
    ld de, 0
    or a
    sbc hl, de
    ret nz
    ; Timer expired — hide the sprite.
    xor a
    ld (fruit_active), a
    ld a, SPRITE_CHERRY_0_ID
    call vdu_sprite_select
    jp vdu_sprite_hide

; fruit_collision_check — called once per frame from state_play_tick.
; If the fruit is up and Pac's centre tile is on the fruit's tile,
; award points and hide.
; Destroys: af, bc, de, hl
fruit_collision_check:
    ld a, (fruit_active)
    or a
    ret z

    ld bc, (sprite_x)
    ld de, (sprite_y)
    call sprite_to_tile
    ret c                               ; Pac off-grid — no eat

    ; Match Pac's row to FRUIT_TILE_ROW.
    ld a, b
    cp FRUIT_TILE_ROW
    ret nz

    ; Match Pac's col to either of the fruit's two tile columns.
    ld a, c
    cp FRUIT_TILE_COL_A
    jr z, @eat
    cp FRUIT_TILE_COL_B
    ret nz

@eat:
    ; Award points.
    ld hl, CHERRY_POINTS
    ld de, (player_1_score)
    add hl, de
    ld (player_1_score), hl

    ; Hide the fruit.
    xor a
    ld (fruit_active), a
    ld hl, 0
    ld (fruit_timer), hl
    ld a, SPRITE_CHERRY_0_ID
    call vdu_sprite_select
    jp vdu_sprite_hide
