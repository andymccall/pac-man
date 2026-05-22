; engine/config.inc - Persistent settings + high-score table via PACMAN.CFG.
;
; Single binary file, 80 bytes, schema v3. Modelled on from-the-dead's
; OPT pattern. Loaded once at boot (config_load); written when a run
; qualifies for the top-10 (config_high_score_commit) and on first-boot
; defaults.
;
; Layout (offsets):
;   0..1   magic 'P','C'
;   2      schema version (0x03)
;   3      cfg_lives_start (DIP — 3 or 5)
;   4..63  cfg_score_table — 10 entries × 6 bytes:
;            entry N at offset 4 + N*6:
;              +0..+2  initials (3 ASCII chars)
;              +3..+5  score (24-bit LE)
;          Entries are kept sorted high → low by score; entry 0 is the top.
;   64     cfg_show_score_table (DIP — 0 = off [arcade-faithful default],
;          1 = show the GS_ATTRACT_SCORES screen between DEMO and CHARS).
;          The 1980 cabinet had no top-10 screen; the table is a deliberate
;          non-arcade affordance, off by default.
;   65..79 reserved (zero — future DIPs)
;
; Older schemas (v1 16 bytes, v2 80 bytes) are treated as "stale, reset
; to factory defaults" — see config_load. The tool tools/pacman_cfg.py
; can read the current shape for inspection.
;
; MOS file API (rst.lil $08; mos_load / mos_save defined in mos_api.inc):
;   mos_load — HLU = filename (zero-term), DEU = dest, BCU = max size
;              Returns A = file error (0 = OK)
;   mos_save — HLU = filename, DEU = src, BCU = byte count

CFG_SIZE:           equ 80
CFG_MAGIC_0:        equ 'P'
CFG_MAGIC_1:        equ 'C'
CFG_VERSION:        equ $03

CFG_LIVES_DEFAULT:  equ 3

; Default for the score-table-screen DIP. 0 = off (arcade-faithful 1980
; behaviour, no GS_ATTRACT_SCORES in the attract rotation). 1 = on. The
; Python tool (tools/pacman_cfg.py --show-score-table on) flips it.
CFG_SHOW_TABLE_DEFAULT: equ 0

; Score-table geometry. Kept as named constants so the attract-mode
; renderer and the insert logic agree on layout.
CFG_TABLE_ENTRIES:  equ 10
CFG_TABLE_ENTRY_SZ: equ 6                 ; 3 initials + 3-byte score
CFG_TABLE_BYTES:    equ CFG_TABLE_ENTRY_SZ * CFG_TABLE_ENTRIES   ; 60

; Placeholder initials used when a run qualifies but we haven't shipped
; the initials-entry screen yet (#29 phase 2). The arcade prompts for 3
; letters here — for now everyone is "AGN" (Agon).
CFG_PLACEHOLDER_0:  equ 'A'
CFG_PLACEHOLDER_1:  equ 'G'
CFG_PLACEHOLDER_2:  equ 'N'

; config_load — read PACMAN.CFG into config_buffer; on miss / bad magic /
; wrong version, populate factory defaults and write a fresh file. Always
; returns with the buffer valid.
; Destroys: af, bc, de, hl
config_load:
    ld hl, cfg_filename
    ld de, config_buffer
    ld bc, CFG_SIZE
    ld a, mos_load
    rst.lil $08
    or a
    jr nz, config_new                   ; file error → defaults

    ld a, (cfg_magic + 0)
    cp CFG_MAGIC_0
    jr nz, config_new
    ld a, (cfg_magic + 1)
    cp CFG_MAGIC_1
    jr nz, config_new
    ld a, (cfg_schema_version)
    cp CFG_VERSION
    jr nz, config_new                   ; v1 (or unknown) → reset
    ret

; config_new — populate factory defaults and persist. Score table is
; seeded with the arcade ROM's canonical entries (FIL/SMM/BIL/MLK/MEL ×2
; with the 10000…100 score ladder) so a fresh boot looks like the arcade.
; Destroys: af, bc, de, hl
config_new:
    ld a, CFG_MAGIC_0
    ld (cfg_magic + 0), a
    ld a, CFG_MAGIC_1
    ld (cfg_magic + 1), a
    ld a, CFG_VERSION
    ld (cfg_schema_version), a
    ld a, CFG_LIVES_DEFAULT
    ld (cfg_lives_start), a

    ; Seed the score table from cfg_factory_table.
    ld hl, cfg_factory_table
    ld de, cfg_score_table
    ld bc, CFG_TABLE_BYTES
    ldir

    ; DIP: score-table screen — default to off (arcade-faithful).
    ld a, CFG_SHOW_TABLE_DEFAULT
    ld (cfg_show_score_table), a

    ; Zero the reserved tail (15 bytes — show-table claimed offset 64).
    ld hl, cfg_reserved
    ld b, 15
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

; config_high_score_get — load HL with the current top score (entry 0's
; score, 24-bit LE). Used by the HUD readout so it tracks whatever is at
; the top of the persisted table.
; Destroys: hl
config_high_score_get:
    ld hl, (cfg_score_table + 3)        ; +3 = score field of entry 0
    ret

; config_high_score_commit — if player_1_score qualifies for the top-10
; (i.e. > entry 9's score), insert it at the correct rank with placeholder
; initials, drop the bottom entry, and persist. Idempotent on no-qualify.
;
; Insert pass: walk entries 0..9. For each, compare player_1_score with
; that entry's score. The first slot where player wins is the rank;
; shift entries [rank..8] down one slot to make room, then write the
; new entry at `rank`. Entry 9 (the loser) is overwritten by the shift.
;
; Destroys: af, bc, de, hl, ix
config_high_score_commit:
    ; Quick reject — compare against the *bottom* of the table first.
    ; If player_1_score <= entry 9's score there's nothing to do.
    ld hl, (player_1_score)
    ld de, (cfg_score_table + 9 * CFG_TABLE_ENTRY_SZ + 3)
    or a
    sbc hl, de
    ret c
    ret z

    ; Find rank: scan entries 0..9 for the first whose score is < player_1_score.
    ld ix, cfg_score_table
    ld b, 0                             ; b = candidate rank
@rank_loop:
    push bc
    call @score_gt                      ; CF=1 if player_1_score > (ix+3..ix+5)
    pop bc
    jr c, @found_rank
    ld de, CFG_TABLE_ENTRY_SZ
    add ix, de
    inc b
    ld a, b
    cp CFG_TABLE_ENTRIES
    jr c, @rank_loop
    ret                                 ; safety net (quick reject should have caught this)

@found_rank:
    ; b = rank to insert at (0..9). Shift entries [rank..8] → [rank+1..9].
    ; Walk top-down: copy entry 8→9, 7→8, …, rank→rank+1.
    ; Compute number of shifts = 9 - rank = (CFG_TABLE_ENTRIES - 1) - b.
    ld a, CFG_TABLE_ENTRIES - 1
    sub b
    jr z, @do_write                     ; rank == 9, nothing to shift
    ld c, a                             ; c = shift count

    ; Source = entry 8, dest = entry 9. Walk c times, decrementing.
    ld hl, cfg_score_table + 8 * CFG_TABLE_ENTRY_SZ
    ld de, cfg_score_table + 9 * CFG_TABLE_ENTRY_SZ
@shift_loop:
    push bc
    ld bc, CFG_TABLE_ENTRY_SZ
    push hl
    push de
    ldir                                ; copy CFG_TABLE_ENTRY_SZ bytes
    pop de
    pop hl
    pop bc

    ld a, c
    dec a
    ld c, a
    jr z, @do_write

    ; Step src and dst back by one entry.
    push bc
    ld bc, CFG_TABLE_ENTRY_SZ
    or a
    sbc hl, bc
    ex de, hl
    sbc hl, bc
    ex de, hl
    pop bc
    jr @shift_loop

@do_write:
    ; ix still points at the rank slot from the scan (we never advanced
    ; past it after finding the rank).
    ld (ix + 0), CFG_PLACEHOLDER_0
    ld (ix + 1), CFG_PLACEHOLDER_1
    ld (ix + 2), CFG_PLACEHOLDER_2
    ld hl, (player_1_score)
    ld (ix + 3), l
    ld (ix + 4), h
    ; Top byte of the 24-bit score — fetch via E register.
    ld hl, player_1_score + 2
    ld a, (hl)
    ld (ix + 5), a
    jp config_save

; @score_gt — compare 24-bit (player_1_score) > 24-bit at (ix+3..ix+5).
; Returns CF=1 if player strictly greater, CF=0 otherwise (including
; equal). Used by the rank scan above.
; Destroys: af, hl, de
@score_gt:
    ; Compare high byte first; if different, that decides it.
    ld a, (player_1_score + 2)
    cp (ix + 5)
    jr nz, @hi_diff
    ld a, (player_1_score + 1)
    cp (ix + 4)
    jr nz, @mid_diff
    ld a, (player_1_score + 0)
    cp (ix + 3)
@lo_cmp:
    ; CP sets CF when A < operand. We want CF=1 when player > entry,
    ; which is "A > (ix+...)" — i.e. NOT carry AND NOT zero.
    ret z                               ; equal → CF=0, return
    jr nc, @set_cf                      ; A > operand → set CF
    or a                                ; A < operand → clear CF
    ret
@mid_diff:
    jr @lo_cmp
@hi_diff:
    jr @lo_cmp
@set_cf:
    scf
    ret

; config_post_demo_state — pick the state to enter after the attract-mode
; demo ends. The 1980 arcade goes straight back to the character reveal;
; the optional top-10 table (#29) is a deliberate non-arcade extra, only
; visited when the cfg_show_score_table DIP is on.
;
; Returns: A = GS_ATTRACT_SCORES (DIP on) or GS_ATTRACT_CHARS (DIP off).
; Destroys: af
config_post_demo_state:
    ld a, (cfg_show_score_table)
    or a
    ld a, GS_ATTRACT_CHARS
    ret z
    ld a, GS_ATTRACT_SCORES
    ret

; Storage -----------------------------------------------------------------
;
; The 80-byte buffer is the on-disk image. Field labels alias into it so
; the load/save routines can read/write directly without packing.
cfg_filename:           .db "PACMAN.CFG", 0

config_buffer:
cfg_magic:              .ds 2
cfg_schema_version:     .ds 1
cfg_lives_start:        .ds 1
cfg_score_table:        .ds CFG_TABLE_BYTES        ; 60 bytes (10 × 6)
cfg_show_score_table:   .ds 1                       ; DIP — 0 = off, 1 = on
cfg_reserved:           .ds 15                      ; future DIPs

; Factory-default score table. Top six slots are personalised — credits
; to the folks who helped along the way. Bottom four (the 100..400 pt
; rows) keep the arcade ROM's seed initials so the lower half still
; looks like a Namco/Midway machine. Score ladder is arcade-authentic
; (note the non-uniform jump between rows 5 and 6 — that's how the ROM
; ships it).
;   1  AEM  010000      6  747  000500
;   2  SIJ  009000      7  FIL  000400
;   3  RTB  008000      8  SMM  000300
;   4  SS7  007000      9  BIL  000200
;   5  CLD  006000     10  MLK  000100
cfg_factory_table:
    ; Scores are 24-bit LE; columns are (initials, lo, mid, hi).
    .db "AEM", $10, $27, $00     ; 10000
    .db "SIJ", $28, $23, $00     ;  9000
    .db "RTB", $40, $1F, $00     ;  8000
    .db "SS7", $58, $1B, $00     ;  7000
    .db "CLD", $70, $17, $00     ;  6000
    .db "747", $F4, $01, $00     ;   500
    .db "FIL", $90, $01, $00     ;   400
    .db "SMM", $2C, $01, $00     ;   300
    .db "BIL", $C8, $00, $00     ;   200
    .db "MLK", $64, $00, $00     ;   100
