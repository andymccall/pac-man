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
    .db 31, 24, 37
    .db "PAC-MAN",153," & ",169,"BNEI",13,10
    .db 31, 21, 38
    .db "Agon remake by Bitriot",0
trade_and_copy_message_end:

character_nickname_heading:
    .db 31, 22, 09
    .db "CHARACTER / NICKNAME",13,10,0
character_nickname_heading_end:

blinky_name:
    .db 31, 22, 12
    .db "-SHADOW       ",147,"BLINKY",148,13,10,0
blinky_name_end:

pinky_name:
    .db 31, 22, 15
    .db "-SPEEDY       ",147,"PINKY",148,13,10,0
pinky_name_end:

inky_name:
    .db 31, 22, 18
    .db "-BASHFUL      ",147,"INKY",148,13,10,0
inky_name_end:

clyde_name:
    .db 31, 22, 21
    .db "-POKEY        ",147,"CLYDE",148,13,10,0
clyde_name_end:

pellet_points:
    .db 31, 28, 31
    .db "10 PTS",13,10,0
pellet_points_end:

power_pellet_points:
    .db 31, 28, 33
    .db "50 PTS",13,10,0
power_pellet_points_end: