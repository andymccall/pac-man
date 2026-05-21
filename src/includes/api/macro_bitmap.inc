; Name: macro_bitmap_make_buffered
; Description: Upload an RGBA2 file into a VDP buffer and create a
;              bitmap from it. The bitmap can then be plotted directly
;              via vdu_bitmap_plot, OR added as a sprite frame via
;              macro_sprite_add_frame (see macro_sprite.inc).
; Note:
; URL:
;
; Inputs: bitmapId, width, height, data
; Returns: None
; Destroys: None
    macro macro_bitmap_make_buffered id, width, height, data
    .db     23,0,$A0                    ; write block
    .dw     id                          ; ID (word)
    .db     0                           ; 'write' command
    .dw     width *  height             ; data size assuming RGBA2222           
    incbin data
    .db     23,27,$20                   ; select buffer bitmap to use
    .dw     id                          ; ID (word)
    .db     23,27,$21                   ; create bitmap from buffer
    .dw     width, height               ; width, height (both words)
    .db     BITMAP_FORMAT_RGBA2222         
    endmacro