; GS_ATTRACT_HOLD — character menu fully drawn, waiting for credit.
; The screen is already painted by the previous state; we only watch input.

state_attract_hold_enter:
    ret

state_attract_hold_tick:
    ld a, mos_getkbmap
    rst.lil $08

    ; C key — deposit credit
    ld a, (ix + $0A)
    bit 2, a
    call nz, credit_deposit

    ; Any credit available? -> player select
    ld a, (credit)
    or a
    ret z
    ld a, GS_PLAYER_SELECT
    jp game_state_set
