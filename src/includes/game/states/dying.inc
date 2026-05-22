; GS_DYING — Pac just collided with a ghost.
;
; Hide the ghosts (mirrors arcade: ghosts vanish before Pac's death
; animation), play the 11-frame death animation on the Pac-Man sprite,
; then route to READY (which respawns sprites and deducts a life from
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

state_dying_anim_period: equ 8        ; ticks per death frame (~0.13 s)
state_dying_post_hold:   equ 30       ; ticks held on the last frame

state_dying_anim_index: db 0          ; 0..10 — death frame currently shown
state_dying_anim_tick:  db 0          ; ticks remaining before next frame
state_dying_post_tick:  db 0          ; ticks remaining once anim done

state_dying_enter:
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

    ; Snap Pac to the first death frame.
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
