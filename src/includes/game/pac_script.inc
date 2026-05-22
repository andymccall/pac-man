; Pac AI scripted route for the demo (#11, phase 3).
;
; Replays the arcade demo's exact path through our maze. The path was
; transcribed by the user against the maze layout — see
; reference/attract_mode_route_template.md and the four annotated maps
; that follow it.
;
; Each entry is a (col, row, dir) triple — when Pac centres on the tile
; at (col, row), pac_desired_dir is forced to dir and the cursor
; advances. Between waypoints the heuristic AI [[pac_ai.inc]] does
; nothing — Pac just continues in pac_dir, which is the arcade behaviour
; (Pac only re-decides at junctions).
;
; If the cursor runs out (script exhausted) we hand off to the heuristic
; in [[pac_ai.inc]] so Pac doesn't freeze. The demo's fail-safe timer in
; [[state_attract_demo.inc]] catches any unrecoverable stall.

PAC_SCRIPT_LEN: equ 55

pac_script_route:
    ; Kick-off — pac_reset leaves pac_dir = NONE so the heuristic would
    ; normally seed DIR_LEFT here, but the script bypasses the heuristic
    ; while it's active. Encode the initial LEFT explicitly at the
    ; spawn tile so Pac starts moving the moment the READY pause ends.
    db 14, 23, DIR_LEFT
    ; --- Map 1: spawn (14, 23) heading LEFT → E (5, 5) ----------------
    db  9, 23, DIR_DOWN
    db  9, 26, DIR_RIGHT
    db 12, 26, DIR_DOWN
    db 12, 29, DIR_RIGHT
    db 26, 29, DIR_UP
    db 26, 26, DIR_LEFT
    db 24, 26, DIR_UP
    db 24, 23, DIR_RIGHT
    db 26, 23, DIR_UP
    db 26, 20, DIR_LEFT
    db 18, 20, DIR_UP
    db 18, 11, DIR_LEFT
    db  9, 11, DIR_DOWN
    db  9, 14, DIR_LEFT
    db  6, 14, DIR_UP
    db  6,  1, DIR_LEFT
    db  1,  1, DIR_DOWN
    ; (1, 5) → LEFT would walk Pac into the col-0 wall and stall the
    ; script. The user's row-5 markings (D D E S, L L L on row 5 of
    ; map 1) describe a path that has to leave (1, 5) heading RIGHT to
    ; reach the E marker at (5, 5). Treating those L's as a notation
    ; slip for R.
    db  1,  5, DIR_RIGHT
    ; --- Map 2: (4, 5) → loop upper-left → E (3, 5) -------------------
    db 12,  5, DIR_UP
    db 12,  1, DIR_LEFT
    db  1,  1, DIR_DOWN
    db  1,  5, DIR_RIGHT
    ; --- Map 3: (4, 5) → tunnel wrap → E (20, 20) ---------------------
    db  9,  5, DIR_DOWN
    db  9,  8, DIR_RIGHT
    db 12,  8, DIR_DOWN
    db 12, 11, DIR_LEFT
    db  9, 11, DIR_DOWN
    db  9, 14, DIR_LEFT
    ; After the tunnel wrap Pac is heading LEFT on row 14 with the
    ; col-17 wall blocking him at (18, 14). The user marked col 21 of
    ; row 14 with R but the path actually needs DOWN here, walking
    ; straight through (21, 20) until the row-27 wall forces a right
    ; turn at (21, 26).
    db 21, 14, DIR_DOWN
    db 21, 26, DIR_RIGHT
    db 26, 26, DIR_DOWN
    db 26, 29, DIR_LEFT
    db  1, 29, DIR_UP
    db  1, 26, DIR_RIGHT
    db  3, 26, DIR_UP
    db  3, 23, DIR_LEFT
    db  1, 23, DIR_UP
    db  1, 20, DIR_RIGHT
    db 12, 20, DIR_DOWN
    db 12, 23, DIR_RIGHT
    db 15, 23, DIR_UP
    db 15, 20, DIR_RIGHT
    ; --- Map 4: (20, 20) → X (3, 20) — caught by Bashful --------------
    ; Pac continues RIGHT from map 3's (15, 20) into (20, 20) where
    ; map 4 nominally starts, then on to (26, 20) for the first turn.
    db 26, 20, DIR_DOWN
    db 26, 23, DIR_LEFT
    db 24, 23, DIR_DOWN
    db 24, 26, DIR_RIGHT
    db 26, 26, DIR_DOWN
    db 26, 29, DIR_LEFT
    db  1, 29, DIR_UP
    db  1, 26, DIR_RIGHT
    db  3, 26, DIR_UP
    db  3, 23, DIR_LEFT
    db  1, 23, DIR_UP
    db  1, 20, DIR_RIGHT
pac_script_route_end:

; State --------------------------------------------------------------------

; Cursor as a pointer into pac_script_route. 24-bit so the upper byte is
; clean (ADL ld de,(addr) reads 3 bytes).
pac_script_ptr:    dl pac_script_route
pac_script_active: db 1        ; reset to 1 in state_attract_demo_enter

; pac_script_reset — called from state_attract_demo_enter on each new demo.
; Destroys: af, hl
pac_script_reset:
    ld hl, pac_script_route
    ld (pac_script_ptr), hl
    ld a, 1
    ld (pac_script_active), a
    ret

; pac_script_sample — if the cursor's waypoint matches Pac's current
; tile, set pac_desired_dir and advance the cursor.
; Returns A=1 if the script is still running (caller skips heuristic),
;         A=0 once the cursor is exhausted (caller falls back to AI).
; Destroys: af, bc, de, hl
pac_script_sample:
    ld a, (pac_script_active)
    or a
    ret z                               ; already exhausted — caller uses AI

    ; Check cursor against end-of-table.
    ld hl, (pac_script_ptr)
    ld de, pac_script_route_end
    or a
    sbc hl, de
    jr c, @still_active
    ; Cursor past the end — exhaust.
    xor a
    ld (pac_script_active), a
    ret
@still_active:
    ; Read waypoint (col, row, dir) at the cursor.
    ld hl, (pac_script_ptr)
    ld a, (hl)
    ld (pac_script_wp_col), a
    inc hl
    ld a, (hl)
    ld (pac_script_wp_row), a
    inc hl
    ld a, (hl)
    ld (pac_script_wp_dir), a

    ; Get Pac's current tile.
    ld bc, (sprite_x)
    ld de, (sprite_y)
    call sprite_to_tile                 ; carry=1 if off-grid (tunnel mid-warp)
    jr c, @ret_active                   ; mid-tunnel: keep script active, no turn
    ; b=row, c=col

    ld a, (pac_script_wp_col)
    cp c
    jr nz, @ret_active                  ; not the waypoint's column
    ld a, (pac_script_wp_row)
    cp b
    jr nz, @ret_active                  ; not the waypoint's row

    ; Match — apply the turn and advance the cursor by 3 bytes.
    ld a, (pac_script_wp_dir)
    ld (pac_desired_dir), a
    ld hl, (pac_script_ptr)
    inc hl
    inc hl
    inc hl
    ld (pac_script_ptr), hl

@ret_active:
    ld a, 1
    ret

; Scratch for the waypoint comparison above.
pac_script_wp_col: db 0
pac_script_wp_row: db 0
pac_script_wp_dir: db 0
