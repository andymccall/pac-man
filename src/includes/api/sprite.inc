; Bitmap pixel-format constant used by macro_bitmap_make_buffered when
; issuing the "create bitmap from buffer" VDU command (VDU 23, 27, $21).
; Not a sprite type — sprites are composed of bitmap frames added via
; macro_sprite_add_frame after the bitmap has been created.
BITMAP_FORMAT_RGBA2222: equ 1
