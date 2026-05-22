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

; On-eat score popup (#63). Held for ~2 s @ 60 fps, then auto-hidden
; by score_popup_tick. Longer than the ghost-eat popup (60 frames) —
; the fruit popup doesn't pause gameplay so it needs the longer
; window to be legible.
FRUIT_SCORE_POPUP_FRAMES: equ 120

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
    call vdu_sprite_hide

    ; Score popup (#63). Show the points value at the fruit's
    ; position; score_popup_tick (called from state_play_tick via
    ; ghost_ai_tick) hides it on expiry. Cherry → "100" → frame 6
    ; (200/400/800/1600 = frames 0-3, 10/50/100/... = 4-10). When
    ; #62 lands the frame index will be looked up per-level alongside
    ; the points value.
    ld a, SPRITE_SCORE_POPUP_ID
    call vdu_sprite_select
    ld a, 6                             ; frame 6 = SPRITE_SCORE_100
    call vdu_sprite_select_frame
    ld bc, FRUIT_X
    ld de, FRUIT_Y
    call vdu_sprite_move_abs
    call vdu_sprite_show
    ld a, FRUIT_SCORE_POPUP_FRAMES
    ld (score_popup_timer), a
    ret

; --- Fruit history HUD (#60) -----------------------------------------------
;
; Arcade-style indicator strip along the bottom-right of the maze. Shows
; the bonus fruit for each of the player's recent levels — up to seven
; icons, with the current level's fruit anchored on the right and older
; ones extending to the left. On level 1 only the cherry is visible;
; level 2 adds a strawberry to the right of the cherry; and so on.
;
; Re-blitted by state_ready_enter's @draw_static (same hook lives_display
; uses) so it survives the screen clear that runs at the start of every
; ready / death sequence.

FRUIT_HISTORY_MAX:    equ 7
; Left edge of the rightmost icon in absolute screen coords. The maze
; right edge is at sprite_x = 362 (origin_left 138 + 28 cols × 8 = 362),
; and lives_display_x = 162 sits one icon width in from the maze left
; edge (146). Mirror that on the right: rightmost icon's right edge =
; 362 − 16 = 346, so its left edge = 330.
; (Hard-coded rather than computed from SPRITE_FRUIT_WIDTH because this
; file is included before `sprites/fruit/fruit.inc`; ez80asm resolves
; `equ` expressions in pass 1 and won't accept a forward reference.)
fruit_history_x_right: equ 330
fruit_history_y:       equ 320            ; matches lives_display_y

fruit_history_x_scratch: dl 0             ; running x while looping right→left

; fruit_history_display — draw the per-level fruit history strip.
;   * Walks levels (current_level) → (current_level − count + 1).
;   * `count` is min(current_level, FRUIT_HISTORY_MAX).
;   * Each fruit's buffer is looked up via fruit_for_level (B-major
;     register conv: A = level → HL = buffer id).
; Destroys: af, bc, de, hl
fruit_history_display:
    ld a, (current_level)
    or a
    ret z                                ; pre-game; nothing to draw

    ; Cap count at FRUIT_HISTORY_MAX. We need to keep `current_level` as
    ; the level used for the rightmost icon, so save it before clamping.
    ld c, a                              ; c = current_level (cursor)
    cp FRUIT_HISTORY_MAX + 1
    jr c, @count_ok
    ld a, FRUIT_HISTORY_MAX
@count_ok:
    ld b, a                              ; b = loop count

    ; Plot in screen-absolute coords — see lives_display for the same
    ; reasoning. (lives_display already sets the origin to (0,0), but
    ; fruit_history_display might be called standalone in the future.)
    push bc
    ld bc, 0
    ld de, 0
    call vdu_set_gfx_origin
    pop bc

    ld hl, fruit_history_x_right
    ld (fruit_history_x_scratch), hl

@loop:
    push bc                              ; save count + level

    ld a, c
    call fruit_for_level                 ; HL = bitmap buffer id for level
    call vdu_buffer_select

    ld bc, (fruit_history_x_scratch)
    ld de, fruit_history_y
    call vdu_bitmap_plot

    ; advance x leftward by one icon width
    ld hl, (fruit_history_x_scratch)
    ld de, SPRITE_FRUIT_WIDTH
    or a
    sbc hl, de
    ld (fruit_history_x_scratch), hl

    pop bc                               ; restore count + level
    dec c                                ; next iteration shows previous level
    djnz @loop

    ; Restore gfx origin so subsequent gameplay bitmap plots land
    ; at the right place. See lives_display for the same reasoning.
    ld bc, origin_left
    ld de, origin_top
    jp vdu_set_gfx_origin

; fruit_for_level — A = level (1..); HL = bitmap buffer id of that level's
; bonus fruit, per the arcade table:
;   1            cherry
;   2            strawberry (berry)
;   3            orange
;   4-5          apple
;   6-7          melon
;   8-9          galaxian
;   10-11        bell
;   12+          key
; Destroys: af, hl
fruit_for_level:
    cp 2
    jr nc, @not_cherry
    ld hl, SPRITE_CHERRY_0
    ret
@not_cherry:
    cp 3
    jr nc, @not_berry
    ld hl, SPRITE_BERRY_0
    ret
@not_berry:
    cp 4
    jr nc, @not_orange
    ld hl, SPRITE_ORANGE_0
    ret
@not_orange:
    cp 6
    jr nc, @not_apple
    ld hl, SPRITE_APPLE_0
    ret
@not_apple:
    cp 8
    jr nc, @not_melon
    ld hl, SPRITE_MELON_0
    ret
@not_melon:
    cp 10
    jr nc, @not_galaxian
    ld hl, SPRITE_GALAXIAN_0
    ret
@not_galaxian:
    cp 12
    jr nc, @key
    ld hl, SPRITE_BELL_0
    ret
@key:
    ld hl, SPRITE_KEY_0
    ret
