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
SPRITE_MAZE_TILE_00_WHITE:      equ 500
SPRITE_MAZE_TILE_01_WHITE:      equ 501
SPRITE_MAZE_TILE_02_WHITE:      equ 502
SPRITE_MAZE_TILE_03_WHITE:      equ 503
SPRITE_MAZE_TILE_04_WHITE:      equ 504
SPRITE_MAZE_TILE_05_WHITE:      equ 505
SPRITE_MAZE_TILE_06_WHITE:      equ 506
SPRITE_MAZE_TILE_07_WHITE:      equ 507
SPRITE_MAZE_TILE_08_WHITE:      equ 508
SPRITE_MAZE_TILE_09_WHITE:      equ 509
SPRITE_MAZE_TILE_10_WHITE:      equ 510
SPRITE_MAZE_TILE_11_WHITE:      equ 511
SPRITE_MAZE_TILE_12_WHITE:      equ 512
SPRITE_MAZE_TILE_13_WHITE:      equ 513
SPRITE_MAZE_TILE_14_WHITE:      equ 514
SPRITE_MAZE_TILE_15_WHITE:      equ 515
SPRITE_MAZE_TILE_16_WHITE:      equ 516
SPRITE_MAZE_TILE_17_WHITE:      equ 517
SPRITE_MAZE_TILE_18_WHITE:      equ 518
SPRITE_MAZE_TILE_19_WHITE:      equ 519
SPRITE_MAZE_TILE_20_WHITE:      equ 520
SPRITE_MAZE_TILE_21_WHITE:      equ 521
SPRITE_MAZE_TILE_22_WHITE:      equ 522
SPRITE_MAZE_TILE_23_WHITE:      equ 523
SPRITE_MAZE_TILE_24_WHITE:      equ 524
SPRITE_MAZE_TILE_25_WHITE:      equ 525
SPRITE_MAZE_TILE_26_WHITE:      equ 526
SPRITE_MAZE_TILE_27_WHITE:      equ 527
SPRITE_MAZE_TILE_28_WHITE:      equ 528
SPRITE_MAZE_TILE_29_WHITE:      equ 529
SPRITE_MAZE_TILE_30_WHITE:      equ 530
SPRITE_MAZE_TILE_31_WHITE:      equ 531
SPRITE_MAZE_TILE_32_WHITE:      equ 532
SPRITE_MAZE_TILE_33_WHITE:      equ 533
SPRITE_MAZE_TILE_34_WHITE:      equ 534
SPRITE_MAZE_TILE_35_WHITE:      equ 535
