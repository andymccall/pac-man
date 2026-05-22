; Banner bitmaps (READY!, GAME OVER) — pre-rendered from the arcade font
; so they can be plotted at pixel positions instead of 8-px text-grid
; rows. Lets the banner sit cleanly inside maze row 17's open lane.

BANNER_BANNER_H:     equ 8
BANNER_READY_W:      equ 48          ; 6 chars × 8 px
BANNER_GAME_OVER_W:  equ 80          ; 10 chars × 8 px ("GAME  OVER" with
                                     ; a two-space gap — the gap is the
                                     ; arcade's fruit-icon slot, used in
                                     ; later levels' GAME OVER screen)

BANNER_READY:        equ 110
BANNER_READY_ERASE:  equ 111
BANNER_GAME_OVER:    equ 112

; Position of READY! / GAME OVER, expressed RELATIVE to the gfx origin
; in use during state_ready / state_play / state_game_over (which is
; origin_left, origin_top = 138, 60). Pixel y = 204 puts the 8-px-tall
; banner cleanly into maze row 17 — the open lane below the pen.
;
; READY! is 48 px wide; centred on the door column (pixel x = 258) it
; starts at pixel x = 258 - 24 = 234, relative x = 234 - 138 = 96.
;
; GAME  OVER is 80 px wide; centred start at pixel x = 258 - 40 = 218,
; relative x = 218 - 138 = 80.
BANNER_READY_REL_X:      equ 96
BANNER_GAME_OVER_REL_X:  equ 80
BANNER_REL_Y:            equ 144     ; 204 - 60
