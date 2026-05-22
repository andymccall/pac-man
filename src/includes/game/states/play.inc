; GS_PLAY — main gameplay.
;
; Movement is driven by pac_move.inc (input_sample → try_turn → step).
; Ghost animation cycles every animation_period frames.
;
; Animation has two flavours, switched on fright_active:
;   * Normal — call vdu_sprite_next_frame to cycle through frames 0-7 of
;     each ghost sprite.
;   * Fright — alternate frames 8 and 9 (the blue frightened-ghost
;     bitmaps appended to each ghost's frame list).

state_play_animation_counter: db 10
state_play_animation_period:  equ 10
state_play_normal_anim_phase: db 0       ; 0..7 — cycles through normal frames
state_play_fright_anim_phase: db 0       ; 0..3 — low bit = wiggle, high bit = blue/white

; Eaten-ghost freeze. When Pac eats a frightened ghost, the arcade pauses
; the screen for ~1 s with only the score popup visible. Movement, ghost
; AI, the mode timer, and the fright timer are all frozen — that last
; one is what stops a long chain from eating into the fright timer.
; ghost_eat sets the timer and hides Pac + the eaten ghost; state_play_tick
; counts it down and restores visibility on expiry.
EAT_FREEZE_DURATION:        equ 60
state_play_eat_freeze_timer: db 0

; ready_clear_txt — six spaces at the same VDU cursor position as
; ready_txt in pac-man.asm. Overwrites the "READY!" string when we leave
; the READY state.

state_play_enter:
    ld a, state_play_animation_period
    ld (state_play_animation_counter), a

    ; Reset the power-pellet blink so it always starts from "visible" —
    ; state_ready_enter just drew the maze with energizers showing.
    ld a, 1
    ld (power_pellet_visible), a
    ld a, POWER_PELLET_BLINK_PERIOD
    ld (power_pellet_blink_counter), a

    ; Reset the 1UP blink (#71) so each life starts with the label
    ; visible and a full period before the first toggle.
    call display_1up_blink_reset

    ; Make sure we don't enter PLAY mid-freeze (e.g. eaten-ghost freeze
    ; carried over via DYING → READY → PLAY if Pac dies during fright).
    xor a
    ld (state_play_eat_freeze_timer), a

    ; Erase the READY! banner left over from state_ready_enter by
    ; plotting an opaque-black rectangle over it.
    ld hl, BANNER_READY_ERASE
    call vdu_buffer_select
    ld bc, BANNER_READY_REL_X
    ld de, BANNER_REL_Y
    call vdu_bitmap_plot
    ret

state_play_tick:
    ; Active player's 1UP label blinks at ~1 Hz (#71). 2UP stays
    ; static — there's no player 2 turn yet (see #69).
    call display_1up_blink
    call display_high_score
    call display_2up
    call credit_display
    macro_text_set_color VDU_COL_WHITE
    call display_player_1_score
    call bonus_life_check
    call power_pellets_blink_tick
    call fruit_tick
    call fruit_collision_check

    ; Eaten-ghost freeze: hold movement, ghost AI, mode timer, and fright
    ; timer for EAT_FREEZE_DURATION frames after ghost_eat fires. HUD and
    ; the score-popup countdown above are kept ticking so the freeze
    ; doesn't look like a hang.
    call score_popup_tick
    ld a, (state_play_eat_freeze_timer)
    or a
    jr z, @no_eat_freeze
    dec a
    ld (state_play_eat_freeze_timer), a
    ret nz                              ; still frozen
    ; Just expired — bring Pac and any eyes-mode ghosts back on screen.
    call state_play_eat_freeze_restore
@no_eat_freeze:

    ; Cycle ghost frames every N ticks. Advance the appropriate phase
    ; (normal or fright) then call ghosts_animate, which picks the right
    ; per-ghost frame based on (EATEN, fright_active, current phase):
    ;   eaten → eyes (12-15 based on direction)
    ;   fright → blue (8 or 9)
    ;   normal → staggered frame 0-7
    ld hl, state_play_animation_counter
    dec (hl)
    jr nz, @skip_anim
    ld a, state_play_animation_period
    ld (hl), a

    ld a, (fright_active)
    or a
    jr nz, @advance_fright

    ld a, (state_play_normal_anim_phase)
    inc a
    and 7
    ld (state_play_normal_anim_phase), a
    jr @apply_anim
@advance_fright:
    ld a, (state_play_fright_anim_phase)
    inc a
    and 3                              ; cycles 0..3 (blue A/B, white A/B)
    ld (state_play_fright_anim_phase), a
@apply_anim:
    call ghosts_animate
@skip_anim:

    ld a, mos_getkbmap
    rst.lil $08

    ; P — pause (stub). Check first while IX is fresh.
    ld a, (ix + $06)
    bit 7, a
    call nz, pause

    ; C — deposit credit. The HUD redraw at the top of state_play_tick
    ; picks up the new value next frame so the player sees CREDIT bump
    ; immediately.
    ld a, (ix + $0A)
    bit 2, a
    call nz, credit_deposit

    ; Movement: queued-direction tile-based turn, then 1-pixel step
    call pac_input_sample
    call pac_try_turn
    call pac_step

    ; Ghost AI: mode timer + Blinky's scatter/chase movement.
    call ghost_ai_tick

    ; Maze-clear check (#14). Arcade behaviour: the level-clear transition
    ; takes precedence over a ghost collision on the same frame, so this
    ; runs *before* pac_ghost_collision.
    ld a, (pellets_eaten_level)
    cp MAZE_TOTAL_PELLETS
    jr nz, @check_collision
    ld a, GS_LEVEL_CLEAR
    jp game_state_set

@check_collision:
    ; Pac-vs-ghost collision check. On hit, drop into DYING and let it
    ; handle the lives decrement + respawn-or-game-over transition.
    call pac_ghost_collision
    ret nz
    ld a, GS_DYING
    jp game_state_set

; state_play_eat_freeze_restore — called once when the eaten-ghost freeze
; expires. Re-shows Pac and any eyes-mode ghosts (the eaten one was
; hidden by ghost_eat; if Pac chained more than one during the same
; freeze window, the timer just gets re-armed and they all stay hidden
; until the last one expires).
; Destroys: af
state_play_eat_freeze_restore:
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    call vdu_sprite_show
    ld ix, blinky_state
    call @show_if_eyes
    ld ix, pinky_state
    call @show_if_eyes
    ld ix, inky_state
    call @show_if_eyes
    ld ix, clyde_state
@show_if_eyes:
    ld a, (ix + GS_EATEN)
    or a
    ret z                               ; not eyes — was visible the whole time
    ld a, (ix + GS_ID)
    inc a
    call vdu_sprite_select
    jp vdu_sprite_show
