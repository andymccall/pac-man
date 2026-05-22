; GS_ATTRACT_SCORES — high-score table screen in the attract rotation (#29).
;
; Plays after ATTRACT_DEMO and before looping back to ATTRACT_CHARS. Shows
; the persisted top-10 from PACMAN.CFG (see config.inc) with the arcade's
; per-row colour cycle (red / pink / cyan / orange repeating).
;
; Cycle position:
;   SPLASH → CHARS → HOLD → PLAYER_SELECT (10 s timeout) → DEMO → SCORES
;          ↑                                                       │
;          └───────────────────────────────────────────────────────┘
;
; State entry paints the screen once (HUD, title, 10 rows). State tick
; just counts down the display timer, paints the live HUD high score,
; and polls inputs (C = deposit credit + jump to PLAYER_SELECT; 1 / 2
; with banked credit start a game).
;
; Phase 2 (initials entry on qualifying game-over) will need this state's
; layout constants and the per-row paint helper — both factored so the
; entry screen can highlight one slot in cyan while the rest stay in
; their static colour.

ATTRACT_SCORES_DURATION:   equ 600         ; 10 s at 60 fps

ATTRACT_SCORES_TITLE_ROW:  equ 8           ; "HIGH SCORES" row
ATTRACT_SCORES_TITLE_COL:  equ 27          ; centred for the 11-char title
ATTRACT_SCORES_TABLE_ROW0: equ 11          ; first entry's row
ATTRACT_SCORES_ROW_STEP:   equ 3           ; rows between consecutive entries
ATTRACT_SCORES_TABLE_COL:  equ 24          ; left edge of each row

; Per-row colour cycle: entries 0,4,8 = red; 1,5,9 = pink; 2,6 = cyan;
; 3,7 = orange. Index = entry_index mod 4.
attract_scores_colors:
    .db VDU_COL_BRIGHT_RED            ; Blinky red
    .db 0x3D                          ; Pinky pink
    .db 0x16                          ; Inky cyan
    .db 0x36                          ; Clyde orange

; State -------------------------------------------------------------------

; 3-byte allocation so `ld (state_attract_scores_timer), hl` doesn't spill
; HLU into the next byte — same flavour of bug as #54's chase accumulators.
state_attract_scores_timer: dl ATTRACT_SCORES_DURATION

attract_scores_title_text:
    .db "HIGH SCORES", 0

; Entry -------------------------------------------------------------------

state_attract_scores_enter:
    ; Reset graphics origin to (0, 0). attract_demo / attract_chars leave
    ; it at (origin_left, origin_top); the table renders purely in text,
    ; but anything bitmap-y we add later (e.g. a separator) wants the
    ; absolute origin.
    ld bc, 0
    ld de, 0
    call vdu_set_gfx_origin

    ; CLG (VDU 16) clears the bitmap plane — that wipes the demo's maze
    ; and any leftover text glyphs, since mode 20 renders text as bitmap
    ; plots. Don't ALSO call vdu_screen_text_clear (VDU 12) — that one
    ; has a side effect on this VDP firmware that breaks sprite display
    ; on the next state transition (#29 follow-up: ghosts and Pac come
    ; back invisible in the next demo loop after the table screen).
    call vdu_screen_graphics_clear

    ; Hide every play-state sprite — the demo left Pac/ghosts on screen
    ; until @demo_to_attract hid them, but a defensive sweep keeps this
    ; state self-contained if some future path bypasses that exit.
    call attract_hide_game_sprites

    ; Reset the display-duration timer for this entry.
    ld hl, ATTRACT_SCORES_DURATION
    ld (state_attract_scores_timer), hl

    ; HUD top row — same as every other attract state.
    macro_text_set_color VDU_COL_WHITE
    call display_1up
    call display_high_score
    call display_2up
    call credit_display

    ; "HIGH SCORES" title.
    macro_text_set_color VDU_COL_WHITE
    ld a, 31
    rst.lil $10
    ld a, ATTRACT_SCORES_TITLE_COL
    rst.lil $10
    ld a, ATTRACT_SCORES_TITLE_ROW
    rst.lil $10
    ld hl, attract_scores_title_text
    call vdu_text_print

    ; Paint all 10 entries.
    call attract_scores_paint_table
    ret

; attract_scores_paint_table — iterate the score table and paint one row
; per entry. Factored out of enter so the (future) initials-entry screen
; can repaint a single slot without touching the others.
; Destroys: af, bc, de, hl, ix
attract_scores_paint_table:
    ld ix, cfg_score_table
    ld b, CFG_TABLE_ENTRIES                 ; loop counter
    ld c, 0                                  ; entry index (0..9) — drives colour + rank
    ld d, ATTRACT_SCORES_TABLE_ROW0          ; current text row

@row_loop:
    push bc                                  ; save (b=remaining, c=index)
    push de                                  ; save (d=row)

    ; Pick this row's colour: attract_scores_colors[c & 3].
    ld a, c
    and 3
    ld hl, 0
    ld l, a
    ld de, attract_scores_colors
    add hl, de
    ld a, (hl)
    ; Inline macro_text_set_color with A as the colour byte.
    push af
    ld a, 17
    rst.lil $10
    pop af
    rst.lil $10

    ; VDU 31 cursor: col = TABLE_COL, row = d (saved on stack).
    ld a, 31
    rst.lil $10
    ld a, ATTRACT_SCORES_TABLE_COL
    rst.lil $10
    pop de                                   ; restore d
    push de
    ld a, d
    rst.lil $10

    ; Rank (c+1), 2 chars right-aligned (' 1'..' 9' or '10').
    ld a, c
    inc a
    cp 10
    jr nz, @single_digit
    ld a, '1'
    rst.lil $10
    ld a, '0'
    rst.lil $10
    jr @after_rank
@single_digit:
    push af
    ld a, ' '
    rst.lil $10
    pop af
    add a, '0'
    rst.lil $10
@after_rank:

    ; "  III  " — two spaces, three initials, two spaces.
    ld a, ' '
    rst.lil $10
    ld a, ' '
    rst.lil $10
    ld a, (ix + 0)
    rst.lil $10
    ld a, (ix + 1)
    rst.lil $10
    ld a, (ix + 2)
    rst.lil $10
    ld a, ' '
    rst.lil $10
    ld a, ' '
    rst.lil $10

    ; 6-digit zero-padded score. (ix+3..+5) is 24-bit LE — `ld hl, (ix+3)`
    ; loads all three bytes in ADL mode, matching what score_print wants.
    ld hl, (ix + 3)
    call score_print

    ; Advance: next entry, next row.
    ld de, CFG_TABLE_ENTRY_SZ
    add ix, de
    pop de
    ld a, d
    add a, ATTRACT_SCORES_ROW_STEP
    ld d, a
    pop bc
    inc c
    djnz @row_loop
    ret

; Tick --------------------------------------------------------------------

state_attract_scores_tick:
    ; HUD's live high-score readout keeps repainting so a new top scored
    ; in the previous demo (won't happen today, but free here for when
    ; demo-mode scoring lands) tracks in real time. Other HUD elements
    ; are static once painted by enter.
    macro_text_set_color VDU_COL_WHITE
    call display_high_score

    ; C / 1 / 2 — same shape as attract_hold / attract_demo.
    call @poll_input

    ; Display-duration countdown. On expiry → ATTRACT_CHARS (start of the
    ; next attract loop).
    ld hl, (state_attract_scores_timer)
    dec hl
    ld (state_attract_scores_timer), hl
    ld a, h
    or l
    ret nz
    ld a, GS_ATTRACT_CHARS
    jp game_state_set

@poll_input:
    ld a, mos_getkbmap
    rst.lil $08
    ld a, (ix + $0A)
    bit 2, a
    jr z, @ip_check_start
    call credit_deposit
    ld a, GS_PLAYER_SELECT
    jp game_state_set
@ip_check_start:
    ld a, (credit)
    or a
    ret z
    ld a, (ix + $06)
    bit 0, a
    jp nz, player_start_one_player
    ld a, (credit)
    cp 2
    ret c
    ld a, (ix + $06)
    bit 1, a
    jp nz, player_start_two_player
    ret
