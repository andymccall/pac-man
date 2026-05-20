; Pac-Man

SPRITE_PAC_MAN_ID: 		equ 0
SPRITE_PAC_MAN_HEIGHT: 	equ 16
SPRITE_PAC_MAN_WIDTH: 	equ 16	
SPRITE_PAC_MAN_00_00: 	equ 256
SPRITE_PAC_MAN_00_01: 	equ 257
SPRITE_PAC_MAN_00_02: 	equ 258
SPRITE_PAC_MAN_01_00: 	equ 259
SPRITE_PAC_MAN_01_01: 	equ 260
SPRITE_PAC_MAN_01_02: 	equ 261
SPRITE_PAC_MAN_02_00: 	equ 262
SPRITE_PAC_MAN_02_01: 	equ 263
SPRITE_PAC_MAN_02_02: 	equ 264
SPRITE_PAC_MAN_03_00: 	equ 265
SPRITE_PAC_MAN_03_01: 	equ 266
SPRITE_PAC_MAN_03_02: 	equ 267

SPRITE_PAC_BIG_00_00:   equ 268
SPRITE_PAC_BIG_00_01:   equ 269
SPRITE_PAC_BIG_00_02:   equ 270

; Death frames live in a free buffer block above the ghost frames (270-308)
; and the menu-ghost block (309-317). They're appended as frames 12-22 of
; SPRITE_PAC_MAN_ID; state_dying uses vdu_sprite_select_frame to cycle.
SPRITE_PAC_DED_00_00:   equ 318
SPRITE_PAC_DED_00_01:   equ 319
SPRITE_PAC_DED_00_02:   equ 320
SPRITE_PAC_DED_00_03:   equ 321
SPRITE_PAC_DED_00_04:   equ 322
SPRITE_PAC_DED_00_05:   equ 323
SPRITE_PAC_DED_00_06:   equ 324
SPRITE_PAC_DED_00_07:   equ 325
SPRITE_PAC_DED_00_08:   equ 326
SPRITE_PAC_DED_00_09:   equ 327
SPRITE_PAC_DED_00_10:   equ 328

; Index of the first death frame within the Pac-Man sprite's frame list.
; Pac-Man has 12 live frames (0..11); death frames are appended after.
PAC_DEATH_FIRST_FRAME:  equ 12
PAC_DEATH_LAST_FRAME:   equ 22