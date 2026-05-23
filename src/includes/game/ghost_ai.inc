; Ghost AI — all four ghosts, struct-based state.
;
; Each ghost has an 11-byte state block; ghost_tick takes IX = pointer to
; that block so the same code drives all of them. Per-ghost behaviour is
; localised to (a) the chase-target function, dispatched by GS_ID, and
; (b) the scatter corner, stored in the struct.
;
; Level-1 scatter/chase schedule (same as before): SCATTER 7/CHASE 20/
; SCATTER 7/CHASE 20/SCATTER 5/CHASE 20/SCATTER 5/CHASE ∞. Every mode
; transition force-reverses every ghost.
;
; Distance comparisons use SIGNED 8-bit math (jp p / neg) so Pinky's
; "4 tiles ahead of Pac" and Inky's "2 ahead + mirror across Blinky"
; targets — which can land off the maze and produce negative tile
; coordinates — work the way the arcade ROM intends.

; --- Mode constants --------------------------------------------------------

GHOST_MODE_SCATTER: equ 0
GHOST_MODE_CHASE:   equ 1
GHOST_MODE_FRIGHT:  equ 2     ; (unused in v1)
GHOST_MODE_EATEN:   equ 3     ; (unused in v1)

; --- Per-ghost identity ----------------------------------------------------

GHOST_BLINKY: equ 0
GHOST_PINKY:  equ 1
GHOST_INKY:   equ 2
GHOST_CLYDE:  equ 3

; --- Ghost state struct (13 bytes per ghost) -------------------------------

GS_X:          equ 0      ; sprite_x (3 bytes / dl)
GS_Y:          equ 3      ; sprite_y (3 bytes / dl)
GS_DIR:        equ 6      ; current direction
GS_SUBPIXEL:   equ 7      ; speed accumulator
GS_ID:         equ 8      ; GHOST_* identity
GS_SCAT_ROW:   equ 9      ; scatter corner row
GS_SCAT_COL:   equ 10     ; scatter corner col
GS_EATEN:      equ 11     ; 0 = normal/regenerated, 1 = eyes returning to pen
GS_HOUSE:      equ 12     ; GHOST_HOUSE_* — where the ghost is w.r.t. the pen
GS_POST_EATEN: equ 13     ; sticky "ate me this fright cycle, immune from blue"

; --- Ghost house states ----------------------------------------------------

GHOST_HOUSE_OUT:      equ 0   ; running normal AI in the maze
GHOST_HOUSE_INSIDE:   equ 1   ; bobbing in the pen, waiting for release
GHOST_HOUSE_EXITING:  equ 2   ; sliding to door + climbing up through it
GHOST_HOUSE_ENTERING: equ 3   ; eyes descending through the door into the pen

; Target exit point: between cols 13 and 14, row above the door (matches
; Blinky's start position).
HOUSE_EXIT_X:   equ 250
HOUSE_EXIT_Y:   equ 152
HOUSE_FLOOR_Y:  equ 176     ; sprite_y of in-pen ghosts (eyes descend to here)

; In-pen bobbing range — one tile either side of sprite_y=176.
HOUSE_BOB_TOP:    equ 172
HOUSE_BOB_BOTTOM: equ 180

; Eyes-return pathfinding target — the tile centre above the door
; (sprite (254, 152) = tile (row 11, col 14)). Once the eyes' sprite
; reaches that exact pixel position, ghost_tick transitions them to
; GHOST_HOUSE_ENTERING and ghost_eyes_enter takes over for the descent.
EYES_TARGET_ROW: equ 11
EYES_TARGET_COL: equ 14
EYES_ARRIVE_X:   equ 254
EYES_ARRIVE_Y:   equ 152

ghost_speed:        equ 192    ; 75% of Pac's 1 px/frame (arcade level-1)
ghost_speed_fright: equ 96     ; ~37% — fleeing ghosts move much slower
ghost_speed_house:  equ 128    ; ~50% — bobbing/exiting is slower
ghost_speed_eyes:   equ 240    ; ~94% — eyes hustle back to the pen
ghost_speed_tunnel: equ 102    ; ~40% — arcade rule: ghosts slow on the
                               ; tunnel row. Pac is unaffected. Eaten ghosts
                               ; (eyes) also unaffected — they take the
                               ; @speed_eyes branch before this check.
; Fright timing. Total duration is per-level (see fright_duration_table /
; fright_duration_for_level) — the arcade ramps it down across levels and
; turns it off entirely at L17 and L19+. The flash threshold (last "blue/
; white" window before fright ends) stays a flat 2 s at all levels: when
; total fright is shorter than that, ghosts simply flash for the entire
; window, which roughly matches the arcade's reduced flash count at high
; levels (5 flashes at 2 s+, 3 flashes at 1 s).
FRIGHT_FLASH_THRESHOLD: equ 120    ; last 2 s — alternate blue/white

; Per-ghost dot-counter release thresholds. Pinky releases immediately,
; Inky after 30 pellets eaten this level, Clyde after 60. (Arcade has a
; more nuanced per-ghost-counter scheme with a post-death global override
; — deferred; see arcade-fidelity-backlog.)
pinky_release_threshold: equ 0
inky_release_threshold:  equ 30
clyde_release_threshold: equ 60

; Pellets eaten since the start of the level. Driven by pellet_eat;
; consumed by ghost_house_release_tick. Not reset on death — once Inky/
; Clyde are out, they stay out.
pellets_eaten_level: db 0

; --- Shared mode state -----------------------------------------------------

ghost_mode:       db GHOST_MODE_SCATTER
ghost_mode_phase: db 0
ghost_mode_timer: dl 420

; Per-level scatter / chase phase durations (#16). Eight entries per
; level — phases 0..7. Phase 0 is scatter, 1 is chase, alternating.
; Phase 7 has duration 0 which means "infinite" (no further advance).
; All values are frame counts (60 fps).
;
; Three tiers, matching the arcade:
;   level 1     — leisurely; lots of scatter time
;   levels 2-4  — middle tier; phase 5 chase nearly forever, phase 6
;                 scatter shrinks to 1 frame
;   levels 5+   — hardest; same as 2-4 but phases 0 and 2 scatter shrink
;                 from 7s (420f) to 5s (300f)
;
; Values from the Pittman / Birch dossier on the arcade ROM. The "1
; frame" phase-6 scatter on levels 2+ is intentional — it triggers a
; ghosts_all_reverse twitch right before chase ∞.
mode_timings_level_1:
    dw 420, 1200, 420, 1200, 300, 1200, 300, 0
mode_timings_level_2_4:
    dw 420, 1200, 420, 1200, 300, 61980, 1, 0
mode_timings_level_5plus:
    dw 300, 1200, 300, 1200, 300, 62220, 1, 0

; --- Cruise Elroy (#17) ----------------------------------------------------
;
; Late in each level Blinky accelerates and starts chasing Pac even during
; scatter phases. Two tiers:
;
;   Elroy 1 — kicks in when pellets remaining drops to e1_threshold.
;   Elroy 2 — kicks in when pellets remaining drops to e2_threshold; uses
;             the higher of two speeds. Strictly faster than Elroy 1.
;
; Thresholds and speeds are per-level (Pittman / Birch dossier). L1 uses
; 80% / 85%; L2+ uses 90% / 95%. Both speed and "chase during scatter"
; survive across power-pellet fright (fright still slows Blinky and turns
; him blue — Elroy resumes when fright expires).
;
; State lifetime: blinky_elroy_level is reset to 0 by level_init (so each
; level starts with normal Blinky), then bumped 0 → 1 → 2 by elroy_update
; on pellet eats. Never demotes within a level. Arcade's "suspend Elroy
; until Clyde leaves the house after a death" rule is NOT modelled here —
; see arcade-fidelity-backlog if it ever matters.
;
; Storage layout: the four bytes elroy1_threshold..elroy2_speed are
; intentionally contiguous so elroy_load_for_level can ldir them straight
; out of the per-level table.

blinky_elroy_level: db 0     ; 0 = not Elroy; 1 = Elroy 1; 2 = Elroy 2
elroy1_threshold:   db 0     ; pellets remaining trigger for Elroy 1
elroy1_speed:       db 0     ; Q0.8 speed when in Elroy 1
elroy2_threshold:   db 0
elroy2_speed:       db 0

ELROY_SPEED_80: equ 205      ; 80% — L1 Elroy 1
ELROY_SPEED_85: equ 218      ; 85% — L1 Elroy 2
ELROY_SPEED_90: equ 230      ; 90% — L2+ Elroy 1
ELROY_SPEED_95: equ 243      ; 95% — L2+ Elroy 2

; Per-level Elroy table. 19 entries indexed by min(current_level, 19) - 1
; (so L1 = index 0, L19+ = index 18). Each entry is 4 bytes laid out
; (e1_threshold, e1_speed, e2_threshold, e2_speed) — same order as the
; contiguous globals above, so elroy_load_for_level just ldirs them in.
elroy_table:
    .db  20, ELROY_SPEED_80,  10, ELROY_SPEED_85    ; L1
    .db  30, ELROY_SPEED_90,  15, ELROY_SPEED_95    ; L2
    .db  40, ELROY_SPEED_90,  20, ELROY_SPEED_95    ; L3
    .db  40, ELROY_SPEED_90,  20, ELROY_SPEED_95    ; L4
    .db  40, ELROY_SPEED_90,  20, ELROY_SPEED_95    ; L5
    .db  50, ELROY_SPEED_90,  25, ELROY_SPEED_95    ; L6
    .db  50, ELROY_SPEED_90,  25, ELROY_SPEED_95    ; L7
    .db  50, ELROY_SPEED_90,  25, ELROY_SPEED_95    ; L8
    .db  60, ELROY_SPEED_90,  30, ELROY_SPEED_95    ; L9
    .db  60, ELROY_SPEED_90,  30, ELROY_SPEED_95    ; L10
    .db  60, ELROY_SPEED_90,  30, ELROY_SPEED_95    ; L11
    .db  80, ELROY_SPEED_90,  40, ELROY_SPEED_95    ; L12
    .db  80, ELROY_SPEED_90,  40, ELROY_SPEED_95    ; L13
    .db  80, ELROY_SPEED_90,  40, ELROY_SPEED_95    ; L14
    .db 100, ELROY_SPEED_90,  50, ELROY_SPEED_95    ; L15
    .db 100, ELROY_SPEED_90,  50, ELROY_SPEED_95    ; L16
    .db 100, ELROY_SPEED_90,  50, ELROY_SPEED_95    ; L17
    .db 100, ELROY_SPEED_90,  50, ELROY_SPEED_95    ; L18
    .db 120, ELROY_SPEED_90,  60, ELROY_SPEED_95    ; L19+

; elroy_load_for_level — populate the four contiguous Elroy globals from
; elroy_table[min(current_level, 19) - 1], and reset blinky_elroy_level
; to 0. Called from level_init.
; Destroys: af, bc, de, hl
elroy_load_for_level:
    xor a
    ld (blinky_elroy_level), a

    ld a, (current_level)
    or a
    jr nz, @nonzero
    inc a                                ; defensive — level 0 shouldn't happen
@nonzero:
    cp 20
    jr c, @ok
    ld a, 19                             ; clamp at L19+
@ok:
    dec a                                ; → index 0..18
    add a, a
    add a, a                             ; index × 4
    ld hl, 0
    ld l, a
    ld de, elroy_table
    add hl, de                           ; HL = entry pointer
    ld de, elroy1_threshold              ; contiguous (e1_thr, e1_spd, e2_thr, e2_spd)
    ld bc, 4
    ldir
    ret

; elroy_update — re-evaluate Blinky's Elroy tier from pellets remaining.
; Called by pellet_eat after pellets_eaten_level increments. Tiers only
; ever go UP within a level; level_init clears them.
; Destroys: af
elroy_update:
    ld a, (pellets_eaten_level)
    ld b, a
    ld a, MAZE_TOTAL_PELLETS
    sub b                                ; A = pellets remaining

    ; Check Elroy 2 first (stricter threshold).
    ;   Trigger condition: remaining <= threshold.
    ;   After the lines below: A = threshold, B = remaining. CP B sets:
    ;     threshold >  remaining → CF=0, ZF=0 → trigger
    ;     threshold == remaining → CF=0, ZF=1 → trigger
    ;     threshold <  remaining → CF=1       → not yet (skip)
    ld b, a                              ; B = remaining
    ld a, (elroy2_threshold)             ; A = threshold
    cp b
    jr c, @check_e1                      ; threshold < remaining → not E2 yet
    ; threshold >= remaining → trigger E2.
    ld a, 2
    ld (blinky_elroy_level), a
    ret
@check_e1:
    ld a, (elroy1_threshold)
    cp b
    ret c                                ; threshold < remaining → not E1 yet
    ; threshold >= remaining → would trigger E1; only set if we aren't
    ; already at a higher tier (defensive — would only matter if
    ; thresholds were ever reordered).
    ld a, (blinky_elroy_level)
    or a
    ret nz
    ld a, 1
    ld (blinky_elroy_level), a
    ret

; --- Per-level fright duration ---------------------------------------------
;
; Arcade Pac-Man ramps the power-pellet fright window down across levels
; and turns it off entirely at L17 and L19+. Values are durations in
; frames at 60 fps (Pittman / Birch dossier):
;
;   L1:  360 (6 s)   L7:  120 (2 s)   L13:  60 (1 s)   L19+:  0
;   L2:  300 (5 s)   L8:  120 (2 s)   L14: 180 (3 s)
;   L3:  240 (4 s)   L9:   60 (1 s)   L15:  60 (1 s)
;   L4:  180 (3 s)   L10: 300 (5 s)   L16:  60 (1 s)
;   L5:  120 (2 s)   L11: 120 (2 s)   L17:   0
;   L6:  300 (5 s)   L12:  60 (1 s)   L18:  60 (1 s)
;
; A 0 entry means "no fright at all" — Pac still scores the 50 pts for
; the power pellet but ghosts don't turn blue. fright_activate detects
; this and bails before flipping fright_active.
;
; Table is 19 entries (L1..L19+) of two bytes each; lookup clamps any
; level >= 19 onto the L19+ row.
fright_duration_table:
    dw 360            ; L1
    dw 300            ; L2
    dw 240            ; L3
    dw 180            ; L4
    dw 120            ; L5
    dw 300            ; L6
    dw 120            ; L7
    dw 120            ; L8
    dw  60            ; L9
    dw 300            ; L10
    dw 120            ; L11
    dw  60            ; L12
    dw  60            ; L13
    dw 180            ; L14
    dw  60            ; L15
    dw  60            ; L16
    dw   0            ; L17 — no fright
    dw  60            ; L18
    dw   0            ; L19+ — no fright

; fright_duration_for_level — return HL = fright duration in frames for
; current_level. Clamps L>=19 to the L19+ row. Returns 0 when fright is
; disabled for the level (callers should treat 0 as "skip fright").
;
; HLU on return is 0 — the caller writes `ld (fright_timer), hl` (a
; 24-bit ADL store) and we don't want $04 (the upper byte of the data
; segment, picked up by `add hl, de` against the table address) leaking
; into fright_timer's upper byte where it would mis-trigger the
; timer-expiry test on a later tick. Standard ADL upper-byte hygiene.
; Destroys: af, de, hl
fright_duration_for_level:
    ld a, (current_level)
    or a
    jr nz, @nonzero
    inc a                            ; defensive — level 0 shouldn't happen
@nonzero:
    cp 20
    jr c, @ok
    ld a, 19                         ; clamp at L19+
@ok:
    dec a                            ; → index 0..18
    add a, a                         ; index × 2 (dw entries)

    ; Build table entry address in HL. HLU ends up = $04 here (since
    ; fright_duration_table is in the code segment), which is fine
    ; while we use HL purely as a pointer.
    ld hl, 0
    ld l, a
    ld de, fright_duration_table
    add hl, de

    ; Read low + high bytes via E and D, then rebuild HL with HLU = 0.
    ld a, (hl)
    inc hl
    ld d, (hl)
    ld e, a
    ld hl, 0                         ; clears HLU + H + L
    ld h, d
    ld l, e
    ret

; --- Per-ghost state -------------------------------------------------------

blinky_state:
    dl 250                    ; GS_X
    dl 152                    ; GS_Y
    db DIR_LEFT               ; GS_DIR
    db 0                      ; GS_SUBPIXEL
    db GHOST_BLINKY           ; GS_ID
    db 0                      ; GS_SCAT_ROW — top-right corner
    db 25                     ; GS_SCAT_COL
    db 0                      ; GS_EATEN
    db GHOST_HOUSE_OUT        ; GS_HOUSE
    db 0                      ; GS_POST_EATEN

pinky_state:
    dl 250                    ; centred under the door (half-column)
    dl 176
    db DIR_UP
    db 0
    db GHOST_PINKY
    db 0                      ; top-left corner
    db 2
    db 0
    db GHOST_HOUSE_INSIDE
    db 0                      ; GS_POST_EATEN

inky_state:
    dl 234                    ; two tiles left of Pinky (half-column)
    dl 176
    db DIR_UP
    db 0
    db GHOST_INKY
    db 30                     ; bottom-right corner
    db 27
    db 0
    db GHOST_HOUSE_INSIDE
    db 0                      ; GS_POST_EATEN

clyde_state:
    dl 266                    ; two tiles right of Pinky (half-column)
    dl 176
    db DIR_UP
    db 0
    db GHOST_CLYDE
    db 30                     ; bottom-left corner
    db 0
    db 0
    db GHOST_HOUSE_INSIDE
    db 0                      ; GS_POST_EATEN

; --- Fright state ---------------------------------------------------------
;
; fright_timer != 0 means the four ghosts are currently in their "ran from
; Pac, edible" mode. Set by fright_activate when a power pellet is eaten.
; While non-zero, ghost_mode_tick (scatter/chase) is paused, ghosts move at
; ghost_speed_fright, and they choose random directions at tile centres.
;
; fright_eaten_count counts ghosts Pac has eaten during the current fright
; phase, used to compute the 200/400/800/1600 score chain. Resets each
; time fright is re-activated.

; fright_active is a fast 1-byte flag for the per-frame check; fright_timer
; is the actual countdown (24-bit so 6 s = 360 frames fits). They're kept
; in sync by fright_activate and fright_tick.
fright_active:       db 0
fright_timer:        dl 0
fright_eaten_count:  db 0

; Scratch used by ghost_decide_fright to collect valid neighbours before
; picking one at random.
fright_dirs:    db 0, 0, 0, 0
fright_count:   db 0

; --- Score popup ----------------------------------------------------------
; When Pac eats a frightened ghost, ghost_eat shows the chain-bonus number
; (200/400/800/1600) as a sprite at the ghost's position. score_popup_tick
; counts the visible duration down and hides the sprite on expiry.
SCORE_POPUP_DURATION: equ 60       ; ~1 s @ 60 fps
score_popup_timer:    db 0         ; 0 = hidden / inactive

; --- Decision scratch ------------------------------------------------------

bd_cur_row:    db 0
bd_cur_col:    db 0
bd_tgt_row:    db 0
bd_tgt_col:    db 0
bd_rev_dir:    db 0
bd_best_dist:  dl 0
bd_best_dir:   db 0
bd_try_arg:    db 0

; ---------------------------------------------------------------------------
; ghost_ai_init — reset shared mode state and every ghost's state +
; sprite position. Called from state_ready_enter.
; ---------------------------------------------------------------------------
ghost_ai_init:
    xor a
    ld (ghost_mode_phase), a
    ld a, GHOST_MODE_SCATTER
    ld (ghost_mode), a
    xor a                          ; phase 0
    call mode_duration_for_phase   ; HL = level-appropriate scatter time
    ld (ghost_mode_timer), hl

    ; Clear fright state.
    xor a
    ld (fright_active), a
    ld (fright_eaten_count), a
    ld hl, 0
    ld (fright_timer), hl

    ; Pac's per-level speed (#22): defensive reset to normal so a
    ; respawn after a "died during fright" doesn't carry the fright
    ; speed into the next life. No-op if Pac is already at normal.
    call pac_speed_select_normal

    ; Blinky — outside the house, above the door.
    ld ix, blinky_state
    ld bc, 250
    ld (ix + GS_X), bc
    ld de, 152
    ld (ix + GS_Y), de
    ld a, DIR_LEFT
    ld (ix + GS_DIR), a
    xor a
    ld (ix + GS_SUBPIXEL), a
    ld (ix + GS_EATEN), a
    ld (ix + GS_POST_EATEN), a
    ld a, GHOST_HOUSE_OUT
    ld (ix + GS_HOUSE), a
    ld a, SPRITE_BLINKY_ID
    call vdu_sprite_select
    ld bc, 250
    ld de, 152
    call vdu_sprite_move_abs

    ; Pinky — inside the house, centred under the door (half-column).
    ; Bobs vertically while waiting for her release threshold.
    ld ix, pinky_state
    ld bc, 250
    ld (ix + GS_X), bc
    ld de, 176
    ld (ix + GS_Y), de
    ld a, DIR_UP
    ld (ix + GS_DIR), a
    xor a
    ld (ix + GS_SUBPIXEL), a
    ld (ix + GS_EATEN), a
    ld (ix + GS_POST_EATEN), a
    ld a, GHOST_HOUSE_INSIDE
    ld (ix + GS_HOUSE), a
    ld a, SPRITE_PINKY_ID
    call vdu_sprite_select
    ld bc, 250
    ld de, 176
    call vdu_sprite_move_abs

    ; Inky — inside the house, two tiles left of Pinky.
    ld ix, inky_state
    ld bc, 234
    ld (ix + GS_X), bc
    ld de, 176
    ld (ix + GS_Y), de
    ld a, DIR_UP
    ld (ix + GS_DIR), a
    xor a
    ld (ix + GS_SUBPIXEL), a
    ld (ix + GS_EATEN), a
    ld (ix + GS_POST_EATEN), a
    ld a, GHOST_HOUSE_INSIDE
    ld (ix + GS_HOUSE), a
    ld a, SPRITE_INKY_ID
    call vdu_sprite_select
    ld bc, 234
    ld de, 176
    call vdu_sprite_move_abs

    ; Clyde — inside the house, two tiles right of Pinky.
    ld ix, clyde_state
    ld bc, 266
    ld (ix + GS_X), bc
    ld de, 176
    ld (ix + GS_Y), de
    ld a, DIR_UP
    ld (ix + GS_DIR), a
    xor a
    ld (ix + GS_SUBPIXEL), a
    ld (ix + GS_EATEN), a
    ld (ix + GS_POST_EATEN), a
    ld a, GHOST_HOUSE_INSIDE
    ld (ix + GS_HOUSE), a
    ld a, SPRITE_CLYDE_ID
    call vdu_sprite_select
    ld bc, 266
    ld de, 176
    call vdu_sprite_move_abs

    ; Reset the score popup — if Pac died mid-popup, the sprite would
    ; otherwise still be visible and the timer would still tick down.
    xor a
    ld (score_popup_timer), a
    ld a, SPRITE_SCORE_POPUP_ID
    call vdu_sprite_select
    call vdu_sprite_hide

    ; Snap every ghost back to a normal frame. Without this, dying during
    ; fright would carry the blue / white-flash / eyes sprite over into
    ; the next life — fright state is cleared but the sprite frame isn't.
    jp ghosts_animate

; ---------------------------------------------------------------------------
; ghost_ai_tick — mode timer + fright timer + tick every ghost.
; ---------------------------------------------------------------------------
ghost_ai_tick:
    call ghost_mode_tick
    call fright_tick

    ld ix, blinky_state
    call ghost_tick
    ld ix, pinky_state
    call ghost_tick
    ld ix, inky_state
    call ghost_tick
    ld ix, clyde_state
    jp ghost_tick

; ---------------------------------------------------------------------------
; Fright mode
; ---------------------------------------------------------------------------

; fright_activate — called from pellet_eat when Pac eats a power pellet.
; Starts/restarts the fright timer, resets the score chain, swaps every
; non-eaten/non-in-house ghost to its blue frame, and reverses them.
; Eaten ghosts (eyes returning) keep their direction and frame; in-pen
; ghosts keep bobbing.
; Destroys: af
fright_activate:
    ; Per-level fright duration: 0 means "no fright at all on this
    ; level" (arcade behaviour at L17 and L19+). Bail before flipping
    ; any state so Pac just scores the 50-pt power-pellet and ghosts
    ; keep their normal targeting / colour / speed.
    call fright_duration_for_level
    ld a, h
    or l
    ret z

    ld (fright_timer), hl
    ld a, 1
    ld (fright_active), a
    xor a
    ld (fright_eaten_count), a
    ld (state_play_fright_anim_phase), a   ; restart blink at frame 8

    ; Swap Pac to the fright-active speed for this level (#22).
    ; pac_speed_select_normal in fright_tick (on expiry) reverses it;
    ; ghost_ai_init also resets to normal as a defensive measure for
    ; the "died during fright → respawn" path.
    call pac_speed_select_fright

    ; Clear post-eaten flags — a new power pellet makes all surviving
    ; ghosts (including any regenerated from a previous fright) blue again.
    ld (blinky_state + GS_POST_EATEN), a
    ld (pinky_state + GS_POST_EATEN), a
    ld (inky_state + GS_POST_EATEN), a
    ld (clyde_state + GS_POST_EATEN), a

    call ghosts_animate                    ; per-ghost: blue / eyes / pen

    ld ix, blinky_state
    call @reverse_if_not_eaten
    ld ix, pinky_state
    call @reverse_if_not_eaten
    ld ix, inky_state
    call @reverse_if_not_eaten
    ld ix, clyde_state
    jp @reverse_if_not_eaten
@reverse_if_not_eaten:
    ld a, (ix + GS_EATEN)
    or a
    ret nz
    ld a, (ix + GS_HOUSE)
    or a
    ret nz                          ; in-pen ghosts don't reverse on fright
    jp ghost_reverse

; fright_tick — decrement timer; on expiry clear fright_active and
; re-animate. Eyes that are still returning keep their EATEN flag and
; continue toward the pen (ghost_tick's eyes branch is independent of
; fright_active).
; Destroys: af, bc, de, hl
fright_tick:
    ld a, (fright_active)
    or a
    ret z

    ld hl, (fright_timer)
    dec hl
    ld (fright_timer), hl
    ld de, 0
    or a
    sbc hl, de
    ret nz                        ; still ticking

    ; Expired. Clear the active flag, swap Pac back to his normal-mode
    ; speed (#22), and reapply per-ghost frames: non-eaten ghosts swap
    ; back to their normal colours, eaten ghosts stay on their eyes frame.
    xor a
    ld (fright_active), a
    call pac_speed_select_normal
    jp ghosts_animate

; ---------------------------------------------------------------------------
; ghosts_animate — per-ghost sprite-frame selection. Called from
; state_play_tick's animation block AND from fright_activate / fright_tick
; (so frame transitions on fright start/end happen instantly).
;
; For each ghost, ghost_animate_one picks the right frame from
; (GS_EATEN, fright_active, normal/fright anim phase) and applies it via
; vdu_sprite_select + vdu_sprite_select_frame.
; ---------------------------------------------------------------------------
ghosts_animate:
    ld ix, blinky_state
    ld a, 0
    call ghost_animate_one
    ld ix, pinky_state
    ld a, 1
    call ghost_animate_one
    ld ix, inky_state
    ld a, 2
    call ghost_animate_one
    ld ix, clyde_state
    ld a, 3
    jp ghost_animate_one

; ghost_animate_one — pick + apply the right frame for the ghost at IX.
; Inputs: IX = ghost state. (A used to carry a per-ghost stagger before
; #20; now ignored — the direction-specific frame layout means all four
; ghosts can share the same leg phase.)
; Frame priority:
;   1. GS_EATEN set → eyes frame for current direction (12-15)
;   2. fright_active → blue frame (8 or 9 from fright_anim_phase)
;   3. otherwise   → normal frame (dir_base + (anim_phase & 1))
;
; Frame index goes via memory because vdu_sprite_select clobbers BC.
; Destroys: af, bc
ghost_animate_one:
    ld a, (ix + GS_EATEN)
    or a
    jr nz, @eyes

    ld a, (ix + GS_POST_EATEN)
    or a
    jr nz, @normal                      ; eaten this fright — stay normal
    ld a, (fright_active)
    or a
    jr nz, @fright

@normal:
    ; #20 direction-specific frames. The 8 normal frames are laid out as
    ; 2 anim phases × 4 directions: right(0-1) / left(2-3) / up(4-5) /
    ; down(6-7). Frame index = dir_base + (anim_phase & 1).
    ;
    ; The per-ghost stagger we used to apply with the shared 8-frame
    ; cycle is dropped — with direction-specific frames the ghosts face
    ; the way they move and a shared leg-phase looks fine (arcade
    ; behaviour).
    ld a, (ix + GS_DIR)
    cp DIR_LEFT
    jr z, @dir_left
    cp DIR_UP
    jr z, @dir_up
    cp DIR_DOWN
    jr z, @dir_down
    ld b, 0                            ; DIR_RIGHT (or NONE — default right)
    jr @dir_done
@dir_left:
    ld b, 2
    jr @dir_done
@dir_up:
    ld b, 4
    jr @dir_done
@dir_down:
    ld b, 6
@dir_done:
    ld a, (state_play_normal_anim_phase)
    and 1
    add a, b
    jr @apply

@fright:
    ; Phase is 0..3. Low bit = body wiggle (8/9 in blue, 10/11 in white);
    ; high bit = colour. When fright_timer is above the flash threshold
    ; we mask to low bit only so we stay on the blue pair (8/9).
    ld a, (state_play_fright_anim_phase)
    ld b, a
    ld hl, (fright_timer)
    ld de, FRIGHT_FLASH_THRESHOLD
    or a
    sbc hl, de
    jr c, @fright_flash               ; timer < threshold → flashing
    ld a, b
    and 1                             ; only blue wiggle
    add a, 8
    jr @apply
@fright_flash:
    ld a, b
    and 3                             ; full 8..11 cycle (blue A, blue B, white A, white B)
    add a, 8
    jr @apply

@eyes:
    ld a, (ix + GS_DIR)
    call ghost_eyes_frame_for_dir

@apply:
    ld (@frame), a
    ld a, (ix + GS_ID)
    inc a                          ; sprite_id = ghost_id + 1
    call vdu_sprite_select
    ld a, (@frame)
    jp vdu_sprite_select_frame

@frame:   db 0

; ghost_eat — Pac has eaten the ghost at IX. Mark eaten and snap the
; sprite to the eyes frame matching the ghost's current direction (so
; the visual switch from blue→eyes happens immediately, not on the next
; anim cycle ~150 ms later). Award the chain bonus (200/400/800/1600).
ghost_eat:
    ld a, 1
    ld (ix + GS_EATEN), a
    ld (ix + GS_POST_EATEN), a         ; stays set until next fright_activate
    xor a
    ld (ix + GS_SUBPIXEL), a           ; clean accumulator for eyes movement

    ; Snap sprite to eyes frame (so when the freeze ends and we re-show
    ; the ghost, it comes back as eyes facing the last travel direction).
    ld a, (ix + GS_DIR)
    call ghost_eyes_frame_for_dir       ; returns frame index in A
    ld (@eat_frame), a
    ld a, (ix + GS_ID)
    inc a
    call vdu_sprite_select
    ld a, (@eat_frame)
    call vdu_sprite_select_frame

    ; Arcade eaten-ghost freeze. Hide Pac and the eaten ghost so the
    ; score popup is alone on screen, and start the freeze timer that
    ; state_play_tick uses to pause movement / AI / mode + fright timers.
    call vdu_sprite_hide                ; hide eaten ghost (still selected)
    ld a, SPRITE_PAC_MAN_ID
    call vdu_sprite_select
    call vdu_sprite_hide
    ld a, EAT_FREEZE_DURATION
    ld (state_play_eat_freeze_timer), a

    ; points = 200 << fright_eaten_count   (200, 400, 800, 1600)
    ld a, (fright_eaten_count)
    ld hl, 200
    or a
    jr z, @award
@shift:
    add hl, hl
    dec a
    jr nz, @shift
@award:
    ld de, (player_1_score)
    add hl, de
    ld (player_1_score), hl

    ; Show the score popup at the ghost's position. Frame index = current
    ; chain count (0=200, 1=400, 2=800, 3=1600); incremented just below.
    ld a, (fright_eaten_count)
    ld (@popup_frame), a
    ld a, SPRITE_SCORE_POPUP_ID
    call vdu_sprite_select
    ld a, (@popup_frame)
    call vdu_sprite_select_frame
    ld bc, (ix + GS_X)
    ld de, (ix + GS_Y)
    call vdu_sprite_move_abs
    call vdu_sprite_show
    ld a, SCORE_POPUP_DURATION
    ld (score_popup_timer), a

    ld a, (fright_eaten_count)
    inc a
    ld (fright_eaten_count), a
    ret
@eat_frame:   db 0
@popup_frame: db 0

; score_popup_tick — decrement popup timer; hide sprite on expiry.
; Called once per frame from ghost_ai_tick.
score_popup_tick:
    ld a, (score_popup_timer)
    or a
    ret z
    dec a
    ld (score_popup_timer), a
    ret nz
    ld a, SPRITE_SCORE_POPUP_ID
    call vdu_sprite_select
    jp vdu_sprite_hide

; ghost_eyes_frame_for_dir — A = direction (DIR_*); returns the eyes
; sprite frame index in A (12=right, 13=left, 14=up, 15=down).
ghost_eyes_frame_for_dir:
    cp DIR_LEFT
    jr nz, @nl
    ld a, 13
    ret
@nl:
    cp DIR_UP
    jr nz, @nu
    ld a, 14
    ret
@nu:
    cp DIR_DOWN
    jr nz, @nd
    ld a, 15
    ret
@nd:
    ld a, 12
    ret

; ---------------------------------------------------------------------------
; Helpers
; ---------------------------------------------------------------------------

; opposite_dir — A = direction; returns opposite in A.
opposite_dir:
    cp DIR_UP
    jr nz, @nu
    ld a, DIR_DOWN
    ret
@nu:
    cp DIR_DOWN
    jr nz, @nd
    ld a, DIR_UP
    ret
@nd:
    cp DIR_LEFT
    jr nz, @nl
    ld a, DIR_RIGHT
    ret
@nl:
    ld a, DIR_LEFT
    ret

; ghost_reverse — flip the direction of the ghost at IX.
ghost_reverse:
    ld a, (ix + GS_DIR)
    call opposite_dir
    ld (ix + GS_DIR), a
    ret

; ghost_reverse_if_out — same as ghost_reverse but only acts when the
; ghost is HOUSE_OUT. In-pen and exiting ghosts ignore mode-change
; reversals (they're not running scatter/chase).
ghost_reverse_if_out:
    ld a, (ix + GS_HOUSE)
    or a
    ret nz
    jp ghost_reverse

; ghosts_all_reverse — apply the arcade "all ghosts turn around" effect
; on a scatter↔chase mode transition. In-pen ghosts are skipped.
ghosts_all_reverse:
    ld ix, blinky_state
    call ghost_reverse_if_out
    ld ix, pinky_state
    call ghost_reverse_if_out
    ld ix, inky_state
    call ghost_reverse_if_out
    ld ix, clyde_state
    jp ghost_reverse_if_out

; ---------------------------------------------------------------------------
; ghost_mode_tick — decrement mode timer; on expiry advance phase and
; (if the mode actually changed) reverse all ghosts.
; ---------------------------------------------------------------------------
ghost_mode_tick:
    ; Scatter/chase is paused while fright is active.
    ld a, (fright_active)
    or a
    ret nz

    ld hl, (ghost_mode_timer)
    ld de, 0
    or a
    sbc hl, de
    ret z                         ; timer 0 = infinite phase

    dec hl
    ld (ghost_mode_timer), hl
    ld de, 0
    or a
    sbc hl, de
    ret nz                        ; still ticking

    ; Phase expired — advance.
    ld a, (ghost_mode_phase)
    cp 7
    ret nc                        ; already at last phase
    inc a
    ld (ghost_mode_phase), a

    ; New mode = phase & 1 (phase 0=scatter, 1=chase, 2=scatter, ...).
    ld c, a                       ; save phase
    and 1                         ; A = new mode (0=SCATTER, 1=CHASE)
    ld b, a                       ; B = new mode

    ; Look up the new phase's duration from the per-level table.
    ld a, c
    call mode_duration_for_phase
    ld (ghost_mode_timer), hl

    ; Apply the new mode and reverse if it changed (it always changes
    ; in our alternating 8-phase setup, but the guard is harmless).
    ld a, (ghost_mode)
    cp b
    ret z                         ; same mode — no reversal
    ld a, b
    ld (ghost_mode), a
    jp ghosts_all_reverse

; mode_duration_for_phase — return the duration (HL, 24-bit clean) of
; the given scatter/chase phase for the current level.
; Inputs:  A = phase (0..7)
; Outputs: HL = duration in frames (0 means "infinite")
; Destroys: af, bc, de, ix
mode_duration_for_phase:
    push af
    call mode_timings_table_for_ix    ; IX = table base
    pop af
    add a, a                          ; offset = phase * 2 (dw entries)
    ld d, 0
    ld e, a
    add ix, de                        ; IX = entry
    ld hl, 0                          ; clear HLU so subsequent 24-bit
                                      ; arithmetic on HL is well-defined
    ld l, (ix+0)
    ld h, (ix+1)
    ret

; mode_timings_table_for_ix — pick the right per-level table.
; Inputs:  current_level (1..)
; Outputs: IX = base of the table for this level
; Destroys: af
mode_timings_table_for_ix:
    ld a, (current_level)
    cp 2
    jr c, @level_1                    ; current_level == 1
    cp 5
    jr c, @level_2_4                  ; 2..4
    ld ix, mode_timings_level_5plus
    ret
@level_1:
    ld ix, mode_timings_level_1
    ret
@level_2_4:
    ld ix, mode_timings_level_2_4
    ret

; ---------------------------------------------------------------------------
; ghost_target — fill (bd_tgt_row, bd_tgt_col) for the ghost at IX.
;
; Eaten ghosts (eyes returning to the pen) override mode/scatter/chase
; targets and aim for the tile above the door.
; ---------------------------------------------------------------------------
ghost_target:
    ld a, (ix + GS_EATEN)
    or a
    jp nz, ghost_eyes_target

    ld a, (ghost_mode)
    cp GHOST_MODE_SCATTER
    jr nz, @chase_target

    ; Scatter mode — but Cruise Elroy Blinky (#17) keeps chasing.
    ld a, (ix + GS_ID)
    cp GHOST_BLINKY
    jp nz, ghost_scatter_target
    ld a, (blinky_elroy_level)
    or a
    jp z, ghost_scatter_target           ; Blinky not in Elroy yet
    jp blinky_chase_target               ; Elroy Blinky chases in scatter

@chase_target:
    ld a, (ix + GS_ID)
    cp GHOST_BLINKY
    jp z, blinky_chase_target
    cp GHOST_PINKY
    jp z, pinky_chase_target
    cp GHOST_INKY
    jp z, inky_chase_target
    jp clyde_chase_target

; ghost_eyes_target — fixed target above the pen door.
ghost_eyes_target:
    ld a, EYES_TARGET_ROW
    ld (bd_tgt_row), a
    ld a, EYES_TARGET_COL
    ld (bd_tgt_col), a
    ret

; ghost_scatter_target — use the scatter corner from the struct.
ghost_scatter_target:
    ld a, (ix + GS_SCAT_ROW)
    ld (bd_tgt_row), a
    ld a, (ix + GS_SCAT_COL)
    ld (bd_tgt_col), a
    ret

; blinky_chase_target — Pac's current tile.
blinky_chase_target:
    ld bc, (sprite_x)
    ld de, (sprite_y)
    call sprite_to_tile
    jr c, ghost_scatter_target
    ld a, b
    ld (bd_tgt_row), a
    ld a, c
    ld (bd_tgt_col), a
    ret

; pinky_chase_target — 4 tiles ahead of Pac in Pac's direction. Faithfully
; reproduces the arcade up-direction overflow bug: when Pac faces UP the
; target is computed as 4-up AND 4-left, not just 4-up.
pinky_chase_target:
    ld bc, (sprite_x)
    ld de, (sprite_y)
    call sprite_to_tile
    jr c, ghost_scatter_target
    ld a, (pac_dir)
    cp DIR_RIGHT
    jr nz, @nr
    ld a, c
    add a, 4
    ld c, a
    jr @save
@nr:
    cp DIR_LEFT
    jr nz, @nl
    ld a, c
    sub 4
    ld c, a
    jr @save
@nl:
    cp DIR_DOWN
    jr nz, @nu
    ld a, b
    add a, 4
    ld b, a
    jr @save
@nu:
    ; DIR_UP — overflow bug: row -= 4 AND col -= 4
    ld a, b
    sub 4
    ld b, a
    ld a, c
    sub 4
    ld c, a
@save:
    ld a, b
    ld (bd_tgt_row), a
    ld a, c
    ld (bd_tgt_col), a
    ret

; inky_chase_target — pivot = 2 tiles ahead of Pac (same UP-overflow bug
; as Pinky's 4-ahead, since they share the same arcade routine), then
; target = pivot mirrored across Blinky's tile (i.e. 2*pivot - blinky).
inky_chase_target:
    ld bc, (sprite_x)
    ld de, (sprite_y)
    call sprite_to_tile
    jp c, ghost_scatter_target
    ld a, (pac_dir)
    cp DIR_RIGHT
    jr nz, @nr
    ld a, c
    add a, 2
    ld c, a
    jr @reflect
@nr:
    cp DIR_LEFT
    jr nz, @nl
    ld a, c
    sub 2
    ld c, a
    jr @reflect
@nl:
    cp DIR_DOWN
    jr nz, @nu
    ld a, b
    add a, 2
    ld b, a
    jr @reflect
@nu:
    ; DIR_UP — same overflow bug as Pinky
    ld a, b
    sub 2
    ld b, a
    ld a, c
    sub 2
    ld c, a
@reflect:
    ; bc = (pivot_row, pivot_col). Reflect Blinky's tile.
    push bc                       ; save pivot
    ld bc, (blinky_state + GS_X)
    ld de, (blinky_state + GS_Y)
    call sprite_to_tile
    pop hl                        ; H = pivot_row, L = pivot_col
    jp c, ghost_scatter_target

    ; target_row = 2*pivot_row - blinky_row
    ld a, h
    add a, a
    sub b
    ld (bd_tgt_row), a
    ; target_col = 2*pivot_col - blinky_col
    ld a, l
    add a, a
    sub c
    ld (bd_tgt_col), a
    ret

; clyde_chase_target — if distance² to Pac > 64 tiles² (Euclidean) then
; target = Pac's tile, otherwise scatter to his corner.
clyde_chase_target:
    ld bc, (sprite_x)
    ld de, (sprite_y)
    call sprite_to_tile
    jp c, ghost_scatter_target
    ld a, b
    ld (bd_tgt_row), a            ; tentatively target Pac
    ld a, c
    ld (bd_tgt_col), a

    push bc                       ; save Pac's tile
    ld bc, (ix + GS_X)
    ld de, (ix + GS_Y)
    call sprite_to_tile
    pop hl                        ; H = pac_row, L = pac_col
    jp c, ghost_scatter_target

    ld a, l
    sub c
    jp p, @dx_pos
    neg
@dx_pos:
    ld e, a
    ld a, h
    sub b
    jp p, @dy_pos
    neg
@dy_pos:
    ld d, a

    ld bc, 0
    ld b, e
    ld c, e
    mlt bc
    push bc
    ld bc, 0
    ld b, d
    ld c, d
    mlt bc
    pop hl
    add hl, bc                    ; HL = dist²

    ld de, 64
    or a
    sbc hl, de
    jp z, ghost_scatter_target    ; == 64 → arcade treats as "close" → scatter
    ret nc                        ; > 64 → keep targeting Pac

    jp ghost_scatter_target       ; < 64 → close → override to scatter corner

; ---------------------------------------------------------------------------
; ghost_at_no_up_tile — Z=1 iff (bd_cur_row, bd_cur_col) is one of the
; four arcade no-up tiles: (11, 12), (11, 15), (23, 12), (23, 15).
; Caller must have populated bd_cur_row/col (ghost_decide does this
; before any direction tries).
; Destroys: af
; ---------------------------------------------------------------------------
NO_UP_ROW_UPPER: equ 11           ; intersection row directly above the pen
NO_UP_ROW_LOWER: equ 23           ; mirrored intersection row in the lower maze
NO_UP_COL_LEFT:  equ 12           ; left flank of the pen / lower mirror
NO_UP_COL_RIGHT: equ 15           ; right flank

ghost_at_no_up_tile:
    ld a, (bd_cur_row)
    cp NO_UP_ROW_UPPER
    jr z, @check_col
    cp NO_UP_ROW_LOWER
    jr z, @check_col
    or 1                          ; not on a no-up row → Z=0
    ret
@check_col:
    ld a, (bd_cur_col)
    cp NO_UP_COL_LEFT
    ret z
    cp NO_UP_COL_RIGHT
    ret

; ---------------------------------------------------------------------------
; ghost_at_tile_center — Z=1 iff the ghost at IX is on a tile centre.
; ---------------------------------------------------------------------------
ghost_at_tile_center:
    ld a, (ix + GS_X)
    and 7
    cp 6
    ret nz
    ld a, (ix + GS_Y)
    and 7
    ret

; ---------------------------------------------------------------------------
; ghost_decide — at a tile centre, pick the legal neighbour closest to
; target. Updates (ix + GS_DIR). No reverse allowed (except as a
; last-resort dead-end fallback).
; ---------------------------------------------------------------------------
ghost_decide:
    ld bc, (ix + GS_X)
    ld de, (ix + GS_Y)
    call sprite_to_tile
    ret c
    ld a, b
    ld (bd_cur_row), a
    ld a, c
    ld (bd_cur_col), a

    call ghost_target

    ld a, (ix + GS_DIR)
    call opposite_dir
    ld (bd_rev_dir), a

    ld hl, $FFFFFF
    ld (bd_best_dist), hl
    xor a
    ld (bd_best_dir), a

    ; Arcade "no-up" tiles. Four specific intersections above and below
    ; the pen where ghost AI is forbidden from picking DIR_UP — used by
    ; skilled players to deny ghosts an escape route. Eaten ghosts (eyes
    ; returning to the pen) ignore the rule so they can still take the
    ; direct route home through these tiles.
    ld a, (ix + GS_EATEN)
    or a
    jr nz, @try_up                ; eyes — no-up rule does not apply
    call ghost_at_no_up_tile
    jr z, @skip_up
@try_up:
    ld a, DIR_UP
    call @try_dir
@skip_up:
    ld a, DIR_LEFT
    call @try_dir
    ld a, DIR_DOWN
    call @try_dir
    ld a, DIR_RIGHT
    call @try_dir

    ld a, (bd_best_dir)
    or a
    jr nz, @set_dir
    ld a, (bd_rev_dir)            ; all blocked → forced reverse
@set_dir:
    ld (ix + GS_DIR), a
    ret

@try_dir:
    ld (bd_try_arg), a
    ld b, a
    ld a, (bd_rev_dir)
    cp b
    ret z

    ld a, (bd_cur_row)
    ld d, a
    ld a, (bd_cur_col)
    ld e, a

    ld a, b
    cp DIR_UP
    jr nz, @t1
    ld a, d
    or a
    ret z
    dec d
    jr @check_wall
@t1:
    cp DIR_DOWN
    jr nz, @t2
    ld a, d
    cp maze_play_rows - 1
    ret nc
    inc d
    jr @check_wall
@t2:
    cp DIR_LEFT
    jr nz, @t3
    ld a, e
    or a
    ret z
    dec e
    jr @check_wall
@t3:
    ; DIR_RIGHT
    ld a, e
    cp maze_play_cols - 1
    ret nc
    inc e

@check_wall:
    ld b, d
    ld c, e
    push bc
    call tile_row_col_to_offset
    ld de, maze_wall_map
    add hl, de
    ld a, (hl)
    pop bc
    cp maze_wall_floor
    ret nz

    ; Signed |dx| and |dy|. jp p (SF check) works for two's-complement
    ; target coordinates produced by Pinky/Inky's up-overflow bug.
    ld a, (bd_tgt_col)
    sub c
    jp p, @dx_pos
    neg
@dx_pos:
    ld e, a
    ld a, (bd_tgt_row)
    sub b
    jp p, @dy_pos
    neg
@dy_pos:
    ld d, a

    ld bc, 0
    ld b, e
    ld c, e
    mlt bc
    push bc
    ld bc, 0
    ld b, d
    ld c, d
    mlt bc
    pop hl
    add hl, bc

    push hl
    ld de, (bd_best_dist)
    or a
    sbc hl, de
    pop hl
    ret nc

    ld (bd_best_dist), hl
    ld a, (bd_try_arg)
    ld (bd_best_dir), a
    ret

; ---------------------------------------------------------------------------
; ghost_decide_fright — at a tile centre during fright mode, pick a random
; legal (non-reverse, walkable) neighbour direction. The "random" source
; is the system frame counter mixed with the ghost ID so the four ghosts
; don't pick in lock-step.
;
; Sets (ix + GS_DIR). If no valid neighbour exists (dead end), forces a
; reverse.
; ---------------------------------------------------------------------------
ghost_decide_fright:
    ld bc, (ix + GS_X)
    ld de, (ix + GS_Y)
    call sprite_to_tile
    ret c
    ld a, b
    ld (bd_cur_row), a
    ld a, c
    ld (bd_cur_col), a

    ld a, (ix + GS_DIR)
    call opposite_dir
    ld (bd_rev_dir), a

    xor a
    ld (fright_count), a

    ld a, DIR_UP
    call @check_and_add
    ld a, DIR_LEFT
    call @check_and_add
    ld a, DIR_DOWN
    call @check_and_add
    ld a, DIR_RIGHT
    call @check_and_add

    ld a, (fright_count)
    or a
    jr nz, @pick

    ; Dead end — forced reverse.
    ld a, (bd_rev_dir)
    ld (ix + GS_DIR), a
    ret

@pick:
    ld b, a                       ; b = count
    ld a, (timestamp_now)
    add a, (ix + GS_ID)
@mod:
    cp b
    jr c, @mod_done
    sub b
    jr @mod
@mod_done:
    ld de, 0
    ld e, a
    ld hl, fright_dirs
    add hl, de
    ld a, (hl)
    ld (ix + GS_DIR), a
    ret

@check_and_add:
    ld (bd_try_arg), a
    ld b, a
    ld a, (bd_rev_dir)
    cp b
    ret z                         ; reverse, skip

    ld a, (bd_cur_row)
    ld d, a
    ld a, (bd_cur_col)
    ld e, a

    ld a, b
    cp DIR_UP
    jr nz, @t1
    ld a, d
    or a
    ret z
    dec d
    jr @check_wall
@t1:
    cp DIR_DOWN
    jr nz, @t2
    ld a, d
    cp maze_play_rows - 1
    ret nc
    inc d
    jr @check_wall
@t2:
    cp DIR_LEFT
    jr nz, @t3
    ld a, e
    or a
    ret z
    dec e
    jr @check_wall
@t3:
    ld a, e
    cp maze_play_cols - 1
    ret nc
    inc e

@check_wall:
    ld b, d
    ld c, e
    push bc
    call tile_row_col_to_offset
    ld de, maze_wall_map
    add hl, de
    ld a, (hl)
    pop bc
    cp maze_wall_floor
    ret nz                        ; wall, skip

    ; Append to fright_dirs[fright_count++].
    ld de, 0
    ld a, (fright_count)
    ld e, a
    ld hl, fright_dirs
    add hl, de
    ld a, (bd_try_arg)
    ld (hl), a
    ld a, e
    inc a
    ld (fright_count), a
    ret

; ---------------------------------------------------------------------------
; Ghost-house mechanics
;
; The ghost pen holds Pinky/Inky/Clyde at level start. They bob vertically
; until released, then slide horizontally to align with the door
; (HOUSE_EXIT_X, between cols 13 and 14) and climb up through it to
; (HOUSE_EXIT_X, HOUSE_EXIT_Y). The release order is governed by a simple
; per-ghost pellet threshold (Pinky=0, Inky=30, Clyde=60). See
; arcade-fidelity-backlog for the more accurate dot-counter scheme.
; ---------------------------------------------------------------------------

; ghost_threshold_for_id — A = ghost ID; returns pellet-release threshold
; in A. Blinky has no in-pen state so isn't expected; falls through to
; Pinky's threshold defensively.
ghost_threshold_for_id:
    cp GHOST_INKY
    jr z, @inky
    cp GHOST_CLYDE
    jr z, @clyde
    ld a, pinky_release_threshold
    ret
@inky:
    ld a, inky_release_threshold
    ret
@clyde:
    ld a, clyde_release_threshold
    ret

; ghost_apply_sprite — push GS_X/GS_Y to the ghost's VDU sprite.
; sprite_id = ghost_id + 1.
ghost_apply_sprite:
    ld a, (ix + GS_ID)
    inc a
    call vdu_sprite_select
    ld bc, (ix + GS_X)
    ld de, (ix + GS_Y)
    jp vdu_sprite_move_abs

; ghost_house_tick — ghost is bobbing inside the pen. Check release
; threshold; if reached, transition to EXITING (and start exit movement
; this same tick). Otherwise bob one pixel up/down within the bob bounds,
; reversing GS_DIR at each bound.
ghost_house_tick:
    ld a, (ix + GS_ID)
    call ghost_threshold_for_id
    ld b, a
    ld a, (pellets_eaten_level)
    cp b
    jr c, @bob                        ; not yet — keep bobbing

    ld a, GHOST_HOUSE_EXITING
    ld (ix + GS_HOUSE), a
    jp ghost_exit

@bob:
    ld a, (ix + GS_DIR)
    cp DIR_UP
    jr z, @bob_up

    ; DIR_DOWN (or anything else — default to down).
    ld de, (ix + GS_Y)
    inc de
    ld (ix + GS_Y), de
    ld hl, HOUSE_BOB_BOTTOM
    or a
    sbc hl, de
    jr nz, @bob_apply
    ld a, DIR_UP
    ld (ix + GS_DIR), a
    jr @bob_apply

@bob_up:
    ld de, (ix + GS_Y)
    dec de
    ld (ix + GS_Y), de
    ld hl, HOUSE_BOB_TOP
    or a
    sbc hl, de
    jr nz, @bob_apply
    ld a, DIR_DOWN
    ld (ix + GS_DIR), a

@bob_apply:
    jp ghost_apply_sprite

; ghost_exit — slide horizontally to HOUSE_EXIT_X then climb up to
; HOUSE_EXIT_Y. When arrived, snap to exit point and transition to
; HOUSE_OUT facing left (matches Blinky's start direction). Caller
; ensures we've gated on the speed accumulator already.
ghost_exit:
    ld hl, (ix + GS_X)
    ld de, HOUSE_EXIT_X
    or a
    sbc hl, de
    jr z, @x_aligned
    jr c, @slide_right                ; GS_X < target

    ; GS_X > target — slide left.
    ld hl, (ix + GS_X)
    dec hl
    ld (ix + GS_X), hl
    ld a, DIR_LEFT
    ld (ix + GS_DIR), a
    jp ghost_apply_sprite

@slide_right:
    ld hl, (ix + GS_X)
    inc hl
    ld (ix + GS_X), hl
    ld a, DIR_RIGHT
    ld (ix + GS_DIR), a
    jp ghost_apply_sprite

@x_aligned:
    ld hl, (ix + GS_Y)
    ld de, HOUSE_EXIT_Y
    or a
    sbc hl, de
    jr z, @arrived
    jr c, @arrived                    ; already above — bail out

    ld hl, (ix + GS_Y)
    dec hl
    ld (ix + GS_Y), hl
    ld a, DIR_UP
    ld (ix + GS_DIR), a
    jp ghost_apply_sprite

@arrived:
    ld hl, HOUSE_EXIT_X
    ld (ix + GS_X), hl
    ld hl, HOUSE_EXIT_Y
    ld (ix + GS_Y), hl
    ld a, GHOST_HOUSE_OUT
    ld (ix + GS_HOUSE), a
    ld a, DIR_LEFT
    ld (ix + GS_DIR), a
    xor a
    ld (ix + GS_SUBPIXEL), a
    jp ghost_apply_sprite

; ghost_eyes_enter — eaten ghost is descending through the gate into
; the pen. Slide horizontally to HOUSE_EXIT_X first (eyes always arrive
; via a tile centre, so they're either at sprite_x 246 or 254 — 4 px
; off the door column), then descend straight down to HOUSE_FLOOR_Y.
; On arrival: clear GS_EATEN, set HOUSE_EXITING, face up — the regular
; ghost_exit flow takes over from the next tick and climbs back out.
ghost_eyes_enter:
    ld hl, (ix + GS_X)
    ld de, HOUSE_EXIT_X
    or a
    sbc hl, de
    jr z, @x_aligned
    jr c, @slide_right

    ld hl, (ix + GS_X)
    dec hl
    ld (ix + GS_X), hl
    ld a, DIR_LEFT
    ld (ix + GS_DIR), a
    jp ghost_apply_sprite

@slide_right:
    ld hl, (ix + GS_X)
    inc hl
    ld (ix + GS_X), hl
    ld a, DIR_RIGHT
    ld (ix + GS_DIR), a
    jp ghost_apply_sprite

@x_aligned:
    ld hl, (ix + GS_Y)
    ld de, HOUSE_FLOOR_Y
    or a
    sbc hl, de
    jr nc, @arrived

    ld hl, (ix + GS_Y)
    inc hl
    ld (ix + GS_Y), hl
    ld a, DIR_DOWN
    ld (ix + GS_DIR), a
    jp ghost_apply_sprite

@arrived:
    xor a
    ld (ix + GS_EATEN), a
    ld (ix + GS_SUBPIXEL), a
    ld a, GHOST_HOUSE_EXITING
    ld (ix + GS_HOUSE), a
    ld a, DIR_UP
    ld (ix + GS_DIR), a
    jp ghost_apply_sprite

; ---------------------------------------------------------------------------
; ghost_tick — speed accumulator; on overflow, dispatch by EATEN flag
; first, then by GS_HOUSE.
;
;   GS_EATEN=1:
;     HOUSE_OUT     → pathfind to (EYES_TARGET_ROW, EYES_TARGET_COL);
;                     on reaching sprite (EYES_ARRIVE_X, EYES_ARRIVE_Y)
;                     transition to ENTERING
;     HOUSE_ENTERING → ghost_eyes_enter (slide to door + descend)
;
;   GS_EATEN=0:
;     HOUSE_OUT      → normal AI (decide at tile centre, then step)
;     HOUSE_INSIDE   → bob in pen (check release threshold first)
;     HOUSE_EXITING  → ghost_exit (slide to door + climb up)
;
; Speeds: ghost_speed (normal), ghost_speed_fright (in fright),
; ghost_speed_house (bobbing/exiting), ghost_speed_eyes (eyes return).
; ---------------------------------------------------------------------------
ghost_tick:
    ; Speed selection.
    ld a, (ix + GS_EATEN)
    or a
    jr nz, @speed_eyes

    ld a, (ix + GS_HOUSE)
    or a
    jr nz, @speed_house

    ld b, ghost_speed
    ld a, (ix + GS_POST_EATEN)
    or a
    jr nz, @house_out_tunnel_check      ; immune from fright slow-down
    ld a, (fright_active)
    or a
    jr z, @house_out_tunnel_check
    ld b, ghost_speed_fright
@house_out_tunnel_check:
    ; Cruise Elroy speed boost (#17). Applies only when B is still
    ; ghost_speed (i.e. fright hasn't taken over), and only to Blinky
    ; when his elroy tier > 0. Elroy speeds are strictly faster than
    ; ghost_speed, so this never accidentally slows him.
    ld a, b
    cp ghost_speed
    jr nz, @no_elroy
    ld a, (ix + GS_ID)
    cp GHOST_BLINKY
    jr nz, @no_elroy
    ld a, (blinky_elroy_level)
    or a
    jr z, @no_elroy
    cp 2
    jr z, @apply_elroy2
    ld a, (elroy1_speed)
    ld b, a
    jr @no_elroy
@apply_elroy2:
    ld a, (elroy2_speed)
    ld b, a
@no_elroy:
    ; Tunnel slow-down: HOUSE_OUT ghosts on the tunnel row drop to ~40%.
    ; If current speed is already <= tunnel speed (e.g. fright at 37%),
    ; we don't speed them up — the slower speed wins.
    ld a, ghost_speed_tunnel
    cp b
    jr nc, @speed_ready                 ; tunnel >= B, no clamp needed
    ld de, (ix + GS_Y)
    call pac_on_tunnel_row
    jr nz, @speed_ready                 ; not on the tunnel row
    ld b, ghost_speed_tunnel
    jr @speed_ready
@speed_house:
    ld b, ghost_speed_house
    jr @speed_ready
@speed_eyes:
    ld b, ghost_speed_eyes
@speed_ready:
    ld a, (ix + GS_SUBPIXEL)
    add a, b
    ld (ix + GS_SUBPIXEL), a
    ret nc

    ; Dispatch.
    ld a, (ix + GS_EATEN)
    or a
    jp nz, @eyes_movement

    ld a, (ix + GS_HOUSE)
    cp GHOST_HOUSE_INSIDE
    jp z, ghost_house_tick
    cp GHOST_HOUSE_EXITING
    jp z, ghost_exit

    ; HOUSE_OUT — normal AI.
    call ghost_at_tile_center
    jr nz, @step                  ; not at centre, just step

    ld a, (ix + GS_POST_EATEN)
    or a
    jr nz, @normal_decide               ; immune — use chase/scatter target
    ld a, (fright_active)
    or a
    jr z, @normal_decide
    call ghost_decide_fright
    jr @step
@normal_decide:
    call ghost_decide
@step:
    jp ghost_step

@eyes_movement:
    ; Eyes-returning ghost. Dispatch on house state.
    ld a, (ix + GS_HOUSE)
    cp GHOST_HOUSE_ENTERING
    jp z, ghost_eyes_enter

    ; HOUSE_OUT: pathfind toward the tile above the door.
    ; If we've reached the arrival pixel, switch to ENTERING and let
    ; ghost_eyes_enter do this tick's move (the 4-px slide to the door).
    ld hl, (ix + GS_X)
    ld de, EYES_ARRIVE_X
    or a
    sbc hl, de
    jr nz, @eyes_pathfind
    ld hl, (ix + GS_Y)
    ld de, EYES_ARRIVE_Y
    or a
    sbc hl, de
    jr nz, @eyes_pathfind

    ld a, GHOST_HOUSE_ENTERING
    ld (ix + GS_HOUSE), a
    ld a, DIR_DOWN
    ld (ix + GS_DIR), a
    jp ghost_eyes_enter

@eyes_pathfind:
    call ghost_at_tile_center
    jr nz, @eyes_step
    call ghost_decide                  ; uses eyes target (via ghost_target)
@eyes_step:
    jp ghost_step

; ---------------------------------------------------------------------------
; ghost_step — move one pixel in the ghost's direction, handling tunnel
; wrap. Selects the appropriate sprite by ghost ID (sprite_id = id + 1)
; before issuing the move_abs.
; ---------------------------------------------------------------------------
ghost_step:
    ld bc, (ix + GS_X)
    ld de, (ix + GS_Y)
    ld a, (ix + GS_DIR)

    cp DIR_UP
    jr z, @up
    cp DIR_DOWN
    jr z, @down
    cp DIR_LEFT
    jr z, @left
    ; DIR_RIGHT (or NONE)
    ld hl, pac_right_edge_sprite_x
    or a
    sbc hl, bc
    jr nz, @inc
    push bc
    push de
    call pac_on_tunnel_row
    pop de
    pop bc
    jr nz, @inc
    ld bc, pac_left_edge_sprite_x
    jr @save
@inc:
    inc bc
    jr @save

@left:
    ld hl, pac_left_edge_sprite_x
    or a
    sbc hl, bc
    jr nz, @dec
    push bc
    push de
    call pac_on_tunnel_row
    pop de
    pop bc
    jr nz, @dec
    ld bc, pac_right_edge_sprite_x
    jr @save
@dec:
    dec bc
    jr @save

@up:
    dec de
    jr @save

@down:
    inc de

@save:
    ld (ix + GS_X), bc
    ld (ix + GS_Y), de

    ld a, (ix + GS_ID)
    inc a                          ; sprite_id = ghost_id + 1
    call vdu_sprite_select
    ld bc, (ix + GS_X)
    ld de, (ix + GS_Y)
    jp vdu_sprite_move_abs

; ---------------------------------------------------------------------------
; pac_ghost_collision — handle Pac/ghost interactions for the current
; frame. Returns Z=1 only if Pac actually DIED (caller transitions to
; DYING). Three cases per ghost:
;
;   GS_EATEN set                 → skip entirely (ghost is just eyes /
;                                  hidden after being eaten this fright).
;   tile shared, fright active   → Pac eats the ghost (score chain,
;                                  sprite hidden). Return Z=0.
;   tile shared, no fright       → Pac dies. Return Z=1.
;
; If multiple ghosts share Pac's tile in the same frame, we eat
; the first frightened one we hit; a non-frightened touch ends Pac.
; ---------------------------------------------------------------------------
pac_ghost_collision:
    ld ix, blinky_state
    call pac_vs_one_ghost
    ret z
    ld ix, pinky_state
    call pac_vs_one_ghost
    ret z
    ld ix, inky_state
    call pac_vs_one_ghost
    ret z
    ld ix, clyde_state
    jp pac_vs_one_ghost

; pac_vs_one_ghost — Z=1 iff Pac dies from contact with the ghost at IX.
pac_vs_one_ghost:
    ld a, (ix + GS_EATEN)
    or a
    jr nz, @no                    ; eaten ghost has no collision

    ld bc, (sprite_x)
    ld de, (sprite_y)
    call sprite_to_tile
    jr c, @no
    push bc
    ld bc, (ix + GS_X)
    ld de, (ix + GS_Y)
    call sprite_to_tile
    pop hl                        ; H=pac_row, L=pac_col
    jr c, @no
    ld a, b
    cp h
    jr nz, @no
    ld a, c
    cp l
    jr nz, @no

    ; Same tile. Can Pac eat this ghost? Only if it's currently blue —
    ; fright is on AND the ghost hasn't already been eaten this cycle.
    ld a, (ix + GS_POST_EATEN)
    or a
    jr nz, @kill                  ; already eaten this fright → kills Pac
    ld a, (fright_active)
    or a
    jr nz, @eat

@kill:
    ; Pac dies.
    cp a                          ; Z=1
    ret

@eat:
    call ghost_eat
    or 1                          ; Z=0 (no death)
    ret
@no:
    or 1                          ; Z=0
    ret
