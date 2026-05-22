; GS_PLAYER_SELECT — show 1 player / 2 player options, wait for input.
;
; Behaviour preserved from the original flow:
;   * 1 player message is shown immediately
;   * 2 player message is shown once credit >= 2
;   * '1' starts a 1-player game, '2' starts a 2-player game (credit >= 2)
;   * 'C' continues to add credit
;
; num_players: 1 or 2, set on transition out.
;
; If no input arrives within PLAYER_SELECT_DEMO_TIMEOUT frames, hand off
; to GS_ATTRACT_DEMO (#11). This is what makes the attract cycle reach
; the demo: SPLASH → CHARS → HOLD → PLAYER_SELECT → DEMO → CHARS …

PLAYER_SELECT_DEMO_TIMEOUT: equ 600       ; 10 s at 60 Hz

num_players: db 0
player_select_demo_timer: dw PLAYER_SELECT_DEMO_TIMEOUT

state_player_select_enter:
    ld a, SPRITE_INKY_ID
    call vdu_sprite_select
    call vdu_sprite_hide

    ld a, SPRITE_CLYDE_ID
    call vdu_sprite_select
    call vdu_sprite_hide

    ld a, SPRITE_PINKY_ID
    call vdu_sprite_select
    call vdu_sprite_hide

    ld a, SPRITE_BLINKY_ID
    call vdu_sprite_select
    call vdu_sprite_hide

    ; #39 chase animation can leave Pac and the score popup on screen
    ; (whether it ends naturally or via a fast-forward through poll_input).
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    call vdu_sprite_hide

    ld a, SPRITE_SCORE_POPUP_ID
    call vdu_sprite_select
    call vdu_sprite_hide

    call vdu_screen_graphics_clear

    ; Reset the demo hand-off timer so each entry to this state gets a
    ; fresh PLAYER_SELECT_DEMO_TIMEOUT window before the demo kicks in.
    ld hl, PLAYER_SELECT_DEMO_TIMEOUT
    ld (player_select_demo_timer), hl

    ; Match the BASHFUL (Inky) cyan used on the previous attract screen.
    macro_text_set_color 0x16
    ld hl, one_player_message
    call vdu_text_print
    ret

state_player_select_tick:
    ; Set the HUD colour explicitly each tick — otherwise the yellow
    ; from two_player_message at the bottom of the previous tick leaks
    ; into display_1up / display_high_score / display_2up here.
    macro_text_set_color VDU_COL_WHITE
    call display_1up
    call display_high_score
    call display_2up
    call credit_display

    macro_text_set_color VDU_COL_BRIGHT_YELLOW
    ld hl, push_start_data
    call vdu_text_print

    macro_text_set_color VDU_COL_WHITE
    ld hl, bonus_message
    call vdu_text_print
    ld hl, trade_and_copy_message
    call vdu_text_print

    ; Show 2-player option once a second credit is in. Same Inky-cyan
    ; as one_player_message so the choice line stays a consistent colour.
    ld a, (credit)
    cp 2
    jr c, @inputs
    macro_text_set_color 0x16
    ld hl, two_player_message
    call vdu_text_print

@inputs:
    ; Demo hand-off — if no input arrives within the timeout window,
    ; transition to the AI-driven demo (#11). The timer is reset on
    ; entry so transient idle from a prior visit doesn't shortcut.
    ld hl, (player_select_demo_timer)
    dec hl
    ld (player_select_demo_timer), hl
    ld a, h
    or l
    jr nz, @no_demo_timeout
    ld a, GS_ATTRACT_DEMO
    jp game_state_set
@no_demo_timeout:

    ld a, mos_getkbmap
    rst.lil $08

    ; C — deposit more credit
    ld a, (ix + $0A)
    bit 2, a
    call nz, credit_deposit

    ; 1 — start 1 player
    ld a, (ix + $06)
    bit 0, a
    jp nz, @start_one

    ; 2 — start 2 players (only if at least 2 credits)
    ld a, (credit)
    cp 2
    ret c
    ld a, (ix + $06)
    bit 1, a
    jp nz, @start_two
    ret

@start_one:
    jp player_start_one_player
@start_two:
    jp player_start_two_player

; player_start_one_player / player_start_two_player — commit a new game.
; Decrements credit, resets per-game state (lives, fruit, level, score),
; sets num_players, and transitions to GS_READY. Called from both
; PLAYER_SELECT (the post-credit second screen) AND ATTRACT_HOLD (so
; banked credit can start a game without round-tripping through the
; second screen). Caller is responsible for verifying credit beforehand.
; Destroys: af, bc, de, hl
player_start_one_player:
    ld a, 1
    ld (num_players), a
    ld a, (credit)
    or a
    ret z
    dec a
    ld (credit), a
    ld a, 1
    ld (current_level), a
    call lives_init
    call fruit_init
    call level_init
    ld hl, 0
    ld (player_1_score), hl
    ld a, GS_READY
    jp game_state_set

player_start_two_player:
    ld a, 2
    ld (num_players), a
    ld a, (credit)
    sub 2
    ld (credit), a
    ld a, 1
    ld (current_level), a
    call lives_init
    call fruit_init
    call level_init
    ld hl, 0
    ld (player_1_score), hl
    ld a, GS_READY
    jp game_state_set
