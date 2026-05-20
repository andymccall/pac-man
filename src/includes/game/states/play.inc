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

; ready_clear_txt — six spaces at the same VDU cursor position as
; ready_txt in pac-man.asm. Overwrites the "READY!" string when we leave
; the READY state.
ready_clear_txt:
    .db     31, 30, 26
    .db     "      "
ready_clear_txt_end:

state_play_enter:
    ld a, state_play_animation_period
    ld (state_play_animation_counter), a

    ; Reset the power-pellet blink so it always starts from "visible" —
    ; state_ready_enter just drew the maze with energizers showing.
    ld a, 1
    ld (power_pellet_visible), a
    ld a, POWER_PELLET_BLINK_PERIOD
    ld (power_pellet_blink_counter), a

    ; Erase the READY! banner left over from state_ready_enter.
    ld hl, ready_clear_txt
    ld bc, ready_clear_txt_end - ready_clear_txt
    rst.lil VDU_OUTPUT_TO_VDP
    ret

state_play_tick:
    call display_1up
    call display_high_score
    call display_2up
    call credit_display
    macro_text_set_color VDU_COL_WHITE
    call display_player_1_score
    call bonus_life_check
    call power_pellets_blink_tick
    call fruit_tick
    call fruit_collision_check

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

    ; Movement: queued-direction tile-based turn, then 1-pixel step
    call pac_input_sample
    call pac_try_turn
    call pac_step

    ; Ghost AI: mode timer + Blinky's scatter/chase movement.
    call ghost_ai_tick

    ; Pac-vs-ghost collision check. On hit, drop into DYING and let it
    ; handle the lives decrement + respawn-or-game-over transition.
    call pac_ghost_collision
    ret nz
    ld a, GS_DYING
    jp game_state_set
