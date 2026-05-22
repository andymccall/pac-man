; GS_ATTRACT_DEMO — AI-driven Pac demo inside the attract cycle (#11).
;
; Phase-1 plumbing: state is wired end-to-end but Pac stands still.
; That's deliberate — it verifies the lifecycle (enter → ready pause →
; "playing" tick → ghost catches Pac → DYING → ATTRACT_CHARS) before
; any AI logic exists. Later phases plug pac_ai_sample (heuristic) or
; pac_script_sample (recorded route) into the input swap point below.
;
; Flow:
;   ENTER         — same setup as a new game (level=1, lives=1, score=0,
;                   fruit + level + sprites reset, maze drawn) plus the
;                   demo_active flag set so DYING knows to bounce back
;                   to ATTRACT_CHARS instead of GAME_OVER.
;   READY pause   — brief hold so the maze and sprites have time to
;                   render; mirrors GS_READY's 2 s pause but shorter.
;   PLAYING       — runs the same per-frame gameplay tick as GS_PLAY,
;                   but with the Pac-input source swapped (none for now)
;                   and a fail-safe timer counting down so the demo
;                   can't loop forever if Pac never dies.
;   EXIT          — collision → GS_DYING → ATTRACT_CHARS (via dying.inc
;                   demo_active branch). Fail-safe timeout → straight to
;                   ATTRACT_CHARS. C / 1 / 2 input → PLAYER_SELECT or
;                   game start, same as ATTRACT_HOLD's poll_input.

; Tunables -----------------------------------------------------------------

DEMO_READY_FRAMES:    equ 60          ; 1 s pause before AI takes over
DEMO_FAILSAFE_FRAMES: equ 1800        ; 30 s cap on the demo

DEMO_PHASE_READY:     equ 0
DEMO_PHASE_PLAYING:   equ 1

; State --------------------------------------------------------------------

; demo_active — checked by state_dying (next-state choice), and by
; future phases for the Pac input swap. Lives outside this state's
; locals so other files can branch on it without a forward include.
demo_active:                db 0

state_attract_demo_phase:   db DEMO_PHASE_READY
state_attract_demo_ready:   db DEMO_READY_FRAMES
; 3-byte allocation: `ld (state_attract_demo_failsafe), hl` and the
; matching read are 24-bit in ADL, so a `dw` here would spill HLU into
; the next byte — which is `state_attract_demo_enter`'s first code
; byte. Same flavour of bug as #54's chase_pac_accum / chase_ghost_accum.
state_attract_demo_failsafe: dl DEMO_FAILSAFE_FRAMES

; Entry --------------------------------------------------------------------

state_attract_demo_enter:
    ld a, 1
    ld (demo_active), a

    ; New-game style reset. Mirrors player_start_one_player minus the
    ; credit decrement (no credit consumed by the demo) and minus the
    ; final game_state_set (we stay in GS_ATTRACT_DEMO, not GS_READY).
    ld a, 1
    ld (current_level), a
    call lives_init
    ; Demo runs on a single life so the first ghost catch falls through
    ; to GS_GAME_OVER (which shows the GAME OVER banner before returning
    ; to ATTRACT_CHARS). Without this override lives_init = 3 → Pac
    ; would respawn through GS_READY instead of ending the demo.
    ld a, 1
    ld (lives), a
    call fruit_init
    call level_init
    ld hl, 0
    ld (player_1_score), hl

    ; Draw the maze and place sprites — same code path as GS_READY's
    ; enter so the demo starts from a known visual state.
    call state_ready_enter_for_demo

    ; Plot GAME OVER throughout the demo. The arcade uses the banner as
    ; an attract-mode marker — viewers see it permanently while the AI
    ; plays, signalling "not a real game in progress". On Pac's death
    ; we route straight back to ATTRACT_CHARS (dying.inc demo branch),
    ; skipping the dedicated GS_GAME_OVER state entirely.
    ld hl, BANNER_GAME_OVER
    call vdu_buffer_select
    ld bc, BANNER_GAME_OVER_REL_X
    ld de, BANNER_REL_Y
    call vdu_bitmap_plot

    ; Reset phase + timers.
    xor a
    ld (state_attract_demo_phase), a
    ld a, DEMO_READY_FRAMES
    ld (state_attract_demo_ready), a
    ld hl, DEMO_FAILSAFE_FRAMES
    ld (state_attract_demo_failsafe), hl

    ; Reset the scripted-route cursor.
    call pac_script_reset
    ret

; state_ready_enter_for_demo — borrow GS_READY's maze + sprite reset.
; Identical to state_ready_enter except we don't arm the 2 s READY timer
; (the demo has its own DEMO_READY_FRAMES counter) and don't show the
; READY! banner (the arcade demo doesn't either — the maze is just live
; once it appears).
; Destroys: af, bc, de, hl, ix
state_ready_enter_for_demo:
    call vdu_screen_graphics_clear

    ld bc, origin_left + 8
    ld de, origin_top + 8
    call vdu_set_gfx_origin

    ld ix, maze_wall
    ld hl, SPRITE_MAZE_TILE_00
    call draw_maze
    ld ix, maze_pellets
    ld hl, SPRITE_NULL_PELLET_00
    call draw_maze

    ld bc, origin_left
    ld de, origin_top
    call vdu_set_gfx_origin

    call lives_display
    call fruit_history_display

    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    ld bc, 250
    ld (sprite_x), bc
    ld de, 248
    ld (sprite_y), de
    call vdu_sprite_move_abs
    call vdu_sprite_show
    call pac_reset

    call fruit_hide_now
    call ghost_ai_init

    ld a, SPRITE_BLINKY_ID
    call vdu_sprite_select
    call vdu_sprite_show
    ld a, SPRITE_PINKY_ID
    call vdu_sprite_select
    call vdu_sprite_show
    ld a, SPRITE_INKY_ID
    call vdu_sprite_select
    call vdu_sprite_show
    ld a, SPRITE_CLYDE_ID
    call vdu_sprite_select
    call vdu_sprite_show
    ret

; Tick ---------------------------------------------------------------------

state_attract_demo_tick:
    ; HUD — always painted so the demo looks live.
    call display_1up
    call display_high_score
    call display_2up
    call credit_display
    macro_text_set_color VDU_COL_WHITE
    call display_player_1_score

    ; C / 1 / 2 — fast-forward to PLAYER_SELECT or start a game with
    ; banked credit. Same shape as attract_hold's poll_input.
    call @poll_input

    ld a, (state_attract_demo_phase)
    cp DEMO_PHASE_PLAYING
    jp z, @playing

    ; READY pause — count down then flip phase. The world is static
    ; until the timer expires.
    ld hl, state_attract_demo_ready
    dec (hl)
    ret nz
    ld a, DEMO_PHASE_PLAYING
    ld (state_attract_demo_phase), a
    ret

@playing:
    ; Fail-safe countdown — guarantees the demo ends even if the AI
    ; never dies (and during phase 1 it definitely won't, since Pac
    ; stands still and waits to be caught).
    ld hl, (state_attract_demo_failsafe)
    dec hl
    ld (state_attract_demo_failsafe), hl
    ld a, h
    or l
    jr nz, @no_timeout
    jp @demo_to_attract
@no_timeout:

    ; The rest mirrors state_play_tick. Differences:
    ;   - no pac_input_sample (Pac input swap point — phase 2 plugs the
    ;     AI here)
    ;   - no 'P' pause handling (would be confusing in attract)
    ;   - 'C' is handled in @poll_input above, not here
    ;   - collision routes to GS_DYING the same way; dying.inc branches
    ;     on demo_active for the next-state choice
    call bonus_life_check
    call power_pellets_blink_tick
    call fruit_tick
    call fruit_collision_check

    call score_popup_tick
    ld a, (state_play_eat_freeze_timer)
    or a
    jr z, @no_eat_freeze
    dec a
    ld (state_play_eat_freeze_timer), a
    ret nz
    call state_play_eat_freeze_restore
@no_eat_freeze:

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
    and 3
    ld (state_play_fright_anim_phase), a
@apply_anim:
    call ghosts_animate
@skip_anim:

    ; Pac movement — script first, heuristic fallback.
    ; While the script is active, pac_script_sample applies forced turns
    ; at waypoint tiles and returns A=1 — we skip the heuristic so it
    ; can't second-guess the arcade path. Once the cursor runs out
    ; (returns A=0) the heuristic AI takes over to keep Pac moving.
    call pac_script_sample
    or a
    jr nz, @after_input
    call pac_ai_sample
@after_input:
    call pac_try_turn
    call pac_step

    call ghost_ai_tick

    ; Level clear (would be a 244-pellet eat — won't happen in phase 1).
    ld a, (pellets_eaten_level)
    cp MAZE_TOTAL_PELLETS
    jr nz, @check_collision
    jp @demo_to_attract                 ; demo somehow cleared the maze;
                                        ; bail to attract rather than
                                        ; advancing levels in demo mode

@check_collision:
    call pac_ghost_collision
    ret nz
    ld a, GS_DYING
    jp game_state_set                   ; dying.inc sends us back to
                                        ; ATTRACT_CHARS via demo_active

@demo_to_attract:
    xor a
    ld (demo_active), a
    ld a, GS_ATTRACT_CHARS
    jp game_state_set

; @poll_input — C deposits a credit and jumps to PLAYER_SELECT; 1 / 2
; with banked credit starts a game. Either way demo_active is cleared
; so subsequent state behaviour reverts to normal.
@poll_input:
    ld a, mos_getkbmap
    rst.lil $08
    ld a, (ix + $0A)
    bit 2, a
    jr z, @ip_check_start
    xor a
    ld (demo_active), a
    call credit_deposit
    ld a, GS_PLAYER_SELECT
    jp game_state_set
@ip_check_start:
    ld a, (credit)
    or a
    ret z
    ld a, (ix + $06)
    bit 0, a
    jr nz, @ip_start_one
    ld a, (credit)
    cp 2
    ret c
    ld a, (ix + $06)
    bit 1, a
    jr nz, @ip_start_two
    ret
@ip_start_one:
    xor a
    ld (demo_active), a
    jp player_start_one_player
@ip_start_two:
    xor a
    ld (demo_active), a
    jp player_start_two_player
