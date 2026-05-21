; Pac-Man Life

SPRITE_MAZE_TILE_HEIGHT: 	    equ 8
SPRITE_MAZE_TILE_WIDTH: 	    equ 8

SPRITE_MAZE_TILE_00:            equ 400
SPRITE_MAZE_TILE_01:            equ 401
SPRITE_MAZE_TILE_02:            equ 402
SPRITE_MAZE_TILE_03:            equ 403
SPRITE_MAZE_TILE_04:            equ 404
SPRITE_MAZE_TILE_05:            equ 405
SPRITE_MAZE_TILE_06:            equ 406
SPRITE_MAZE_TILE_07:            equ 407
SPRITE_MAZE_TILE_08:            equ 408
SPRITE_MAZE_TILE_09:            equ 409
SPRITE_MAZE_TILE_10:            equ 410
SPRITE_MAZE_TILE_11:            equ 411
SPRITE_MAZE_TILE_12:            equ 412
SPRITE_MAZE_TILE_13:            equ 413
SPRITE_MAZE_TILE_14:            equ 414
SPRITE_MAZE_TILE_15:            equ 415
SPRITE_MAZE_TILE_16:            equ 416
SPRITE_MAZE_TILE_17:            equ 417
SPRITE_MAZE_TILE_18:            equ 418
SPRITE_MAZE_TILE_19:            equ 419
SPRITE_MAZE_TILE_20:            equ 420
SPRITE_MAZE_TILE_21:            equ 421
SPRITE_MAZE_TILE_22:            equ 422
SPRITE_MAZE_TILE_23:            equ 423
SPRITE_MAZE_TILE_24:            equ 424
SPRITE_MAZE_TILE_25:            equ 425
SPRITE_MAZE_TILE_26:            equ 426
SPRITE_MAZE_TILE_27:            equ 427
SPRITE_MAZE_TILE_28:            equ 428
SPRITE_MAZE_TILE_29:            equ 429
SPRITE_MAZE_TILE_30:            equ 430
SPRITE_MAZE_TILE_31:            equ 431
SPRITE_MAZE_TILE_32:            equ 432
SPRITE_MAZE_TILE_33:            equ 433
SPRITE_MAZE_TILE_34:            equ 434
SPRITE_MAZE_TILE_35:            equ 435    ; ghost-pen gate (pink horizontal line)

; --- #40 maze-flash: white-tinted variants ---------------------------------
; Same shape as the blue wall tiles but with every non-zero RGBA2 byte
; flattened to 0xFF (= R=3, G=3, B=3, A=3, full white opaque). The block
; sits at the same offset spacing as the blue block (tile N white =
; SPRITE_MAZE_TILE_00_WHITE + N) so state_level_clear can swap the
; maze_wall_base_bufferId and call draw_maze unchanged.
SPRITE_MAZE_TILE_00_WHITE:      equ 700
SPRITE_MAZE_TILE_01_WHITE:      equ 701
SPRITE_MAZE_TILE_02_WHITE:      equ 702
SPRITE_MAZE_TILE_03_WHITE:      equ 703
SPRITE_MAZE_TILE_04_WHITE:      equ 704
SPRITE_MAZE_TILE_05_WHITE:      equ 705
SPRITE_MAZE_TILE_06_WHITE:      equ 706
SPRITE_MAZE_TILE_07_WHITE:      equ 707
SPRITE_MAZE_TILE_08_WHITE:      equ 708
SPRITE_MAZE_TILE_09_WHITE:      equ 709
SPRITE_MAZE_TILE_10_WHITE:      equ 710
SPRITE_MAZE_TILE_11_WHITE:      equ 711
SPRITE_MAZE_TILE_12_WHITE:      equ 712
SPRITE_MAZE_TILE_13_WHITE:      equ 713
SPRITE_MAZE_TILE_14_WHITE:      equ 714
SPRITE_MAZE_TILE_15_WHITE:      equ 715
SPRITE_MAZE_TILE_16_WHITE:      equ 716
SPRITE_MAZE_TILE_17_WHITE:      equ 717
SPRITE_MAZE_TILE_18_WHITE:      equ 718
SPRITE_MAZE_TILE_19_WHITE:      equ 719
SPRITE_MAZE_TILE_20_WHITE:      equ 720
SPRITE_MAZE_TILE_21_WHITE:      equ 721
SPRITE_MAZE_TILE_22_WHITE:      equ 722
SPRITE_MAZE_TILE_23_WHITE:      equ 723
SPRITE_MAZE_TILE_24_WHITE:      equ 724
SPRITE_MAZE_TILE_25_WHITE:      equ 725
SPRITE_MAZE_TILE_26_WHITE:      equ 726
SPRITE_MAZE_TILE_27_WHITE:      equ 727
SPRITE_MAZE_TILE_28_WHITE:      equ 728
SPRITE_MAZE_TILE_29_WHITE:      equ 729
SPRITE_MAZE_TILE_30_WHITE:      equ 730
SPRITE_MAZE_TILE_31_WHITE:      equ 731
SPRITE_MAZE_TILE_32_WHITE:      equ 732
SPRITE_MAZE_TILE_33_WHITE:      equ 733
SPRITE_MAZE_TILE_34_WHITE:      equ 734
SPRITE_MAZE_TILE_35_WHITE:      equ 735
