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
    include "src/includes/game/font.inc"
    include "src/includes/game/vdu_game_data.inc"
    include "src/includes/game/images_sprites.inc"
    include "src/includes/game/timer.inc"
    include "src/includes/game/maze/maze.inc"
    include "src/includes/game/maze/maze_wall_map.inc"
    include "src/includes/game/maze/maze_pellet_map.inc"
    include "src/includes/game/collision.inc"
    include "src/includes/game/pellet_eat.inc"
    include "src/includes/game/pac_move.inc"
    include "src/includes/game/ghost_ai.inc"
    include "src/includes/game/pac_ai.inc"
    include "src/includes/game/pac_script.inc"
    include "src/includes/game/fruit.inc"
    include "src/includes/game/lives.inc"
    include "src/includes/game/level.inc"
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

; Score Popup Bitmaps
    include "src/includes/game/sprites/score/score.inc"

; Banner Bitmaps (READY!, GAME OVER)
    include "src/includes/game/sprites/banner/banner.inc"

; State machine
    include "src/includes/game/state.inc"
    include "src/includes/game/states/attract_splash.inc"
    include "src/includes/game/states/attract_chars.inc"
    include "src/includes/game/states/attract_hold.inc"
    include "src/includes/game/states/player_select.inc"
    include "src/includes/game/states/ready.inc"
    include "src/includes/game/states/play.inc"
    include "src/includes/game/states/dying.inc"
    include "src/includes/game/states/level_clear.inc"
    include "src/includes/game/states/game_over.inc"
    include "src/includes/game/states/attract_demo.inc"

start:
    macro_stack_push_all

    call vdu_buffer_clear_all

    ld a, VDU_MODE_512x384x64_60HZ
    call vdu_screen_set_mode

    ld a, VDU_SCALING_OFF
    call vdu_screen_set_scaling

    call vdu_cursor_off

    ; Enable hardware sprites on the VDP (no-op on emulator / older VDP
    ; firmware). Must run BEFORE the sprite setup in vdu_game_data, since
    ; the per-sprite hardware-mark flag is sampled at activate time.
    ; #15 — fixes the sprite-plane tearing visible on Pac and ghosts
    ; during gameplay on real Agon Light 2 hardware.
    call vdu_sprite_enable_hardware

    ; Upload + select the arcade font so every subsequent text output
    ; uses it instead of the MOS system font.
    call font_init

    ; Start in the attract splash. (state_attract_splash_enter uploads the
    ; splash bitmaps and renders the small_screen banner.)
    ld a, GS_ATTRACT_SPLASH
    call game_state_set

main_loop:
    call game_timer_tick
    call game_state_tick

    ; ESC always quits, regardless of state
    ld a, mos_getkbmap
    rst.lil $08
    ld a, (ix + $0E)
    bit 0, a
    jp nz, quit

    call vdu_vblank
    call vdu_refresh
    jp main_loop

quit:
    ld hl, VDU_MODE_640x480x4_60HZ
    call vdu_screen_set_mode

    call vdu_cursor_flash

    ld hl, quit_msg
    call vdu_text_print

    macro_stack_pop_all

    ld hl, 0

    ret

; End of game

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

