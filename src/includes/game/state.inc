; Game state machine.
;
; Each state has an `enter` (called once on transition) and a `tick`
; (called every frame while in that state). State transitions are made by
; calling game_state_set with the new state id in A; the enter is then
; invoked on the next dispatch.
;
; If a state's enter triggers another transition (by calling game_state_set
; itself) we re-enter the new state in the same frame before ticking, so
; the freshly-entered state always gets its enter before its tick. Tick
; never runs more than once per frame.

GS_ATTRACT_SPLASH:  equ 0    ; bitriot logo splash
GS_ATTRACT_CHARS:   equ 1    ; character/nickname reveal sequence
GS_ATTRACT_HOLD:    equ 2    ; sit waiting for credit
GS_PLAYER_SELECT:   equ 3    ; 1 / 2 player choice
GS_READY:           equ 4    ; "READY!" pause before play
GS_PLAY:            equ 5    ; gameplay
GS_DYING:           equ 6    ; death animation  (stub)
GS_LEVEL_CLEAR:     equ 7    ; maze flash       (stub)
GS_GAME_OVER:       equ 8    ; "GAME OVER"      (stub)
GS_ATTRACT_DEMO:    equ 9    ; AI-driven Pac demo in the attract cycle (#11)

game_state:         db GS_ATTRACT_SPLASH
game_state_entered: db 0      ; 1 = enter already ran for current state

; Transition to a new state. A = new state id.
; Destroys: af
game_state_set:
    ld (game_state), a
    xor a
    ld (game_state_entered), a
    ret

; Dispatch one frame. Calls enter if needed, then tick.
; If enter changes state (clears entered flag), restart so the new state
; gets its enter before its tick.
; Destroys: af, plus whatever the called state code destroys.
game_state_tick:
    ld a, (game_state_entered)
    or a
    jr nz, @do_tick
    ld a, 1
    ld (game_state_entered), a
    call state_call_enter
    ld a, (game_state_entered)
    or a
    jr z, game_state_tick     ; enter triggered a transition; redo
@do_tick:
    jp state_call_tick

; Dispatch helpers: cp-ladder is simpler than a jump table and not in any
; tight loop (one call per frame).
state_call_enter:
    ld a, (game_state)
    cp GS_ATTRACT_SPLASH
    jp z, state_attract_splash_enter
    cp GS_ATTRACT_CHARS
    jp z, state_attract_chars_enter
    cp GS_ATTRACT_HOLD
    jp z, state_attract_hold_enter
    cp GS_PLAYER_SELECT
    jp z, state_player_select_enter
    cp GS_READY
    jp z, state_ready_enter
    cp GS_PLAY
    jp z, state_play_enter
    cp GS_DYING
    jp z, state_dying_enter
    cp GS_LEVEL_CLEAR
    jp z, state_level_clear_enter
    cp GS_GAME_OVER
    jp z, state_game_over_enter
    cp GS_ATTRACT_DEMO
    jp z, state_attract_demo_enter
    ret

state_call_tick:
    ld a, (game_state)
    cp GS_ATTRACT_SPLASH
    jp z, state_attract_splash_tick
    cp GS_ATTRACT_CHARS
    jp z, state_attract_chars_tick
    cp GS_ATTRACT_HOLD
    jp z, state_attract_hold_tick
    cp GS_PLAYER_SELECT
    jp z, state_player_select_tick
    cp GS_READY
    jp z, state_ready_tick
    cp GS_PLAY
    jp z, state_play_tick
    cp GS_DYING
    jp z, state_dying_tick
    cp GS_LEVEL_CLEAR
    jp z, state_level_clear_tick
    cp GS_GAME_OVER
    jp z, state_game_over_tick
    cp GS_ATTRACT_DEMO
    jp z, state_attract_demo_tick
    ret
