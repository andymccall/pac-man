; GS_DYING — Pac just collided with a ghost.
;
; Three phases (matches the arcade):
;
;   1. PRE-ANIM FREEZE (~1 s) — everyone on screen frozen as they were
;      at the moment of collision. Pac stays in his current direction
;      frame, all four ghosts stay visible. Nothing animates or moves.
;   2. DEATH ANIMATION — ghosts vanish, Pac snaps to PAC_DEATH_FIRST_FRAME
;      and cycles through the 11-frame death sequence.
;   3. POST-HOLD — Pac held on the final death frame briefly.
;
; Then route to READY (which respawns sprites and deducts a life from
; the HUD at the +1s mark) or to GAME_OVER if lives is already 0.
;
; The lives decrement lives in READY, not here — see lives_pop. The
; effect is that the HUD always briefly shows the current lives count
; (including the one Pac is about to play), then settles into the
; reserves-only display after Pac is "placed".
;
; Death frames are appended to the Pac-Man sprite as frames 12-22
; (PAC_DEATH_FIRST_FRAME..PAC_DEATH_LAST_FRAME). state_dying_anim_period
; controls per-frame hold; the total visible-death time is
; 11 * period plus state_dying_post_hold extra frames on the last frame.

state_dying_freeze_frames: equ 60     ; ~1 s @ 60 fps — arcade pre-anim pause
state_dying_anim_period:   equ 8      ; ticks per death frame (~0.13 s)
state_dying_post_hold:     equ 60     ; #31: was 30 (~500 ms); 60 (~1 s) matches arcade footage

state_dying_freeze_timer: db 0        ; ticks remaining in the pre-anim freeze
state_dying_anim_index:   db 0        ; 0..10 — death frame currently shown
state_dying_anim_tick:    db 0        ; ticks remaining before next frame
state_dying_post_tick:    db 0        ; ticks remaining once anim done

state_dying_enter:
    ; Arm the pre-anim freeze. Ghosts stay visible and Pac stays in
    ; his current frame for the duration — the actual death sequence
    ; kicks in when the freeze expires (see dying_begin_anim).
    ld a, state_dying_freeze_frames
    ld (state_dying_freeze_timer), a
    ret

; dying_begin_anim — phase 1 → phase 2 transition. Hide all four ghosts,
; snap Pac to the first death frame, arm the animation timers. Called
; once from state_dying_tick when the freeze expires.
; Destroys: af
dying_begin_anim:
    ld a, SPRITE_BLINKY_ID
    call vdu_sprite_select
    call vdu_sprite_hide
    ld a, SPRITE_PINKY_ID
    call vdu_sprite_select
    call vdu_sprite_hide
    ld a, SPRITE_INKY_ID
    call vdu_sprite_select
    call vdu_sprite_hide
    ld a, SPRITE_CLYDE_ID
    call vdu_sprite_select
    call vdu_sprite_hide

    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    ld a, PAC_DEATH_FIRST_FRAME
    call vdu_sprite_select_frame

    xor a
    ld (state_dying_anim_index), a
    ld a, state_dying_anim_period
    ld (state_dying_anim_tick), a
    ld a, state_dying_post_hold
    ld (state_dying_post_tick), a
    ret

state_dying_tick:
    ; Keep the HUD ticking so the death pause doesn't look frozen.
    call display_1up
    call display_high_score
    call display_2up
    call credit_display
    macro_text_set_color VDU_COL_WHITE
    call display_player_1_score

    ; Phase 1 — pre-anim freeze. While this counts down, ghosts and
    ; Pac stay on screen exactly as they were at the collision moment.
    ld a, (state_dying_freeze_timer)
    or a
    jr z, @anim_phase
    dec a
    ld (state_dying_freeze_timer), a
    ret nz                              ; still freezing
    ; Freeze just hit 0 — kick off the death animation (hide ghosts,
    ; snap Pac to death frame, arm anim timers) and return; first
    ; anim frame is shown on the next tick.
    jp dying_begin_anim

@anim_phase:
    ; Per-frame hold.
    ld hl, state_dying_anim_tick
    dec (hl)
    jr nz, @check_done

    ld a, state_dying_anim_period
    ld (hl), a

    ; Advance to the next death frame, unless we're already on the last.
    ld a, (state_dying_anim_index)
    cp 10
    jr nc, @check_done                  ; already at last — just hold
    inc a
    ld (state_dying_anim_index), a
    add a, PAC_DEATH_FIRST_FRAME
    push af
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    pop af
    call vdu_sprite_select_frame

@check_done:
    ; Animation finished only when we've reached the last frame AND the
    ; post-hold timer has elapsed.
    ld a, (state_dying_anim_index)
    cp 10
    ret c                                ; still animating

    ld hl, state_dying_post_tick
    dec (hl)
    ret nz

    ; Hold expired — decide what happens next.
    ; Demo mode (#11): the GAME OVER banner is already on screen
    ; (plotted by state_attract_demo_enter) so we skip GS_GAME_OVER
    ; entirely and bounce straight back to ATTRACT_CHARS.
    ld a, (demo_active)
    or a
    jr nz, @demo_done

    ; READY does the actual decrement; here we only need to know
    ; whether this was the final life. lives==0 means READY already
    ; took the last reserve, so the run we just died on was the last
    ; one — no more respawns.
    ld a, (lives)
    or a
    jr z, @game_over

    ld a, 1
    ld (lives_pop_pending), a       ; arm the HUD deduction for the
                                    ; respawn READY (level-clear path
                                    ; deliberately doesn't)
    ld a, GS_READY
    jp game_state_set
@game_over:
    ld a, GS_GAME_OVER
    jp game_state_set
@demo_done:
    xor a
    ld (demo_active), a
    ld a, GS_ATTRACT_CHARS
    jp game_state_set
