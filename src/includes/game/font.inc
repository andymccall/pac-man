; Arcade font — extracted from reference/sprite_sheet.png and uploaded
; to the VDP at boot via the Font API (VDU 23, 0, $A0 / $95).
;
; Layout: 256 glyphs × 8 bytes (8x8 1bpp). Chars 0x30-0x39 (digits),
; 0x41-0x4F (A-O), 0x50-0x5A (P-Z), and 0x21 ('!') are filled in; every
; other slot is zero (blank glyph = transparent). See scripts in
; /tmp/font_extract... or rebuild by re-running the extractor against
; reference/sprite_sheet.png.

FONT_BUFFER_ID:  equ 100           ; from-the-dead uses this; sprite buffers start at 256
FONT_GLYPH_W:    equ 8
FONT_GLYPH_H:    equ 8
FONT_DATA_LEN:   equ 2048

; font_init — upload the arcade font into FONT_BUFFER_ID, create the
; font from it, and select it as the active text font. Call once at
; boot, after the screen mode has been set.
; Destroys: af, bc, hl
font_init:
    ld hl, vdu_font_clear
    ld bc, vdu_font_clear_end - vdu_font_clear
    rst.lil $18

    ld hl, vdu_font_write_hdr
    ld bc, vdu_font_write_hdr_end - vdu_font_write_hdr
    rst.lil $18

    ld hl, font_data
    ld bc, FONT_DATA_LEN
    rst.lil $18

    ; Consolidate — if the 2048-byte send was split into multiple buffer
    ; blocks by the UART layer, createFontFromBuffer will refuse it for
    ; not being singular. Consolidate merges them into one block.
    ld hl, vdu_font_consolidate
    ld bc, vdu_font_consolidate_end - vdu_font_consolidate
    rst.lil $18

    ld hl, vdu_font_create
    ld bc, vdu_font_create_end - vdu_font_create
    rst.lil $18

    ld hl, vdu_font_select
    ld bc, vdu_font_select_end - vdu_font_select
    rst.lil $18
    ret

; VDU 23, 0, $A0, bufferId(word), 2 — clear the buffer.
vdu_font_clear:
    .db 23, 0, $A0
    .dw FONT_BUFFER_ID
    .db 2
vdu_font_clear_end:

; VDU 23, 0, $A0, bufferId(word), 0, length(word) — start a write block.
vdu_font_write_hdr:
    .db 23, 0, $A0
    .dw FONT_BUFFER_ID
    .db 0
    .dw FONT_DATA_LEN
vdu_font_write_hdr_end:

; VDU 23, 0, $A0, bufferId(word), 14 — consolidate blocks into a single
; singular block (required for font sources).
vdu_font_consolidate:
    .db 23, 0, $A0
    .dw FONT_BUFFER_ID
    .db 14
vdu_font_consolidate_end:

; VDU 23, 0, $95, 1, bufferId(word), w, h, ascent, flags — create the
; font from the uploaded buffer. Ascent 7 means glyphs sit on a baseline
; one pixel above the bottom (standard for 8x8).
vdu_font_create:
    .db 23, 0, $95, 1
    .dw FONT_BUFFER_ID
    .db FONT_GLYPH_W, FONT_GLYPH_H, 7, 0
vdu_font_create_end:

; VDU 23, 0, $95, 0, bufferId(word), flags — set as the active font.
vdu_font_select:
    .db 23, 0, $95, 0
    .dw FONT_BUFFER_ID
    .db 0
vdu_font_select_end:

font_data:
    incbin "src/assets/font/arcade.1bpp"
