; GS_INTERMISSION — between-level cutscenes (issue #13).
;
; Arcade Pac-Man plays three "acts" after specific levels:
;   Act 1 — after level 2
;   Act 2 — after level 5
;   Act 3 — after levels 9, 13, 17
;
; MVP scope: title text + static pac/blinky bitmaps over it + Pac-Man
; running left → right across the screen. No big-sprite chase, no
; ghost rip — those land in a follow-up arcade-fidelity issue.
;
; Entry: level_clear's @post_hold checks the just-cleared level via
; intermission_act_for_level. If it returns non-zero, the act is stored
; in state_intermission_act and we transition here. After the run-across
; completes we route back to GS_READY for the next level.
;
; Dev affordance: attract_hold polls keys 1/2/3 and sets
; state_intermission_dev_mode = 1 + state_intermission_act before
; transitioning. Exit then goes to GS_ATTRACT_CHARS instead of GS_READY
; so the title screen redraws cleanly.

; Pac runs at this Y in absolute screen coords. ~halfway down the
; screen, below the static title sprites and just above the title
; text band.
; Match the attract-screen chase row so the cutscene Pac runs along
; the same line as the title-screen chase (see CHASE_SPRITE_Y in
; attract_hold.inc).
INTERMISSION_PAC_Y:           equ 196
; Pac runs RIGHT → LEFT within the arcade-faithful 224 px play area.
; That area spans absolute x=146..370 (= origin_left + 8 to
; origin_left + 8 + 28*8), with black borders either side of the
; 512 px screen. Pac starts off the right edge of the play area and
; exits off the left edge.
INTERMISSION_PAC_START_X:     equ 370        ; right edge of 224 play area
INTERMISSION_PAC_END_X:       equ 130        ; left edge minus sprite width
INTERMISSION_PAC_SPEED:       equ 2          ; pixels per frame (always +; we subtract)
INTERMISSION_PAC_ANIM_PERIOD: equ 5          ; frames per mouth state
; Pac-Man sprite-frame indices for the left-facing chomp cycle.
INTERMISSION_PAC_FRAME_BASE:  equ 3          ; 3 = wide-open left, 4 = half, 5 = closed

; Static-bitmap positions for the title-row pac + blinky. "PAC-MAN" at
; row 28, col 28 spans pixel rows 224-231 / pixel cols 224-279. The
; first "A" is at col 29 (x=232..239); the second "A" in MAN at col
; 33 (x=264..271). 16×16 sprites centred above each A → sprite top
; at y=200, sprite left-edges at x=228 and x=260.
INTERMISSION_STATIC_PAC_X:    equ 228
INTERMISSION_STATIC_BLINKY_X: equ 260
INTERMISSION_STATIC_Y:        equ 160

; Act-2-specific positions for the wide 32x16 "caught on the floor"
; pose. Independent of the Acts 1 / 3 static positions above so this
; sprite can be placed wherever its anatomy looks right without
; nudging the other acts' layout.
INTERMISSION_STATIC_ACT2_BLINKY_X: equ 240
INTERMISSION_STATIC_ACT2_BLINKY_Y: equ 160

; Phase machine:
;   0 = ACT screen   — title + static sprites on a black canvas, no HUD
;   1 = PAUSE       — black canvas, Pac NOT yet visible
;   2 = RUN         — Pac chomps right → left across a black canvas
INTERMISSION_PHASE_ACT:       equ 0
INTERMISSION_PHASE_PAUSE:     equ 1
INTERMISSION_PHASE_RUN:       equ 2
INTERMISSION_ACT_HOLD_FRAMES: equ 180         ; 3 s @ 60 fps  — ACT screen
INTERMISSION_PAUSE_FRAMES:    equ 60          ; 1 s @ 60 fps  — black hold before Pac

state_intermission_act:      db 1            ; 1 / 2 / 3
state_intermission_dev_mode: db 0            ; 1 = exit to ATTRACT_CHARS, 0 = exit to READY
state_intermission_phase:    db 0            ; INTERMISSION_PHASE_*
state_intermission_phase_timer: db INTERMISSION_ACT_HOLD_FRAMES  ; reused per phase
state_intermission_pac_x:    dl 0            ; current Pac sprite_x during run phase
state_intermission_anim_tick: db INTERMISSION_PAC_ANIM_PERIOD
state_intermission_anim_phase: db 0          ; 0..2 cycling for the chomp anim

; Title-text data. VDU 31, col, row prefix + the string. "PAC-MAN" is
; 7 chars, centred on col 28 of the 64-col grid. "  ACT N" sits two
; rows below, same start col so the "ACT" lines up under the dash.
intermission_title_pac_man:
    .db 31, 28, 23
    .db "PAC-MAN", 0
intermission_title_pac_man_end:

intermission_title_act_1:
    .db 31, 28, 24
    .db " ACT 1", 0
intermission_title_act_1_end:

intermission_title_act_2:
    .db 31, 28, 24
    .db " ACT 2", 0
intermission_title_act_2_end:

intermission_title_act_3:
    .db 31, 28, 24
    .db " ACT 3", 0
intermission_title_act_3_end:

state_intermission_enter:
    ; Phase 0: ACT screen. Black canvas with just the title text +
    ; static sprites; no HUD (1UP / HIGH SCORE / 2UP / score / CREDIT
    ; are NOT drawn during the intermission). After ACT_HOLD_FRAMES
    ; the tick transitions to phase 1 and the Pac chase plays out.
    ld bc, 0
    ld de, 0
    call vdu_set_gfx_origin
    call vdu_screen_graphics_clear

    ; Hide every game sprite (Pac + the four ghosts). Pac re-appears
    ; in phase 1 as the running sprite; the ghosts stay hidden the
    ; whole time.
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
    call vdu_sprite_hide

    ; Draw title text.
    macro_text_set_color VDU_COL_WHITE
    ld hl, intermission_title_pac_man
    call vdu_text_print

    ld a, (state_intermission_act)
    cp 2
    jr z, @act_2_title
    cp 3
    jr z, @act_3_title
    ld hl, intermission_title_act_1
    jr @print_act_title
@act_2_title:
    ld hl, intermission_title_act_2
    jr @print_act_title
@act_3_title:
    ld hl, intermission_title_act_3
@print_act_title:
    call vdu_text_print

    ; Static sprites above the title. Per-act layout — Acts 1 and 3
    ; show Pac + Blinky over each "A"; Act 2 just shows the ghost
    ; (the arcade's "Blinky caught" pose — we use the standard frame
    ; for the MVP, since no rip asset exists yet).
    ld a, (state_intermission_act)
    cp 2
    jr z, @static_act_2
    cp 3
    jr z, @static_act_3

    ; --- Act 1 static: Pac (normal/half-open mouth left) + Blinky left ---
    ld hl, SPRITE_PAC_MAN_01_01                 ; half-open left
    call vdu_buffer_select
    ld bc, INTERMISSION_STATIC_PAC_X
    ld de, INTERMISSION_STATIC_Y
    call vdu_bitmap_plot
    ld hl, SPRITE_BLINKY_2                      ; left-facing anim phase 0
    call vdu_buffer_select
    ld bc, INTERMISSION_STATIC_BLINKY_X
    ld de, INTERMISSION_STATIC_Y
    call vdu_bitmap_plot
    jr @sprites_done

@static_act_2:
    ; Wide 32x16 "Blinky caught on the floor" pose. Position is
    ; independent of Acts 1 / 3 — tweak the ACT2_BLINKY_X / Y
    ; constants near the top of the file.
    ld hl, SPRITE_BLINKY_INTERMISSION_0
    call vdu_buffer_select
    ld bc, INTERMISSION_STATIC_ACT2_BLINKY_X
    ld de, INTERMISSION_STATIC_ACT2_BLINKY_Y
    call vdu_bitmap_plot
    jr @sprites_done

@static_act_3:
    ; Pac (wide-open mouth left) + Blinky left.
    ld hl, SPRITE_PAC_MAN_01_00                 ; wide-open left
    call vdu_buffer_select
    ld bc, INTERMISSION_STATIC_PAC_X
    ld de, INTERMISSION_STATIC_Y
    call vdu_bitmap_plot
    ld hl, SPRITE_BLINKY_2
    call vdu_buffer_select
    ld bc, INTERMISSION_STATIC_BLINKY_X
    ld de, INTERMISSION_STATIC_Y
    call vdu_bitmap_plot

@sprites_done:
    ; Arm the phase-0 hold timer. Pac isn't shown yet — phase 1
    ; (intermission_begin_run) handles his entry.
    xor a
    ld (state_intermission_phase), a            ; PHASE_ACT
    ld a, INTERMISSION_ACT_HOLD_FRAMES
    ld (state_intermission_phase_timer), a
    ret

; intermission_begin_pause — phase 0 → phase 1 transition. Wipes the
; ACT screen to black, arms the pause timer. Pac stays hidden.
; Destroys: af, bc, de, hl
intermission_begin_pause:
    call vdu_screen_graphics_clear

    ld a, INTERMISSION_PHASE_PAUSE
    ld (state_intermission_phase), a
    ld a, INTERMISSION_PAUSE_FRAMES
    ld (state_intermission_phase_timer), a
    ret

; intermission_begin_run — phase 1 → phase 2 transition. Positions
; Pac off-screen right, resets the chomp anim, and shows the Pac
; sprite. The tick takes over from here. Origin is already (0,0)
; from enter; canvas is already black from intermission_begin_pause.
; Destroys: af, bc, de, hl
intermission_begin_run:
    ld hl, INTERMISSION_PAC_START_X
    ld (state_intermission_pac_x), hl
    xor a
    ld (state_intermission_anim_phase), a
    ld a, INTERMISSION_PAC_ANIM_PERIOD
    ld (state_intermission_anim_tick), a

    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    ld a, INTERMISSION_PAC_FRAME_BASE           ; frame 3 = wide-open left
    call vdu_sprite_select_frame

    ld bc, (state_intermission_pac_x)
    ld de, INTERMISSION_PAC_Y
    call vdu_sprite_move_abs
    call vdu_sprite_show

    ld a, INTERMISSION_PHASE_RUN
    ld (state_intermission_phase), a
    ret

state_intermission_tick:
    ; No HUD here — the ACT screen is a fully black canvas, the pause
    ; and run screens mirror it. The score / 1UP / CREDIT bars come
    ; back with the next game state (GS_READY or GS_ATTRACT_CHARS).
    ld a, (state_intermission_phase)
    cp INTERMISSION_PHASE_RUN
    jr z, @run_phase
    cp INTERMISSION_PHASE_PAUSE
    jr z, @pause_phase

    ; Phase 0 (ACT) — count down, then wipe to black for the pause.
    ld a, (state_intermission_phase_timer)
    dec a
    ld (state_intermission_phase_timer), a
    ret nz
    jp intermission_begin_pause

@pause_phase:
    ; Phase 1 — hold a black canvas before Pac enters.
    ld a, (state_intermission_phase_timer)
    dec a
    ld (state_intermission_phase_timer), a
    ret nz
    jp intermission_begin_run

@run_phase:
    ; --- Animation ---------------------------------------------------
    ld a, (state_intermission_anim_tick)
    dec a
    ld (state_intermission_anim_tick), a
    jr nz, @anim_done

    ld a, INTERMISSION_PAC_ANIM_PERIOD
    ld (state_intermission_anim_tick), a
    ld a, (state_intermission_anim_phase)
    inc a
    cp 3
    jr c, @anim_save
    xor a
@anim_save:
    ld (state_intermission_anim_phase), a
    add a, INTERMISSION_PAC_FRAME_BASE          ; offset 0..2 → 3..5 (left-facing)
    push af
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    pop af
    call vdu_sprite_select_frame
@anim_done:

    ; --- Movement (right → left, so subtract) -----------------------
    ld hl, (state_intermission_pac_x)
    ld de, INTERMISSION_PAC_SPEED
    or a                                        ; clear carry for sbc
    sbc hl, de
    ld (state_intermission_pac_x), hl

    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    ld bc, (state_intermission_pac_x)
    ld de, INTERMISSION_PAC_Y
    call vdu_sprite_move_abs

    ; --- Exit check: pac_x < END_X means we're off the left edge ----
    ld hl, (state_intermission_pac_x)
    ld de, INTERMISSION_PAC_END_X
    or a
    sbc hl, de
    ret nc                                      ; pac_x >= END_X — keep running

    ; Pac is off-screen right. Hide him and route to the next state.
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    call vdu_sprite_hide

    ld a, (state_intermission_dev_mode)
    or a
    jr nz, @dev_exit

    ld a, GS_READY
    jp game_state_set

@dev_exit:
    xor a
    ld (state_intermission_dev_mode), a
    ld a, GS_ATTRACT_CHARS
    jp game_state_set

; intermission_act_for_level — A = level number (1-based, the level
; that was JUST cleared). Returns A = act number (1/2/3) or 0 if no
; intermission applies for that level. Used by level_clear's
; @post_hold to route into the cutscene.
; Destroys: af
intermission_act_for_level:
    cp 2
    jr z, @act1
    cp 5
    jr z, @act2
    cp 9
    jr z, @act3
    cp 13
    jr z, @act3
    cp 17
    jr z, @act3
    xor a
    ret
@act1:
    ld a, 1
    ret
@act2:
    ld a, 2
    ret
@act3:
    ld a, 3
    ret
