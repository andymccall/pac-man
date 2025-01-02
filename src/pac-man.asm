;
; Title:		       Pac-Man
;
; Description:         A port of the 1980's game Pac-Man to the Agon Light 2
; Author:		       Andy McCall, mailme@bitriot.dev, others welcome!
;
; Created:		       2024-11-25 @ 17:28
; Last Updated:	       2024-11-25 @ 17:28
;
; Modinfo:

.assume adl=1
.org $040000

    jp start

; MOS header
.align 64
.db "MOS",0,1

; API includes
    include "src/includes/system/mos_api.inc"
    include "src/includes/system/macro_stack.inc"
    include "src/includes/api/vdu.inc"
    include "src/includes/api/vdu_screen.inc"
    include "src/includes/api/vdu_cursor.inc"
    include "src/includes/api/vdu_buffer.inc"
    include "src/includes/api/vdu_bitmap.inc"
    include "src/includes/api/vdu_plot.inc"
    include "src/includes/api/vdu_text.inc"
    include "src/includes/api/vdu_color.inc"
    include "src/includes/api/sprite.inc"
    include "src/includes/api/vdu_sprite.inc"
    include "src/includes/api/keyboard.inc"
    include "src/includes/api/macro_sprite.inc"
    include "src/includes/api/macro_text.inc"
    include "src/includes/api/macro_bitmap.inc"

; Splash
    include "src/includes/game/vdu_splash_data.inc"

; Game includes
    include "src/includes/game/globals.inc"
    include "src/includes/game/vdu_game_data.inc"
    include "src/includes/game/images_sprites.inc"
    include "src/includes/game/timer.inc"
    include "src/includes/game/maze/maze.inc"
    include "src/includes/game/maze/maze_wall_map.inc"
    include "src/includes/game/maze/maze_pellet_map.inc"
    include "src/includes/game/splash.inc"
    include "src/includes/game/credit.inc"
    include "src/includes/game/player.inc"
    include "src/includes/game/pause.inc"
    include "src/includes/game/high_menu.inc"

; Character Sprites
    include "src/includes/game/sprites/pac_man/pac_man.inc"
    include "src/includes/game/sprites/pac_man/pac_man_life.inc"
    include "src/includes/game/sprites/ghosts/blinky.inc"
    include "src/includes/game/sprites/ghosts/pinky.inc"
    include "src/includes/game/sprites/ghosts/inky.inc"
    include "src/includes/game/sprites/ghosts/clyde.inc"
    include "src/includes/game/sprites/ghosts/reverse.inc"
    include "src/includes/game/sprites/fruit/fruit.inc"
    include "src/includes/game/sprites/fruit/cherry.inc"
    include "src/includes/game/sprites/fruit/berry.inc"
    include "src/includes/game/sprites/fruit/orange.inc"
    include "src/includes/game/sprites/fruit/apple.inc"
    include "src/includes/game/sprites/fruit/melon.inc"
    include "src/includes/game/sprites/fruit/galaxian.inc"
    include "src/includes/game/sprites/fruit/bell.inc"
    include "src/includes/game/sprites/fruit/key.inc"

; Maze Bitmaps
    include "src/includes/game/sprites/maze/maze_tile.inc"

; Pellet Bitmaps
    include "src/includes/game/sprites/pellet/pellet.inc"
    include "src/includes/game/sprites/pellet/power_pellet.inc"
    include "src/includes/game/sprites/pellet/null_pellet.inc"

animation_counter:   db 10

start:

    macro_stack_push_all

    call vdu_buffer_clear_all

    ld a, VDU_MODE_512x384x64_60HZ
    call vdu_screen_set_mode

    ld a, VDU_SCALING_OFF
    call vdu_screen_set_scaling

    call vdu_cursor_off

    ld hl, vdu_splash_data
    ld bc, vdu_splash_data_end - vdu_splash_data
    rst.lil VDU_OUTPUT_TO_VDP

    ; Display the splash screen
    call small_screen

splash_loop:

    call game_timer_tick

    ; Get a pointer to the keyboard map
    ld a, mos_getkbmap
	rst.lil $08

    ; If the space key is pressed
    ld a, (ix + $0C)    
    bit 2, a
    jp nz, wait_on_credit

    jp splash_loop

wait_on_credit:

    ; Clear the screen
    call vdu_screen_graphics_clear

    ; Load the VDU data for the game sprites and tiles
    ld hl, vdu_game_data
    ld bc, vdu_game_data_end - vdu_game_data
    rst.lil VDU_OUTPUT_TO_VDP

    macro_text_set_color VDU_COL_WHITE
    call display_1up
    call display_high_score
    call display_2up

    ld hl, character_nickname_heading
    call vdu_text_print

    ; Blinky
    ld hl, SPRITE_BLINKY_MENU_00
    call vdu_buffer_select
    ld bc, 155
    ld de, 91
    call vdu_bitmap_plot

    macro_text_set_color VDU_COL_BRIGHT_RED
    ld hl, blinky_name
    call vdu_text_print

    ; Pinky
    ld hl, SPRITE_PINKY_MENU_00
    call vdu_buffer_select
    ld bc, 155
    ld de, 115
    call vdu_bitmap_plot

    macro_text_set_color 0x3D
    ld hl, pinky_name
    call vdu_text_print

    ; Inky
    ld hl, SPRITE_INKY_MENU_00
    call vdu_buffer_select
    ld bc, 155
    ld de, 139
    call vdu_bitmap_plot

    macro_text_set_color 0x16
    ld hl, inky_name
    call vdu_text_print

    ; Clyde
    ld hl, SPRITE_CLYDE_MENU_00
    call vdu_buffer_select
    ld bc, 155
    ld de, 163
    call vdu_bitmap_plot

    macro_text_set_color 0x36
    ld hl, clyde_name
    call vdu_text_print

    ; Animation

    ; Pellets
    ld hl, SPRITE_PELLET_00
    call vdu_buffer_select
    ld bc, 207
    ld de, 247
    call vdu_bitmap_plot

    macro_text_set_color VDU_COL_WHITE
    ld hl, pellet_points
    call vdu_text_print

    ld hl, SPRITE_POWER_PELLET_00
    call vdu_buffer_select
    ld bc, 207
    ld de, 264
    call vdu_bitmap_plot   

    ld hl, power_pellet_points
    call vdu_text_print

    ; Trade and Copyright Message
    ld hl, trade_and_copy_message
    call vdu_text_print

    call credit_display

wait_on_credit_loop:

    ld a, mos_getkbmap
	rst.lil $08

    ; If the Escape key is pressed
    ld a, (ix + $0E)    
    bit 0, a
    jp nz, quit

    ; If the C key is pressed
    ld a, (ix + $0A)
    bit 2, a
    call nz, credit_deposit

    ; If we've had a credit deposited, leave the menu
    ld a, (credit)
    or a
    jp nz, player_select

    jp wait_on_credit_loop

player_select:

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

    ;Clear the screen to get rid of the player select menu
    call vdu_screen_graphics_clear

    ; If we clear the text screen, we need to
    ; reactivate the sprites as they were cleared
    ; in the text screen clear routine
    ;call vdu_screen_text_clear
    ;ld a, SPRITE_COUNT
    ;call vdu_sprite_activate

    ; Placeholders for the precredit menu
    macro_text_set_color VDU_COL_BRIGHT_YELLOW

    ld hl, one_player_message
    call vdu_text_print

player_select_loop:
    
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

    ld a, mos_getkbmap
	rst.lil $08

    ; If the Escape key is pressed
    ld a, (ix + $0E)    
    bit 0, a
    jp nz, quit

    ; If the C key is pressed
    ld a, (ix + $0A)
    bit 2, a
    call nz, credit_deposit

    ; If the 1 key is pressed
    ld a, (ix + $06)
    bit 0, a
    call nz, continue_after_player_select

    ld a, (credit)
    cp 1
    jp z, player_select_loop

two_player:
    ; Placeholders for the precredit menu
    macro_text_set_color VDU_COL_BRIGHT_YELLOW
    ld hl, two_player_message
    call vdu_text_print
    macro_text_set_color VDU_COL_WHITE

    ; If the 2 key is pressed
    ld a, (ix + $06)
    bit 1, a
    call nz, continue_after_player_select
    jp player_select_loop

continue_after_player_select:

    ;Clear the screen to get rid of the player select menu
    call vdu_screen_graphics_clear

    ; If we clear the text screen, we need to
    ; reactivate the sprites as they were cleared
    ; in the text screen clear routine
    ;call vdu_screen_text_clear
    ;ld a, SPRITE_COUNT
    ;call vdu_sprite_activate

    macro_text_set_color VDU_COL_BRIGHT_YELLOW

    ; Print the ready text
    ld hl, ready_txt
    ld bc, ready_txt_end - ready_txt
    rst.lil VDU_OUTPUT_TO_VDP
    macro_text_set_color VDU_COL_WHITE

    ld bc,origin_left+8
    ld de,origin_top+8
    call vdu_set_gfx_origin

    ld ix,maze_wall
    ld hl,SPRITE_MAZE_TILE_00
    call draw_maze
    ld ix,maze_pellets
    ld hl,SPRITE_NULL_PELLET_00
    call draw_maze

    ld bc,origin_left
    ld de,origin_top
    call vdu_set_gfx_origin

    ; Cherry
    ld a, SPRITE_CHERRY_0_ID
    call vdu_sprite_select
    call vdu_sprite_show

    ; Pac-man
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    call vdu_sprite_show

    ; Increament each sprite frame by one for each
    ; ghost so the animation is not in sync

    ; Blinky
    ld a, SPRITE_BLINKY_ID
    call vdu_sprite_select
    ld bc, 250
    ld de, 152
    call vdu_sprite_move_abs
    call vdu_sprite_show

    ; Pinky
    ld a, SPRITE_PINKY_ID
    call vdu_sprite_select
    call vdu_sprite_next_frame
    ld bc, 250
    ld de, 175
    call vdu_sprite_move_abs
    call vdu_sprite_show

    ; Inky
    ld a, SPRITE_INKY_ID
    call vdu_sprite_select
    call vdu_sprite_next_frame
    call vdu_sprite_next_frame
    ld bc, 234
    ld de, 175
    call vdu_sprite_move_abs
    call vdu_sprite_show

    ; Clyde
    ld a, SPRITE_CLYDE_ID
    call vdu_sprite_select
    call vdu_sprite_next_frame
    call vdu_sprite_next_frame
    call vdu_sprite_next_frame
    ld bc, 266
    ld de, 175
    call vdu_sprite_move_abs
    call vdu_sprite_show

game_loop:

    call game_timer_tick
   
    call display_1up
    call display_high_score
    call display_2up
    call credit_display

    ld hl, animation_counter
    dec (hl)
    jr nz, skip_animation 

    ld a, SPRITE_BLINKY_ID
    call vdu_sprite_select
    call vdu_sprite_next_frame

    ld a, SPRITE_PINKY_ID
    call vdu_sprite_select
    call vdu_sprite_next_frame

    ld a, SPRITE_INKY_ID
    call vdu_sprite_select
    call vdu_sprite_next_frame

    ld a, SPRITE_CLYDE_ID
    call vdu_sprite_select
    call vdu_sprite_next_frame

    ; Reset the animation counter
    ld a, 10 ;
    ld hl, animation_counter
    ld (hl), a 

skip_animation:
 
    ld a, mos_getkbmap
	rst.lil $08

; Keypresses
; C = Credit
; 1 = Player 1 Start
; 2 = Player 2 Start
; A = Move Left
; D = Move Right
; W = Move Up
; S = Move Down
; P = Pause

    ; If the Escape key is pressed
    ld a, (ix + $0E)    
    bit 0, a
    jp nz, quit

    ; If the C key is pressed
    ld a, (ix + $0A)
    bit 2, a
    call nz, credit_deposit

    ; If the 1 key is pressed
    ld a, (ix + $06)
    bit 0, a
    call nz, player_1_start

    ; If the 2 key is pressed
    ld a, (ix + $06)
    bit 1, a
    call nz, player_2_start

    ; If the A key is pressed
    ld a, (ix + $08)    
    bit 1, a
    call nz, a_pressed

    ; If the D key is pressed
    ld a, (ix + $06)    
    bit 2, a
    call nz, d_pressed

    ; If the W key is pressed
    ld a, (ix + $04)
    bit 1, a
    call nz, w_pressed

    ; If the S key is pressed
    ld a, (ix + $0A)
    bit 1, a
    call nz, s_pressed

    ; If the P key is pressed
    ld a, (ix + $06)
    bit 7, a
    call nz, pause

    call vdu_vblank

    call vdu_refresh

    jp game_loop

quit:

    ld hl, VDU_MODE_640x480x4_60HZ
    call vdu_screen_set_mode

    call vdu_cursor_flash

    ld hl, quit_msg
    call vdu_text_print

    macro_stack_pop_all

    ld hl,0

    ret

; End of game

precredit_placeholder_message:
    .db "Precredit placeholder menu, insert credit with C",13,10,0

quit_msg:
    .db "Thank you for playing Pac-Man!",13,10,0

high_score_data:
    .db     31, 27, 3
    .db     "HIGH SCORE"
high_score_data_end:

up1_txt:
    .db     31, 20, 3
    .db     "1UP"
up1_txt_end:

up2_txt:
    .db     31, 41, 3
    .db     "2UP"
up2_txt_end:

ready_txt:
    .db     31, 30, 26
    .db     "READY!"
ready_txt_end:

pac_man:        EQU     0
sprite:         EQU     0
sprite_x:
     .dw 250
sprite_y:
     .dw 248

d_pressed:
    ld a, pac_man
    call vdu_sprite_select
    ld a, 0
    call vdu_sprite_select_frame
    
    ld hl, sprite_x    ; Load the address of sprite_x into hl
    ld a, (hl)        ; Load the low byte of sprite_x into a
    inc a             ; Increment the low byte
    ld (hl), a        ; Store the updated low byte
    inc hl             ; Move to the high byte of sprite_x
    ld a, (hl)        ; Load the high byte into a
    adc a, 0          ; Add 0 (with carry from previous inc)
    ld (hl), a        ; Store the updated high byte

    ; Load sprite_x into bc
    ld hl, sprite_x    ; Load the address of sprite_x again
    ld c, (hl)        ; Load the low byte into c
    inc hl             ; Move to the high byte
    ld b, (hl)        ; Load the high byte into b

    ld de,0
    ld de, (sprite_y)

    call vdu_sprite_move_abs
    ret

a_pressed:
    ld a, pac_man
    call vdu_sprite_select
    ld a, 3
    call vdu_sprite_select_frame
    ld a, (sprite_x)
    dec a
    ld (sprite_x), a
    ld bc,0
    ld c,a
    ld de,0
    ld a, (sprite_y)
    ld e, a
    call vdu_sprite_move_abs
    ret

w_pressed:
    ld a, pac_man
    call vdu_sprite_select
    ld a, 7
    call vdu_sprite_select_frame
    ld a, (sprite_x)
    ld bc,0
    ld c,a
    ld de,0
    ld a, (sprite_y)
    dec a
    ld (sprite_y), a
    ld e, a
    call vdu_sprite_move_abs
    ret

s_pressed:
    ld a, pac_man
    call vdu_sprite_select
    ld a, 10
    call vdu_sprite_select_frame
    ld a, (sprite_x)
    ld bc,0
    ld c,a
    ld de,0
    ld a, (sprite_y)
    inc a
    ld (sprite_y), a
    ld e, a
    call vdu_sprite_move_abs
    ret