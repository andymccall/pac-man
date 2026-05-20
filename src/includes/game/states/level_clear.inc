; GS_LEVEL_CLEAR — placeholder. Will hold the maze-flash + next-level setup.
state_level_clear_enter:
    ret

state_level_clear_tick:
    ld a, GS_GAME_OVER
    jp game_state_set
