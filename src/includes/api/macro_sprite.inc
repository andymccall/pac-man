; Name: macro_sprite_select_sprite
; Description: Select a sprite
; Note: 
; URL:
;
; Inputs: Sprite Number
; Returns: None
; Destroys: None
    macro macro_sprite_select_sprite spriteNumber
    .db 23,27,4
    .db spriteNumber
    endmacro


; Name: macro_sprite_clear_current
; Description: Clear the current sprite
; Note: 
; URL:
;
; Inputs: Sprite Number
; Returns: None
; Destroys: None
    macro macro_sprite_clear_current 
    .db 23,27,5
    endmacro

; Name: macro_sprite_clear_current
; Description: Clear the current sprite
; Note: 
; URL:
;
; Inputs: Sprite Number
; Returns: None
; Destroys: None
   macro macro_sprite_add_frame bitmapNumber
    .db 23,27,$26
    .dw bitmapNumber
    endmacro


; Name: macro_sprite_mark_hardware
; Description: Mark the currently-selected sprite as a hardware sprite
;              (VDU 23, 27, 19). On VDP 2.12.0+ real Agon hardware the
;              sprite then composites at VGA signal time — no tearing,
;              no per-frame VDU 23, 27, 15 refresh needed. Silently
;              ignored by the emulator (which keeps it as a software
;              sprite, still rendered via the per-frame refresh).
;
;              Must be paired with the hardware-sprites test flag
;              (VDU 23, 0, &F8, 2; 1;) issued once at boot AND must run
;              BEFORE the activate command, because the VDP samples the
;              hardware flag at activate time.
;
; Inputs: None (uses currently-selected sprite).
    macro macro_sprite_mark_hardware
    .db 23, 27, 19
    endmacro


; Name: macro_sprite_hide_current
; Description: Hide the current sprite
; Note: 
; URL:
;
; Inputs: None
; Returns: None
; Destroys: None
    macro macro_sprite_hide_current 
    .db 23,27,12
    endmacro


    ; ------------------

    macro macro_sprite_show_current 
    .db 23,27,11
    endmacro


    ; ------------------

    macro SHOW_SPRITE spriteNumber
    .db 23,27,4
    .db spriteNum
    .db 23,27,11
    endmacro

    ; ------------------

    macro HIDE_SPRITE spriteNum
    .db 23,27,4                         ; select sprite...
    .db spriteNum                       ; with ID spriteNum
    .db 23,27,12                        ; show current sprite
    endmacro


    ; ------------------

    macro macro_sprite_activate spriteNumber
    .db 23,27,7                         ; activate
    .db spriteNumber                             ; this many
    .db 23,27,15                        ; update sprites in GPU.
    endmacro


   
; Name: macro_sprite_move
; Description: Move a sprite
; Note: 
; URL:
;
; Inputs: None
; Returns: None
; Destroys: None
    macro macro_sprite_move spriteNumber, x, y
    .db 23, 27, 4, spriteNumber
    .db 23, 27, 13
    .dw x, y
    .db 23, 27, 15
    endmacro