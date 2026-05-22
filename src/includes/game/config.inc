; engine/config.inc - Persistent settings + high score via PACMAN.CFG.
;
; Single binary file, 16 bytes, modelled on from-the-dead's OPT pattern.
; Loaded once at boot (config_load); written on high-score change and on
; DIP-write paths (only one for now — config_save).
;
; Layout (offsets):
;   0..1   magic 'P','C'
;   2      schema version (0x01)
;   3      cfg_lives_start (DIP — 3 or 5)
;   4..6   cfg_high_score  (24-bit LE)
;   7..15  reserved (zero — future DIPs: bonus threshold, difficulty, ghost names)
;
; The Python tool tools/pacman_cfg.py decodes the file for inspection and
; can write back the DIP fields, but refuses to alter the high score.
;
; MOS file API (rst.lil $08; mos_load / mos_save defined in mos_api.inc):
;   mos_load — HLU = filename (zero-term), DEU = dest, BCU = max size
;              Returns A = file error (0 = OK)
;   mos_save — HLU = filename, DEU = src, BCU = byte count

CFG_SIZE:           equ 16
CFG_MAGIC_0:        equ 'P'
CFG_MAGIC_1:        equ 'C'
CFG_VERSION:        equ $01

CFG_LIVES_DEFAULT:  equ 3

; config_load — read PACMAN.CFG into config_buffer; on miss / bad magic /
; bad version, populate defaults and write a fresh file. Always returns
; with the buffer valid.
; Destroys: af, bc, de, hl
config_load:
    ld hl, cfg_filename
    ld de, config_buffer
    ld bc, CFG_SIZE
    ld a, mos_load
    rst.lil $08
    or a
    jr nz, config_new                   ; non-zero → file error → defaults

    ld a, (cfg_magic + 0)
    cp CFG_MAGIC_0
    jr nz, config_new
    ld a, (cfg_magic + 1)
    cp CFG_MAGIC_1
    jr nz, config_new
    ld a, (cfg_schema_version)
    cp CFG_VERSION
    jr nz, config_new                   ; unknown version → reset
    ret

; config_new — populate defaults and persist.
; Destroys: af, bc, hl
config_new:
    ld a, CFG_MAGIC_0
    ld (cfg_magic + 0), a
    ld a, CFG_MAGIC_1
    ld (cfg_magic + 1), a
    ld a, CFG_VERSION
    ld (cfg_schema_version), a
    ld a, CFG_LIVES_DEFAULT
    ld (cfg_lives_start), a
    ld hl, 0
    ld (cfg_high_score), hl             ; 24-bit LE — 3 bytes

    ; Zero the reserved tail.
    ld hl, cfg_reserved
    ld b, 9
@zloop:
    ld (hl), 0
    inc hl
    djnz @zloop

    jp config_save

; config_save — write config_buffer to disk.
; Destroys: af, bc, de, hl
config_save:
    ld hl, cfg_filename
    ld de, config_buffer
    ld bc, CFG_SIZE
    ld a, mos_save
    rst.lil $08
    ret

; config_high_score_commit — if player_1_score > cfg_high_score, update
; the stored high score and persist. Safe to call repeatedly (no-op when
; not exceeding); intended endpoints are GAME_OVER and the demo-exit
; path that goes via GS_DYING when a demo run racks up points.
; Destroys: af, bc, de, hl
config_high_score_commit:
    ld hl, (player_1_score)             ; 24-bit
    ld de, (cfg_high_score)
    or a
    sbc hl, de
    ret c                               ; player <= high; nothing to do
    ret z

    ld hl, (player_1_score)
    ld (cfg_high_score), hl
    jp config_save

; Storage -----------------------------------------------------------------
;
; The 16-byte buffer is the on-disk image. Field labels alias into it so
; the load/save routines can read/write directly without packing.
cfg_filename:           .db "PACMAN.CFG", 0

config_buffer:
cfg_magic:              .ds 2
cfg_schema_version:     .ds 1
cfg_lives_start:        .ds 1
cfg_high_score:         .ds 3           ; 24-bit LE — matches player_1_score
cfg_reserved:           .ds 9
