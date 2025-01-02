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
    .db "PAC-MAN",153," & ",169,"BNEI",13,10
    .db 31, 21, 35
    .db "Agon remake by Bitriot",0
trade_and_copy_message_end:

character_nickname_heading:
    .db 31, 22, 10
    .db "CHARACTER / NICKNAME",13,10,0
character_nickname_heading_end:

shadow_name:
    .db 31, 22, 13
    .db "-SHADOW       ",147,"BLINKY",148,13,10,0
shadow_name_end:

speedy_name:
    .db 31, 22, 16
    .db "-SPEEDY       ",147,"PINKY",148,13,10,0
speedy_name_end:

bashful_name:
    .db 31, 22, 19
    .db "-BASHFUL      ",147,"INKY",148,13,10,0
bashful_name_end:

pokey_name:
    .db 31, 22, 22
    .db "-POKEY        ",147,"CLYDE",148,13,10,0
pokey_name_end: