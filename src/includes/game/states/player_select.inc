; GS_PLAYER_SELECT — show 1 player / 2 player options, wait for input.
;
; Behaviour preserved from the original flow:
;   * 1 player message is shown immediately
;   * 2 player message is shown once credit >= 2
;   * '1' starts a 1-player game, '2' starts a 2-player game (credit >= 2)
;   * 'C' continues to add credit
;
; num_players: 1 or 2, set on transition out.

num_players: db 0

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

    call vdu_screen_graphics_clear

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
    ld a, 1
    ld (num_players), a
    ld a, (credit)
    or a
    ret z
    dec a
    ld (credit), a
    call lives_init
    call fruit_init
    ld hl, 0
    ld (player_1_score), hl
    ld a, GS_READY
    jp game_state_set

@start_two:
    ld a, 2
    ld (num_players), a
    ld a, (credit)
    sub 2
    ld (credit), a
    call lives_init
    call fruit_init
    ld hl, 0
    ld (player_1_score), hl
    ld a, GS_READY
    jp game_state_set
