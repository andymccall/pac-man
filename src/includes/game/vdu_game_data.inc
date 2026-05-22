vdu_game_data:

    ; Banner bitmaps (READY!, READY-erase, GAME OVER) are pre-rendered
    ; from the arcade font as 48x8 / 72x8 RGBA2 images and plotted at
    ; pixel-precise positions in state_ready_enter / state_play_enter /
    ; state_game_over_enter. The text-grid (VDU 31) only steps in 8-px
    ; rows so the banner couldn't sit cleanly in the maze row 17 lane.
    macro_bitmap_make_buffered BANNER_READY,       BANNER_READY_W,      BANNER_BANNER_H, "src/assets/banner/ready.rgba2"
    macro_bitmap_make_buffered BANNER_READY_ERASE, BANNER_READY_W,      BANNER_BANNER_H, "src/assets/banner/ready_erase.rgba2"
    macro_bitmap_make_buffered BANNER_GAME_OVER,   BANNER_GAME_OVER_W,  BANNER_BANNER_H, "src/assets/banner/game_over.rgba2"


; Pac-Man

    macro_bitmap_make_buffered SPRITE_PAC_MAN_00_00, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_man_00_00.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_MAN_00_01, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_man_00_01.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_MAN_00_02, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_man_00_02.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_MAN_01_00, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_man_01_00.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_MAN_01_01, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_man_01_01.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_MAN_01_02, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_man_01_02.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_MAN_02_00, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_man_02_00.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_MAN_02_01, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_man_02_01.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_MAN_02_02, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_man_02_02.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_MAN_03_00, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_man_03_00.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_MAN_03_01, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_man_03_01.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_MAN_03_02, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_man_03_02.rgba2"

    ; Death frames — appended to the Pac-Man sprite as frames 12-22.
    macro_bitmap_make_buffered SPRITE_PAC_DED_00_00, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_ded_00_00.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_DED_00_01, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_ded_00_01.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_DED_00_02, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_ded_00_02.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_DED_00_03, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_ded_00_03.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_DED_00_04, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_ded_00_04.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_DED_00_05, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_ded_00_05.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_DED_00_06, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_ded_00_06.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_DED_00_07, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_ded_00_07.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_DED_00_08, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_ded_00_08.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_DED_00_09, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_ded_00_09.rgba2"
    macro_bitmap_make_buffered SPRITE_PAC_DED_00_10, SPRITE_PAC_MAN_WIDTH, SPRITE_PAC_MAN_HEIGHT, "src/assets/pac_man/pac_ded_00_10.rgba2"

    macro_sprite_select_sprite SPRITE_PAC_MAN_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_PAC_MAN_00_00
    macro_sprite_add_frame SPRITE_PAC_MAN_00_01
    macro_sprite_add_frame SPRITE_PAC_MAN_00_02
    macro_sprite_add_frame SPRITE_PAC_MAN_01_00
    macro_sprite_add_frame SPRITE_PAC_MAN_01_01
    macro_sprite_add_frame SPRITE_PAC_MAN_01_02
    macro_sprite_add_frame SPRITE_PAC_MAN_02_00
    macro_sprite_add_frame SPRITE_PAC_MAN_02_01
    macro_sprite_add_frame SPRITE_PAC_MAN_02_02
    macro_sprite_add_frame SPRITE_PAC_MAN_03_00
    macro_sprite_add_frame SPRITE_PAC_MAN_03_01
    macro_sprite_add_frame SPRITE_PAC_MAN_03_02
    ; Frames 12-22: death animation.
    macro_sprite_add_frame SPRITE_PAC_DED_00_00
    macro_sprite_add_frame SPRITE_PAC_DED_00_01
    macro_sprite_add_frame SPRITE_PAC_DED_00_02
    macro_sprite_add_frame SPRITE_PAC_DED_00_03
    macro_sprite_add_frame SPRITE_PAC_DED_00_04
    macro_sprite_add_frame SPRITE_PAC_DED_00_05
    macro_sprite_add_frame SPRITE_PAC_DED_00_06
    macro_sprite_add_frame SPRITE_PAC_DED_00_07
    macro_sprite_add_frame SPRITE_PAC_DED_00_08
    macro_sprite_add_frame SPRITE_PAC_DED_00_09
    macro_sprite_add_frame SPRITE_PAC_DED_00_10

    macro_sprite_mark_hardware                  ; #15

    ;macro_sprite_activate 1

    macro_sprite_move SPRITE_PAC_MAN_ID, 250,248

    ; Reverse-ghost (frightened/blue) bitmaps — uploaded here so each of
    ; the four ghost sprites can include them as additional frames 8-15
    ; for the fright-mode visual swap.
    macro_bitmap_make_buffered SPRITE_REVERSE_0, SPRITE_REVERSE_WIDTH, SPRITE_REVERSE_HEIGHT, "src/assets/ghosts/reverse/reverse_0.rgba2"
    macro_bitmap_make_buffered SPRITE_REVERSE_1, SPRITE_REVERSE_WIDTH, SPRITE_REVERSE_HEIGHT, "src/assets/ghosts/reverse/reverse_1.rgba2"
    macro_bitmap_make_buffered SPRITE_REVERSE_2, SPRITE_REVERSE_WIDTH, SPRITE_REVERSE_HEIGHT, "src/assets/ghosts/reverse/reverse_2.rgba2"
    macro_bitmap_make_buffered SPRITE_REVERSE_3, SPRITE_REVERSE_WIDTH, SPRITE_REVERSE_HEIGHT, "src/assets/ghosts/reverse/reverse_3.rgba2"
    macro_bitmap_make_buffered SPRITE_REVERSE_4, SPRITE_REVERSE_WIDTH, SPRITE_REVERSE_HEIGHT, "src/assets/ghosts/reverse/reverse_4.rgba2"
    macro_bitmap_make_buffered SPRITE_REVERSE_5, SPRITE_REVERSE_WIDTH, SPRITE_REVERSE_HEIGHT, "src/assets/ghosts/reverse/reverse_5.rgba2"
    macro_bitmap_make_buffered SPRITE_REVERSE_6, SPRITE_REVERSE_WIDTH, SPRITE_REVERSE_HEIGHT, "src/assets/ghosts/reverse/reverse_6.rgba2"
    macro_bitmap_make_buffered SPRITE_REVERSE_7, SPRITE_REVERSE_WIDTH, SPRITE_REVERSE_HEIGHT, "src/assets/ghosts/reverse/reverse_7.rgba2"

    ; Blinky

    macro_bitmap_make_buffered SPRITE_BLINKY_0, SPRITE_BLINKY_WIDTH, SPRITE_BLINKY_HEIGHT, "src/assets/ghosts/blinky/blinky_0.rgba2"
    macro_bitmap_make_buffered SPRITE_BLINKY_1, SPRITE_BLINKY_WIDTH, SPRITE_BLINKY_HEIGHT, "src/assets/ghosts/blinky/blinky_1.rgba2" 
    macro_bitmap_make_buffered SPRITE_BLINKY_2, SPRITE_BLINKY_WIDTH, SPRITE_BLINKY_HEIGHT, "src/assets/ghosts/blinky/blinky_2.rgba2" 
    macro_bitmap_make_buffered SPRITE_BLINKY_3, SPRITE_BLINKY_WIDTH, SPRITE_BLINKY_HEIGHT, "src/assets/ghosts/blinky/blinky_3.rgba2" 
    macro_bitmap_make_buffered SPRITE_BLINKY_4, SPRITE_BLINKY_WIDTH, SPRITE_BLINKY_HEIGHT, "src/assets/ghosts/blinky/blinky_4.rgba2" 
    macro_bitmap_make_buffered SPRITE_BLINKY_5, SPRITE_BLINKY_WIDTH, SPRITE_BLINKY_HEIGHT, "src/assets/ghosts/blinky/blinky_5.rgba2" 
    macro_bitmap_make_buffered SPRITE_BLINKY_6, SPRITE_BLINKY_WIDTH, SPRITE_BLINKY_HEIGHT, "src/assets/ghosts/blinky/blinky_6.rgba2" 
    macro_bitmap_make_buffered SPRITE_BLINKY_7, SPRITE_BLINKY_WIDTH, SPRITE_BLINKY_HEIGHT, "src/assets/ghosts/blinky/blinky_7.rgba2"

    macro_sprite_select_sprite SPRITE_BLINKY_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_BLINKY_0
    macro_sprite_add_frame SPRITE_BLINKY_1
    macro_sprite_add_frame SPRITE_BLINKY_2
    macro_sprite_add_frame SPRITE_BLINKY_3
    macro_sprite_add_frame SPRITE_BLINKY_4
    macro_sprite_add_frame SPRITE_BLINKY_5
    macro_sprite_add_frame SPRITE_BLINKY_6
    macro_sprite_add_frame SPRITE_BLINKY_7
    ; Frames 8-15: frightened (blue) ghost — shared across all four ghosts.
    macro_sprite_add_frame SPRITE_REVERSE_0
    macro_sprite_add_frame SPRITE_REVERSE_1
    macro_sprite_add_frame SPRITE_REVERSE_2
    macro_sprite_add_frame SPRITE_REVERSE_3
    macro_sprite_add_frame SPRITE_REVERSE_4
    macro_sprite_add_frame SPRITE_REVERSE_5
    macro_sprite_add_frame SPRITE_REVERSE_6
    macro_sprite_add_frame SPRITE_REVERSE_7

    macro_sprite_mark_hardware                  ; #15 — Blinky

    ; Pinky

    macro_bitmap_make_buffered SPRITE_PINKY_0, SPRITE_PINKY_WIDTH, SPRITE_PINKY_HEIGHT, "src/assets/ghosts/pinky/pinky_0.rgba2" 
    macro_bitmap_make_buffered SPRITE_PINKY_1, SPRITE_PINKY_WIDTH, SPRITE_PINKY_HEIGHT, "src/assets/ghosts/pinky/pinky_1.rgba2" 
    macro_bitmap_make_buffered SPRITE_PINKY_2, SPRITE_PINKY_WIDTH, SPRITE_PINKY_HEIGHT, "src/assets/ghosts/pinky/pinky_2.rgba2" 
    macro_bitmap_make_buffered SPRITE_PINKY_3, SPRITE_PINKY_WIDTH, SPRITE_PINKY_HEIGHT, "src/assets/ghosts/pinky/pinky_3.rgba2" 
    macro_bitmap_make_buffered SPRITE_PINKY_4, SPRITE_PINKY_WIDTH, SPRITE_PINKY_HEIGHT, "src/assets/ghosts/pinky/pinky_4.rgba2" 
    macro_bitmap_make_buffered SPRITE_PINKY_5, SPRITE_PINKY_WIDTH, SPRITE_PINKY_HEIGHT, "src/assets/ghosts/pinky/pinky_5.rgba2" 
    macro_bitmap_make_buffered SPRITE_PINKY_6, SPRITE_PINKY_WIDTH, SPRITE_PINKY_HEIGHT, "src/assets/ghosts/pinky/pinky_6.rgba2" 
    macro_bitmap_make_buffered SPRITE_PINKY_7, SPRITE_PINKY_WIDTH, SPRITE_PINKY_HEIGHT, "src/assets/ghosts/pinky/pinky_7.rgba2"

    macro_sprite_select_sprite SPRITE_PINKY_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_PINKY_0
    macro_sprite_add_frame SPRITE_PINKY_1
    macro_sprite_add_frame SPRITE_PINKY_2
    macro_sprite_add_frame SPRITE_PINKY_3
    macro_sprite_add_frame SPRITE_PINKY_4
    macro_sprite_add_frame SPRITE_PINKY_5
    macro_sprite_add_frame SPRITE_PINKY_6
    macro_sprite_add_frame SPRITE_PINKY_7
    macro_sprite_add_frame SPRITE_REVERSE_0
    macro_sprite_add_frame SPRITE_REVERSE_1
    macro_sprite_add_frame SPRITE_REVERSE_2
    macro_sprite_add_frame SPRITE_REVERSE_3
    macro_sprite_add_frame SPRITE_REVERSE_4
    macro_sprite_add_frame SPRITE_REVERSE_5
    macro_sprite_add_frame SPRITE_REVERSE_6
    macro_sprite_add_frame SPRITE_REVERSE_7

    macro_sprite_mark_hardware                  ; #15 — Pinky

    ; Inky

    macro_bitmap_make_buffered SPRITE_INKY_0, SPRITE_INKY_WIDTH, SPRITE_INKY_HEIGHT, "src/assets/ghosts/inky/inky_0.rgba2" 
    macro_bitmap_make_buffered SPRITE_INKY_1, SPRITE_INKY_WIDTH, SPRITE_INKY_HEIGHT, "src/assets/ghosts/inky/inky_1.rgba2" 
    macro_bitmap_make_buffered SPRITE_INKY_2, SPRITE_INKY_WIDTH, SPRITE_INKY_HEIGHT, "src/assets/ghosts/inky/inky_2.rgba2" 
    macro_bitmap_make_buffered SPRITE_INKY_3, SPRITE_INKY_WIDTH, SPRITE_INKY_HEIGHT, "src/assets/ghosts/inky/inky_3.rgba2" 
    macro_bitmap_make_buffered SPRITE_INKY_4, SPRITE_INKY_WIDTH, SPRITE_INKY_HEIGHT, "src/assets/ghosts/inky/inky_4.rgba2" 
    macro_bitmap_make_buffered SPRITE_INKY_5, SPRITE_INKY_WIDTH, SPRITE_INKY_HEIGHT, "src/assets/ghosts/inky/inky_5.rgba2" 
    macro_bitmap_make_buffered SPRITE_INKY_6, SPRITE_INKY_WIDTH, SPRITE_INKY_HEIGHT, "src/assets/ghosts/inky/inky_6.rgba2" 
    macro_bitmap_make_buffered SPRITE_INKY_7, SPRITE_INKY_WIDTH, SPRITE_INKY_HEIGHT, "src/assets/ghosts/inky/inky_7.rgba2"

    macro_sprite_select_sprite SPRITE_INKY_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_INKY_0
    macro_sprite_add_frame SPRITE_INKY_1
    macro_sprite_add_frame SPRITE_INKY_2
    macro_sprite_add_frame SPRITE_INKY_3
    macro_sprite_add_frame SPRITE_INKY_4
    macro_sprite_add_frame SPRITE_INKY_5
    macro_sprite_add_frame SPRITE_INKY_6
    macro_sprite_add_frame SPRITE_INKY_7
    macro_sprite_add_frame SPRITE_REVERSE_0
    macro_sprite_add_frame SPRITE_REVERSE_1
    macro_sprite_add_frame SPRITE_REVERSE_2
    macro_sprite_add_frame SPRITE_REVERSE_3
    macro_sprite_add_frame SPRITE_REVERSE_4
    macro_sprite_add_frame SPRITE_REVERSE_5
    macro_sprite_add_frame SPRITE_REVERSE_6
    macro_sprite_add_frame SPRITE_REVERSE_7

    macro_sprite_mark_hardware                  ; #15 — Inky

    ; Clyde

    macro_bitmap_make_buffered SPRITE_CLYDE_0, SPRITE_CLYDE_WIDTH, SPRITE_CLYDE_HEIGHT, "src/assets/ghosts/clyde/clyde_0.rgba2" 
    macro_bitmap_make_buffered SPRITE_CLYDE_1, SPRITE_CLYDE_WIDTH, SPRITE_CLYDE_HEIGHT, "src/assets/ghosts/clyde/clyde_1.rgba2" 
    macro_bitmap_make_buffered SPRITE_CLYDE_2, SPRITE_CLYDE_WIDTH, SPRITE_CLYDE_HEIGHT, "src/assets/ghosts/clyde/clyde_2.rgba2" 
    macro_bitmap_make_buffered SPRITE_CLYDE_3, SPRITE_CLYDE_WIDTH, SPRITE_CLYDE_HEIGHT, "src/assets/ghosts/clyde/clyde_3.rgba2" 
    macro_bitmap_make_buffered SPRITE_CLYDE_4, SPRITE_CLYDE_WIDTH, SPRITE_CLYDE_HEIGHT, "src/assets/ghosts/clyde/clyde_4.rgba2" 
    macro_bitmap_make_buffered SPRITE_CLYDE_5, SPRITE_CLYDE_WIDTH, SPRITE_CLYDE_HEIGHT, "src/assets/ghosts/clyde/clyde_5.rgba2" 
    macro_bitmap_make_buffered SPRITE_CLYDE_6, SPRITE_CLYDE_WIDTH, SPRITE_CLYDE_HEIGHT, "src/assets/ghosts/clyde/clyde_6.rgba2" 
    macro_bitmap_make_buffered SPRITE_CLYDE_7, SPRITE_CLYDE_WIDTH, SPRITE_CLYDE_HEIGHT, "src/assets/ghosts/clyde/clyde_7.rgba2"

    macro_sprite_select_sprite SPRITE_CLYDE_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_CLYDE_0
    macro_sprite_add_frame SPRITE_CLYDE_1
    macro_sprite_add_frame SPRITE_CLYDE_2
    macro_sprite_add_frame SPRITE_CLYDE_3
    macro_sprite_add_frame SPRITE_CLYDE_4
    macro_sprite_add_frame SPRITE_CLYDE_5
    macro_sprite_add_frame SPRITE_CLYDE_6
    macro_sprite_add_frame SPRITE_CLYDE_7
    macro_sprite_add_frame SPRITE_REVERSE_0
    macro_sprite_add_frame SPRITE_REVERSE_1
    macro_sprite_add_frame SPRITE_REVERSE_2
    macro_sprite_add_frame SPRITE_REVERSE_3
    macro_sprite_add_frame SPRITE_REVERSE_4
    macro_sprite_add_frame SPRITE_REVERSE_5
    macro_sprite_add_frame SPRITE_REVERSE_6
    macro_sprite_add_frame SPRITE_REVERSE_7

    macro_sprite_mark_hardware                  ; #15 — Clyde

    ; Reverse (bitmaps already uploaded above for the ghost-sprite fright
    ; frames; this sprite slot is for the attract-mode display only).

    macro_sprite_select_sprite SPRITE_REVERSE_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_REVERSE_0
    macro_sprite_add_frame SPRITE_REVERSE_1
    macro_sprite_add_frame SPRITE_REVERSE_2
    macro_sprite_add_frame SPRITE_REVERSE_3
    macro_sprite_add_frame SPRITE_REVERSE_4
    macro_sprite_add_frame SPRITE_REVERSE_5
    macro_sprite_add_frame SPRITE_REVERSE_6
    macro_sprite_add_frame SPRITE_REVERSE_7

    macro_sprite_mark_hardware                  ; #15 — REVERSE (attract slot)

    ;macro_sprite_activate 6

    macro_sprite_move SPRITE_REVERSE_ID, 296,90


    ; Cherry
    macro_bitmap_make_buffered SPRITE_CHERRY_0, SPRITE_FRUIT_WIDTH, SPRITE_FRUIT_HEIGHT, "src/assets/fruit/cherry_0.rgba2" 
    
    macro_sprite_select_sprite SPRITE_CHERRY_0_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_CHERRY_0

    macro_sprite_mark_hardware                  ; #15 — cherry

    ;macro_sprite_activate 7

    macro_sprite_move SPRITE_CHERRY_0_ID, 338,320

    ; Cherry
    macro_bitmap_make_buffered SPRITE_BERRY_0, SPRITE_FRUIT_WIDTH, SPRITE_FRUIT_HEIGHT, "src/assets/fruit/berry_0.rgba2" 
    
    macro_sprite_select_sprite SPRITE_BERRY_0_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_BERRY_0

    macro_sprite_mark_hardware                  ; #15 — berry

    ;macro_sprite_activate 8

    macro_sprite_move SPRITE_BERRY_0_ID, 226,150

    ; Orange
    macro_bitmap_make_buffered SPRITE_ORANGE_0, SPRITE_FRUIT_WIDTH, SPRITE_FRUIT_HEIGHT, "src/assets/fruit/orange_0.rgba2" 
    
    macro_sprite_select_sprite SPRITE_ORANGE_0_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_ORANGE_0

    macro_sprite_mark_hardware                  ; #15 — orange

    ;macro_sprite_activate 9

    macro_sprite_move SPRITE_ORANGE_0_ID, 242,150

    ; Apple
    macro_bitmap_make_buffered SPRITE_APPLE_0, SPRITE_FRUIT_WIDTH, SPRITE_FRUIT_HEIGHT, "src/assets/fruit/apple_0.rgba2" 
    
    macro_sprite_select_sprite SPRITE_APPLE_0_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_APPLE_0

    macro_sprite_mark_hardware                  ; #15 — apple

    ;macro_sprite_activate 10

    macro_sprite_move SPRITE_APPLE_0_ID, 258,150

    ; Melon
    macro_bitmap_make_buffered SPRITE_MELON_0, SPRITE_FRUIT_WIDTH, SPRITE_FRUIT_HEIGHT, "src/assets/fruit/melon_0.rgba2" 
    
    macro_sprite_select_sprite SPRITE_MELON_0_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_MELON_0

    macro_sprite_mark_hardware                  ; #15 — melon

    ;macro_sprite_activate 11

    macro_sprite_move SPRITE_MELON_0_ID, 274,150

    ; Galaxian
    macro_bitmap_make_buffered SPRITE_GALAXIAN_0, SPRITE_FRUIT_WIDTH, SPRITE_FRUIT_HEIGHT, "src/assets/fruit/galaxian_0.rgba2" 
    
    macro_sprite_select_sprite SPRITE_GALAXIAN_0_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_GALAXIAN_0

    macro_sprite_mark_hardware                  ; #15 — galaxian

    ;macro_sprite_activate 12

    macro_sprite_move SPRITE_GALAXIAN_0_ID, 290,150

    ; Bell
    macro_bitmap_make_buffered SPRITE_BELL_0, SPRITE_FRUIT_WIDTH, SPRITE_FRUIT_HEIGHT, "src/assets/fruit/bell_0.rgba2" 
    
    macro_sprite_select_sprite SPRITE_BELL_0_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_BELL_0

    macro_sprite_mark_hardware                  ; #15 — bell

    ;macro_sprite_activate 13

    macro_sprite_move SPRITE_BELL_0_ID, 306,150

    ; Key
    macro_bitmap_make_buffered SPRITE_KEY_0, SPRITE_FRUIT_WIDTH, SPRITE_FRUIT_HEIGHT, "src/assets/fruit/key_0.rgba2" 
    
    macro_sprite_select_sprite SPRITE_KEY_0_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_KEY_0

    macro_sprite_mark_hardware                  ; #15 — key

    ;macro_sprite_activate 14

    macro_sprite_move SPRITE_KEY_0_ID, 318,150

    ; Pac-Man Life
    macro_bitmap_make_buffered SPRITE_PAC_MAN_LIFE_0, SPRITE_PAC_MAN_LIFE_WIDTH, SPRITE_PAC_MAN_LIFE_HEIGHT, "src/assets/pac_man/pac_man_life_0.rgba2" 
    
    macro_sprite_select_sprite SPRITE_PAC_MAN_LIFE_ID
    macro_sprite_clear_current

    macro_sprite_add_frame SPRITE_PAC_MAN_LIFE_0

    macro_sprite_mark_hardware                  ; #15 — life icon

    ; Score popup (200/400/800/1600 chain bonus for eating frightened ghosts).
    macro_bitmap_make_buffered SPRITE_SCORE_200,  SPRITE_SCORE_WIDTH, SPRITE_SCORE_HEIGHT, "src/assets/score/score_200.rgba2"
    macro_bitmap_make_buffered SPRITE_SCORE_400,  SPRITE_SCORE_WIDTH, SPRITE_SCORE_HEIGHT, "src/assets/score/score_400.rgba2"
    macro_bitmap_make_buffered SPRITE_SCORE_800,  SPRITE_SCORE_WIDTH, SPRITE_SCORE_HEIGHT, "src/assets/score/score_800.rgba2"
    macro_bitmap_make_buffered SPRITE_SCORE_1600, SPRITE_SCORE_WIDTH, SPRITE_SCORE_HEIGHT, "src/assets/score/score_1600.rgba2"

    macro_sprite_select_sprite SPRITE_SCORE_POPUP_ID
    macro_sprite_clear_current
    macro_sprite_add_frame SPRITE_SCORE_200
    macro_sprite_add_frame SPRITE_SCORE_400
    macro_sprite_add_frame SPRITE_SCORE_800
    macro_sprite_add_frame SPRITE_SCORE_1600

    macro_sprite_mark_hardware                  ; #15 — score popup

    macro_sprite_activate 16

    macro_sprite_move SPRITE_PAC_MAN_LIFE_ID, 334,150

    ; Maze Walls
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_00, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_00.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_01, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_01.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_02, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_02.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_03, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_03.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_04, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_04.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_05, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_05.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_06, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_06.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_07, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_07.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_08, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_08.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_09, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_09.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_10, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_10.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_11, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_11.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_12, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_12.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_13, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_13.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_14, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_14.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_15, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_15.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_16, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_16.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_17, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_17.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_18, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_18.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_19, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_19.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_20, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_20.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_21, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_21.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_22, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_22.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_23, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_23.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_24, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_24.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_25, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_25.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_26, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_26.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_27, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_27.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_28, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_28.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_29, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_29.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_30, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_30.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_31, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_31.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_32, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_32.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_33, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_33.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_34, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_34.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_35, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_35.rgba2"

    ; #40 maze-flash: white-tinted wall tiles. Same shape as the blue
    ; block above; loaded into a parallel set of buffer IDs so
    ; state_level_clear can swap maze_wall_base_bufferId and call
    ; draw_maze for the flash phases without re-uploading anything.
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_00_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_00_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_01_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_01_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_02_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_02_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_03_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_03_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_04_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_04_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_05_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_05_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_06_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_06_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_07_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_07_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_08_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_08_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_09_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_09_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_10_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_10_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_11_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_11_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_12_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_12_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_13_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_13_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_14_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_14_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_15_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_15_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_16_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_16_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_17_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_17_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_18_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_18_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_19_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_19_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_20_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_20_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_21_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_21_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_22_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_22_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_23_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_23_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_24_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_24_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_25_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_25_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_26_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_26_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_27_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_27_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_28_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_28_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_29_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_29_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_30_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_30_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_31_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_31_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_32_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_32_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_33_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_33_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_34_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_34_white.rgba2"
    macro_bitmap_make_buffered SPRITE_MAZE_TILE_35_WHITE, SPRITE_MAZE_TILE_WIDTH, SPRITE_MAZE_TILE_HEIGHT, "src/assets/maze/tile_35_white.rgba2"

    ; Pellets
    macro_bitmap_make_buffered SPRITE_NULL_PELLET_00, SPRITE_PELLET_TILE_WIDTH, SPRITE_PELLET_TILE_HEIGHT, "src/assets/pellet/null_pellet_00.rgba2"
    macro_bitmap_make_buffered SPRITE_PELLET_00, SPRITE_PELLET_TILE_WIDTH, SPRITE_PELLET_TILE_HEIGHT, "src/assets/pellet/pellet_00.rgba2"
    macro_bitmap_make_buffered SPRITE_POWER_PELLET_00, SPRITE_PELLET_TILE_WIDTH, SPRITE_PELLET_TILE_HEIGHT, "src/assets/pellet/power_pellet_00.rgba2"
    macro_bitmap_make_buffered SPRITE_ERASE_PELLET_00, SPRITE_PELLET_TILE_WIDTH, SPRITE_PELLET_TILE_HEIGHT, "src/assets/pellet/erase_pellet_00.rgba2"

    ; Menu Ghosts
    macro_bitmap_make_buffered SPRITE_BLINKY_MENU_00, SPRITE_BLINKY_WIDTH, SPRITE_BLINKY_HEIGHT, "src/assets/ghosts/blinky/blinky_0.rgba2"
    macro_bitmap_make_buffered SPRITE_PINKY_MENU_00, SPRITE_PINKY_WIDTH, SPRITE_PINKY_HEIGHT, "src/assets/ghosts/pinky/pinky_0.rgba2"
    macro_bitmap_make_buffered SPRITE_INKY_MENU_00, SPRITE_INKY_WIDTH, SPRITE_INKY_HEIGHT, "src/assets/ghosts/inky/inky_0.rgba2"
    macro_bitmap_make_buffered SPRITE_CLYDE_MENU_00, SPRITE_CLYDE_WIDTH, SPRITE_CLYDE_HEIGHT, "src/assets/ghosts/clyde/clyde_0.rgba2"

    ; Wide 32x16 Blinky pose for the Act 2 intermission.
    macro_bitmap_make_buffered SPRITE_BLINKY_INTERMISSION_0, SPRITE_BLINKY_INTERMISSION_WIDTH, SPRITE_BLINKY_INTERMISSION_HEIGHT, "src/assets/ghosts/blinky/blinky_intermission_0.rgba2"

vdu_game_data_end: