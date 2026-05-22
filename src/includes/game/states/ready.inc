; GS_READY — two-phase pre-play sequence.
;
;   Phase 0  ANNOUNCE (2 s)  — Maze + ghosts + READY!. "PLAYER ONE" (or
;                              "PLAYER TWO") in Inky cyan just above the
;                              pen. Pac NOT yet on the maze. HUD shows
;                              the full lives count (3 at game start).
;                              Only runs on a fresh game; respawn /
;                              level-clear entries skip straight to
;                              phase 1.
;   Phase 1  PAC      (2 s)  — PLAYER text gone. One life deducted from
;                              the HUD (per lives_pop's pending gate).
;                              Pac placed at his start tile as the
;                              closed-mouth full-circle frame.
;
; When phase 1 expires we hand off to GS_PLAY.
;
; state_ready_show_announce is the one-shot trigger for phase 0; set by
; player_start_one_player / player_start_two_player and cleared by
; state_ready_enter. Dying.inc and level_clear.inc deliberately leave it
; alone so respawns and level transitions skip the announce.

READY_PHASE_FRAMES:    equ 120                  ; 2 s at 60 fps
READY_PHASE_ANNOUNCE:  equ 0
READY_PHASE_PAC:       equ 1

state_ready_phase:        db READY_PHASE_PAC
state_ready_timer:        db READY_PHASE_FRAMES
state_ready_show_announce: db 0

; "PLAYER ONE" sits above the pen in Inky cyan. Positioned by PIXEL
; rather than text col/row so we can nudge it off the 8 × 8 grid.
; The values below are absolute screen coords (top-left = (0, 0)). The
; PLOT 4 (MOVE) call subtracts the current graphics origin
; (origin_left, origin_top) so the arg stays a constant expression.
PLAYER_TEXT_X:      equ 218                     ; ← nudge this in pixels
PLAYER_TEXT_Y:      equ 157                     ; ← nudge this in pixels

player_one_text:
    .db     "PLAYER ONE"
player_one_text_end:

player_two_text:
    .db     "PLAYER TWO"
player_two_text_end:

state_ready_enter:
    call @draw_static

    ; Position Pac at his start tile regardless of phase — the move
    ; is cheap and means phase 1 only needs to flip visibility.
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    ld bc, 250
    ld (sprite_x), bc
    ld de, 248
    ld (sprite_y), de
    call vdu_sprite_move_abs
    call pac_reset                              ; full-circle closed mouth

    ; Hide any fruit left over from the previous life (the appearance
    ; counter is preserved so we don't re-trigger 70/170 thresholds).
    call fruit_hide_now

    ; Ghosts: ghost_ai_init resets AI state, moves each sprite to its
    ; start position, AND tail-calls ghosts_animate which sets the
    ; per-ghost-staggered frame. Visibility is handled below in the
    ; phase branches — ghosts stay hidden through the PLAYER announce
    ; and only appear together with Pac at phase 0 → 1.
    call ghost_ai_init

    ; Pick the phase based on the one-shot announce flag.
    ld a, (state_ready_show_announce)
    or a
    jr nz, @enter_announce

    ; Respawn / level-clear / "no announce" path — phase 1 immediately.
    ; Pac AND ghosts visible from frame 1; lives_pop honoured if
    ; dying.inc set it pending.
    call ready_show_all_actors
    call lives_pop
    ld a, READY_PHASE_PAC
    ld (state_ready_phase), a
    ld a, READY_PHASE_FRAMES
    ld (state_ready_timer), a
    ret

@enter_announce:
    ; New game — phase 0. Clear the one-shot, hide everyone (Pac and
    ; all four ghosts), paint PLAYER ONE in cyan above the pen.
    xor a
    ld (state_ready_show_announce), a

    call ready_hide_all_actors

    ; "PLAYER ONE" / "PLAYER TWO" in cyan at pixel (PLAYER_TEXT_X,
    ; PLAYER_TEXT_Y). 2P selection isn't wired (no current_player
    ; tracker) so always show "PLAYER ONE" for now; when 2P games
    ; arrive, swap to player_two_text based on whose turn.
    ld a, VDU_COL_BRIGHT_CYAN
    call ready_player_text_paint
    ld hl, player_one_text
    ld bc, player_one_text_end - player_one_text
    rst.lil VDU_OUTPUT_TO_VDP
    call ready_player_text_done

    xor a
    ld (state_ready_phase), a
    ld a, READY_PHASE_FRAMES
    ld (state_ready_timer), a
    ret

; @draw_static — paint the maze + pellets + READY banner + lives HUD
; onto the back buffer. Called once at state_ready_enter; the screen-
; graphics-clear at the top is what wipes out anything left from the
; previous state (including the PLAYER text on a re-entered READY).
; Destroys: af, bc, de, hl, ix
@draw_static:
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

    ; Plot the READY! banner as a pre-rendered yellow bitmap. Lets us
    ; position it per-pixel so it sits cleanly in maze row 17 (the open
    ; lane below the pen) rather than overlapping a wall row.
    ld hl, BANNER_READY
    call vdu_buffer_select
    ld bc, BANNER_READY_REL_X
    ld de, BANNER_REL_Y
    call vdu_bitmap_plot

    ; Lives HUD: re-blit so post-death decrements show.
    call lives_display
    ; Fruit history HUD: per-level bonus fruit strip on the bottom-right.
    jp fruit_history_display

; ready_show_all_actors / ready_hide_all_actors — Pac + all four ghosts. Used to
; flip the whole cast together at the phase 0 → 1 boundary.
; Destroys: af, bc, de, hl
ready_show_all_actors:
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    call vdu_sprite_show
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
    jp vdu_sprite_show

ready_hide_all_actors:
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    call vdu_sprite_hide
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
    jp vdu_sprite_hide

; ready_player_text_paint — prepare to render PLAYER text at pixel
; (PLAYER_TEXT_X, PLAYER_TEXT_Y) in the colour in A. Sends VDU 5
; (text-at-graphics-cursor), VDU 18 GCOL (since VDU 5 text uses the
; graphics foreground, not the text foreground), and PLOT 4 to move
; the graphics cursor. Caller follows with the string bytes, then
; calls ready_player_text_done to switch back to normal text mode.
; Inputs: A = colour. Destroys: af, bc, de, hl
ready_player_text_paint:
    call vdu_plot_set_fill                     ; GCOL plot = A
    ld a, 5
    rst.lil $10                                 ; VDU 5
    ld bc, PLAYER_TEXT_X - origin_left
    ld de, PLAYER_TEXT_Y - origin_top
    jp vdu_text_set_at                          ; PLOT 4 (MOVE) → (BC, DE)

; ready_player_text_done — restore default VDU 4 text-at-text-cursor
; mode so subsequent HUD text (1UP, scores, ...) targets the text
; cursor again.
; Destroys: af
ready_player_text_done:
    ld a, 4
    rst.lil $10
    ret

state_ready_tick:
    call display_1up
    call display_high_score
    call display_2up
    call credit_display
    macro_text_set_color VDU_COL_WHITE
    call display_player_1_score

    ld hl, state_ready_timer
    dec (hl)
    ret nz

    ; Phase just expired — decide what to do next.
    ld a, (state_ready_phase)
    cp READY_PHASE_ANNOUNCE
    jr nz, @to_play
    jp @announce_to_pac

@to_play:
    ld a, GS_PLAY
    jp game_state_set

; @announce_to_pac — phase 0 → phase 1 transition.
;   * Wipe the PLAYER text by repainting it in black (same pixels,
;     same chars — easier than tracking which letters were drawn).
;   * lives_pop: deduct one life from the HUD (and erase its icon).
;   * Show Pac AND all four ghosts together — they all "take the
;     field" at the same moment.
;   * Arm the phase-1 timer.
; Destroys: af, bc, de, hl, ix, iy
@announce_to_pac:
    ld a, VDU_COL_BLACK
    call ready_player_text_paint
    ld hl, player_one_text
    ld bc, player_one_text_end - player_one_text
    rst.lil VDU_OUTPUT_TO_VDP
    call ready_player_text_done

    call lives_pop
    call ready_show_all_actors

    ld a, READY_PHASE_PAC
    ld (state_ready_phase), a
    ld a, READY_PHASE_FRAMES
    ld (state_ready_timer), a
    ret
