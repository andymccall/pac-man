; Global variables

SPRITE_COUNT:   equ 15


push_start_data:
    .db     31, 24, 15
    .db     "PUSH START BUTTON", 13, 10, 0
push_start_data_end:

one_player_message:
    .db 31, 26, 20
    .db "1 PLAYER ONLY",13,10,0
one_player_message_end:

two_player_message:
    .db 31, 26, 20
    .db "1 OR 2 PLAYERS",13,10,0
two_player_message_end:

bonus_message:
    .db 31, 19, 30
    .db "BONUS PAC-MAN FOR 10000 PTS",13,10,0
bonus_message_end:

trade_and_copy_message:
    .db 31, 24, 34
    .db "PAC-MAN",153," & ",169,"BNEI",13,10, 0
trade_and_copy_message_end: