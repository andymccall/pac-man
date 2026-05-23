; Game-side audio (#10) — calls into vdu_audio.inc's primitives.
;
; The arcade waka is a single Namco WSG voice playing wave 3 (a complex
; harmonic-rich ROM shape) at ~375 Hz. On the Agon VDP we don't have
; wave 3, so we stack THREE simple waveforms on three channels to fake
; the harmonic richness: a low sawtooth (deep body), a higher sawtooth
; (top edge), and a triangle layer on top (sparkle / ring). Each pellet
; eat triggers all three in parallel.
;
; Original arcade effect 0 on channel 1 from reference/sound.asm:
;   bytes: 73 20 00 0C 00 0A 1F 00
;   base = 0x20, freq_shift = 3 → freq value = 256
;   Namco WSG @ 96 kHz / 65536-step cycle → fundamental ≈ 375 Hz
;   duration = 12 vblanks @ 60 Hz = 200 ms with volume decay

; ---------------------------------------------------------------------------
; Tunables — change these and rebuild to taste-test on hardware.
; ---------------------------------------------------------------------------
;
; The chomp plays as a 3-layer chord (low saw + high saw + top triangle)
; on three parallel channels. Successive eats alternate between two
; pitch sets: WA_FREQ_* (open-mouth, lower) and KA_FREQ_* (closed-mouth,
; higher). Everything else — channels, waveforms, volumes, duration —
; is shared between the halves so only the pitch flips and the timbre
; stays consistent.
;
; WAKA_WAVEFORM_xxx (Agon VDP waveform id, per layer):
;   0 = square      — bright, hollow, rich odd harmonics
;   1 = triangle    — softer than square, mellower
;   2 = sawtooth    — rough / brassy, full harmonic content
;   3 = sine        — pure tone (default; "smooth")
;   4 = noise       — white noise
;   5 = VIC noise   — quantised noise (chiptune-percussion)
;
; WA_FREQ_xxx / KA_FREQ_xxx (Hz, per layer per half):
;   The two halves' fundamentals. Spread layers out (octaves apart) for
;   a bell-like timbre, closer together for a thicker chord. The KA set
;   is typically 25–30 % higher than the matching WA freq. ~80 Hz is
;   the low audible limit on typical speakers; ~2000 Hz is shrill.
;
; WAKA_VOLUME_xxx (0..127, per layer, shared across halves):
;   Mix between layers. Lower-frequency layers usually want more level
;   to feel "equal loudness" (Fletcher–Munson). Triangle layer typically
;   sits 10–15 below the sawtooths so it adds a top-end ring without
;   dominating. Set any layer's volume to 0 to drop it entirely.
;
; WAKA_DURATION (ms, shared across both halves):
;   Per-tone length, applied to all three layers. At Pac's L1 speed one
;   pellet is eaten every ~130 ms, so values >120 will overlap in the
;   per-channel note queue.

; The chomp is actually TWO distinct sounds in the arcade, played
; alternately ("wa", "ka", "wa", "ka" ...). Per the Vecoven sound
; disassembly: channel-1 effects 0 and 1 both use base freq 0x20 but
; pick different Namco waves (wave 3 vs wave 2) and effect 1 also has
; a small downward freq slide. Each is ~82 ms long, and the maximum
; dot-pickup rate is one every ~122 ms even at Pac's top speed — so
; the per-chomp envelope never has to be retriggered mid-flight.
;
; We don't have those wave shapes on the Agon VDP, so we fake the two
; sounds by sharing the same 3-layer setup (low saw / high saw / top
; triangle) and only varying the FREQUENCIES between "wa" and "ka".
; Channels, waveforms and per-layer volumes stay the same for both
; halves so the timbre is consistent — only the pitch flips.

; --- Per-layer channel + waveform (shared across wa / ka) ----------------
; All three layers are triangle to match the cleaner timbre the rest of
; the game's audio uses (siren, melodies, ghost chord, jingles all run
; on triangle / single-tone). Earlier rough mixes (saw body + square
; bark) sounded arcade-faithful in isolation but clashed against the
; rest. The 3-layer chord stays for fullness — just all the same shape.
WAKA_CHANNEL_LOW:    equ 2
WAKA_WAVEFORM_LOW:   equ 1               ; triangle (was sawtooth)

WAKA_CHANNEL_HIGH:   equ 1
WAKA_WAVEFORM_HIGH:  equ 1               ; triangle (was square)

; Channel 0 originally reserved for the siren — siren has since moved
; to channel 3, so the triangle layer keeps channel 0.
WAKA_CHANNEL_TRI:    equ 0
WAKA_WAVEFORM_TRI:   equ 1               ; triangle — sparkle / ring

; --- "wa" half — open-mouth, lower-pitched, vowel-emphasised ---------------
; Detuned off exact octaves to create slight "beating" between layers —
; simulates the harmonic distortion of the arcade's 4-bit wavetable.
; Volume mix tilts toward LOW for a deeper "ahhh" body.
WA_FREQ_LOW:         equ 135             ; +10 % bump for arcade match (was 123 ≈ B2)
WA_FREQ_HIGH:        equ 280             ; +10 % (was 255 — keeps the slight sharp detune)
WA_FREQ_TRI:         equ 543             ; +10 % (was 494 ≈ B4)
WA_VOLUME_LOW:       equ 0               ; option C: single triangle layer (TRI only)
WA_VOLUME_HIGH:      equ 0
WA_VOLUME_TRI:       equ 65              ; bumped to lead-level since it's now alone
WA_DURATION:         equ 85              ; longer than ka — vowel sustains

; --- "ka" half — closed-mouth, higher-pitched, consonant-emphasised --------
; Closer to clean octave ratios; the "ka" is the tighter / more percussive
; of the two halves. Volume mix tilts toward HIGH for a brighter "click".
KA_FREQ_LOW:         equ 170             ; +10 % (was 155 ≈ Eb3)
KA_FREQ_HIGH:        equ 342             ; +10 % (was 311 ≈ Eb4)
KA_FREQ_TRI:         equ 684             ; +10 % (was 622 ≈ Eb5)
KA_VOLUME_LOW:       equ 0               ; option C: single triangle layer
KA_VOLUME_HIGH:      equ 0
KA_VOLUME_TRI:       equ 70              ; slightly louder than wa for consonant snap
KA_DURATION:         equ 60              ; shorter than wa — consonant clips

; --- Noise burst (ka only) ------------------------------------------------
; A brief VIC-noise pop on channel 4 layered over the start of each "ka",
; simulating the physical "clack" of the mouth shutting. Channel 4 must
; be enabled at boot (default Agon firmware exposes 0–2 only).
NOISE_CHANNEL:       equ 4
NOISE_WAVEFORM:      equ 5               ; VIC noise (quantised, chiptune-percussive)
NOISE_FREQ:          equ 200             ; arbitrary — VIC-noise pitch is coarse
NOISE_VOLUME:        equ 35
NOISE_DURATION:      equ 15              ; ms — short clack

; Toggle: 0 → next call plays "wa", 1 → next call plays "ka". Flipped
; on every audio_waka call.
audio_waka_phase:    db 0

; --- Siren (background "wooowoooo") ---------------------------------------
;
; A continuous triangle tone on channel 3 whose pitch sweeps up and down
; under the VDP's built-in *stepped frequency envelope* (audio command
; #7, envelope type 1). Once configured, the VDP modulates the pitch on
; its own — no per-frame work from us. This is much cleaner than the
; first cut that wrote a new frequency every vblank.
;
; Envelope layout (matches Richard Turnnidge's Agon example, balanced
; so each cycle returns to its starting frequency rather than drifting
; upward):
;
;   base frequency        SIREN_FREQ_BASE (start of phase 1)
;   step length           SIREN_STEP_LEN_MS (per-step dwell)
;   phase 1 (up)          +SIREN_PHASE1_ADJ × SIREN_PHASE1_STEPS
;   phase 2 (down)        -SIREN_PHASE1_ADJ × SIREN_PHASE1_STEPS
;   control bits          bit 0 = repeats, bit 1 = cumulative
;
; With BASE=400, ADJ=30, STEPS=10, STEP_LEN=40ms the sweep goes
; 400 → 700 → 400 → 700 ... in 800 ms per full cycle.
;
; Channel 3 isn't pre-enabled by default Agon firmware — audio_init
; issues a vdu_audio_enable_channel for it before any siren plays.
; SIREN_DURATION is $FFFF (= -1 in the Agon API), which the VDP reads
; as "play forever". siren_stop ends the note by setting the channel
; volume to 0 via the immediate set_volume command, never relying on
; the duration to time out.
;
; Per-pellet-count tier changes (arcade has 4 progressively-faster
; sweeps based on pellets remaining) follow in a later pass.
SIREN_CHANNEL:       equ 3
SIREN_WAVEFORM:      equ 1               ; triangle — mellow, warbling feel
SIREN_VOLUME:        equ 35              ; background-level, sits under the waka
SIREN_DURATION:      equ $FFFF           ; -1 = play forever; siren_stop mutes via set_volume
SIREN_FREQ_BASE:     equ 650             ; centre of the sweep
SIREN_STEP_LEN_MS:   equ 18              ; dwell per step (smaller = faster cycle)

; audio_init — one-shot channel setup. Called from start: in pac-man.asm
; before any sound plays. Sets each waka channel to its configured
; waveform, installs the chomp ADSR on those channels, enables channel
; 3 for the siren, and sets its waveform too.
; Destroys: af, bc, hl
audio_init:
    ld a, WAKA_CHANNEL_LOW
    ld b, WAKA_WAVEFORM_LOW
    call vdu_audio_set_waveform
    ld a, WAKA_CHANNEL_HIGH
    ld b, WAKA_WAVEFORM_HIGH
    call vdu_audio_set_waveform
    ld a, WAKA_CHANNEL_TRI
    ld b, WAKA_WAVEFORM_TRI
    call vdu_audio_set_waveform

    ; Channel 3 (siren) — enable + set triangle waveform.
    ld a, SIREN_CHANNEL
    call vdu_audio_enable_channel
    ld a, SIREN_CHANNEL
    ld b, SIREN_WAVEFORM
    call vdu_audio_set_waveform

    ; Channel 4 (noise burst for "ka" mouth clack) — enable + VIC noise.
    ld a, NOISE_CHANNEL
    call vdu_audio_enable_channel
    ld a, NOISE_CHANNEL
    ld b, NOISE_WAVEFORM
    jp vdu_audio_set_waveform

; --- Per-half envelope install helpers ------------------------------------
;
; Each chomp re-installs the right ADSR + frequency-slide envelope on
; all three waka channels before playing. That's 6 envelope sends + 3
; play_tones per chomp = 9 VDU commands, ~120 bytes — well within the
; UART budget per frame.
;
; The static envelope blocks share a structure:
;   [VDU 23, 0, $85, CHANNEL, ...rest...]
; where CHANNEL is patched per call.

install_adsr_wa:
    ld (adsr_wa_channel), a
    ld hl, adsr_wa_cmd
    ld bc, adsr_wa_end - adsr_wa_cmd
    rst.lil $18
    ret

install_adsr_ka:
    ld (adsr_ka_channel), a
    ld hl, adsr_ka_cmd
    ld bc, adsr_ka_end - adsr_ka_cmd
    rst.lil $18
    ret

install_slide_wa:
    ld (slide_wa_channel), a
    ld hl, slide_wa_cmd
    ld bc, slide_wa_end - slide_wa_cmd
    rst.lil $18
    ret

install_slide_ka:
    ld (slide_ka_channel), a
    ld hl, slide_ka_cmd
    ld bc, slide_ka_end - slide_ka_cmd
    rst.lil $18
    ret

; --- Envelope blocks ------------------------------------------------------

; "wa" ADSR — gentle 8 ms attack, decay across the 85 ms note, small
; sustain tail. Gives the vowel a "swell" character rather than instant
; punch.
adsr_wa_cmd:
    .db 23, 0, vdp_audio
adsr_wa_channel:
    .db 0
    .db 6, 1                                 ; command 6, type 1 = ADSR
    .dw 8                                    ; attack ms
    .dw 70                                   ; decay ms
    .db 5                                    ; sustain (light tail)
    .dw 12                                   ; release ms
adsr_wa_end:

; "ka" ADSR — instant attack, sharp decay across the 60 ms note, silent
; sustain, no release. Pure percussive hit.
adsr_ka_cmd:
    .db 23, 0, vdp_audio
adsr_ka_channel:
    .db 0
    .db 6, 1
    .dw 0                                    ; attack — instant
    .dw 50                                   ; decay
    .db 0                                    ; sustain — silent
    .dw 0                                    ; release
adsr_ka_end:

; "wa" frequency slide — gentle upward bloom (+4 over 10 steps then back
; down -4 over 10 steps). Each step 4 ms; total 80 ms ≈ WA_DURATION.
; Non-repeating, cumulative (ctrl = 2 = bit 1 only). Each new note
; restarts the envelope from its triggering frequency.
slide_wa_cmd:
    .db 23, 0, vdp_audio
slide_wa_channel:
    .db 0
    .db 7, 1                                 ; command 7, type 1 = stepped freq env
    .db 2                                    ; 2 phases
    .db 2                                    ; ctrl: cumulative, no repeat
    .dw 4                                    ; step length ms
    .dw 4                                    ; phase 1 adj: +4
    .dw 10                                   ; phase 1 steps
    .dw $FFFC                                ; phase 2 adj: -4 (two's complement)
    .dw 10                                   ; phase 2 steps
slide_wa_end:

; "ka" frequency slide — sharp downward snap (-8 over 6 steps, then
; level off at the new low). One-phase envelope, ctrl = 2 (cumulative,
; non-repeating). Total slide time: 24 ms, well within KA_DURATION.
slide_ka_cmd:
    .db 23, 0, vdp_audio
slide_ka_channel:
    .db 0
    .db 7, 1
    .db 1                                    ; 1 phase
    .db 2                                    ; ctrl: cumulative, no repeat
    .dw 4                                    ; step length ms
    .dw $FFF8                                ; phase adj: -8
    .dw 6                                    ; phase steps
slide_ka_end:
chomp_adsr_end:

; siren_start — install the frequency envelope on channel 3 and kick
; off the (effectively-infinite) note. The VDP handles per-step pitch
; updates internally from here on, so there is no per-frame work to
; do. Called from state_play_enter and state_attract_demo_enter; safe
; to call repeatedly (the latest play wins).
; Destroys: af, bc, hl
siren_start:
    ; Send the envelope config first so the VDP starts modulating the
    ; moment the note begins.
    ld hl, siren_envelope_cmd
    ld bc, siren_envelope_cmd_end - siren_envelope_cmd
    rst.lil $18

    ; Kick off the note at the base frequency. The envelope drives the
    ; pitch from here.
    ld a, SIREN_CHANNEL
    ld b, SIREN_VOLUME
    ld hl, SIREN_FREQ_BASE
    ld de, SIREN_DURATION
    jp vdu_audio_play_tone

; siren_stop — mute the siren. Sets the channel volume to 0 via the
; "set volume" command, which acts on the currently-playing note
; immediately. A zero-volume *play* command would instead queue a
; silent note behind the in-progress 65 s siren, leaving the channel
; audibly going until the long note's duration expired — which is
; exactly the bug we hit when the maze first cleared.
; Destroys: af, bc, hl
siren_stop:
    ld a, SIREN_CHANNEL
    ld b, 0
    jp vdu_audio_set_volume

; --- Envelope command block -----------------------------------------------
;
; Sent as one VDU stream by siren_start. Layout (after the standard
; VDU 23, 0, $85 prefix):
;
;   channel, 7 (cmd), 1 (type=stepped), 2 (phase count), 3 (ctrl),
;   step_length_ms (word),
;   phase1_adj (word), phase1_steps (word),
;   phase2_adj (word), phase2_steps (word)
;
; ctrl = 3 sets bit 0 (repeats) + bit 1 (cumulative). With balanced
; phases (+ADJ over N steps in phase 1, -ADJ over N in phase 2) the
; cumulative bit is harmless because each cycle nets to zero. If we
; ever shift to non-zero net phases, switch ctrl to 1 to avoid drift.
;
; Phase 2's adjustment is the two's-complement of phase 1: for ADJ=30
; the phase 2 word is $FFE2 (-30). Hand-encoded to dodge any concern
; about ez80asm's handling of negative literals in `.dw`.
siren_envelope_cmd:
    .db 23, 0, vdp_audio
    .db SIREN_CHANNEL
    .db 7                            ; command 7 = frequency envelope
    .db 1                            ; type 1 = stepped
    .db 2                            ; two phases
    .db 3                            ; ctrl: repeats + cumulative
    .dw SIREN_STEP_LEN_MS            ; step dwell (ms)
    .dw 30                           ; phase 1 adj: +30 per step
    .dw 10                           ; phase 1: 10 steps → +300
    .dw $FFE2                        ; phase 2 adj: -30 (two's complement)
    .dw 10                           ; phase 2: 10 steps → -300, balances
siren_envelope_cmd_end:

; siren_set_fright / siren_set_normal — swap the channel-3 envelope to
; the "wakka wakka" fright variant or back. The note keeps playing —
; only the envelope shape changes, so the listener hears the pitch
; pattern shift mid-stream when fright activates / expires.
;
; Fright envelope sweeps faster and wider, sitting higher in the mix
; to read as more frantic. ctrl stays at 3 (repeats + cumulative,
; balanced phases) so the wakka cycles cleanly with no drift.
; Destroys: af, bc, hl
siren_set_fright:
    ld hl, siren_envelope_fright_cmd
    ld bc, siren_envelope_fright_end - siren_envelope_fright_cmd
    rst.lil $18
    ret

siren_set_normal:
    ld hl, siren_envelope_cmd
    ld bc, siren_envelope_cmd_end - siren_envelope_cmd
    rst.lil $18
    ret

siren_envelope_fright_cmd:
    .db 23, 0, vdp_audio
    .db SIREN_CHANNEL
    .db 7, 1, 2, 3
    .dw 12                           ; step dwell — faster than normal
    .dw 50                           ; phase 1 adj: +50/step
    .dw 8                            ; 8 steps → +400
    .dw $FFCE                        ; phase 2 adj: -50 (two's complement)
    .dw 8                            ; 8 steps → -400, balanced
siren_envelope_fright_end:

; --- Ghost-eaten ascending chord ------------------------------------------
;
; Each ghost eaten during a single fright plays one note in an ascending
; arpeggio (C5, E5, G5, C6). The chain index is `fright_eaten_count`
; (read by `audio_ghost_eaten` directly), capped at 3 for safety. One
; tone per call, played on channel 0 (melodic triangle waka layer).

GHOST_EAT_CHANNEL:   equ 0
GHOST_EAT_VOLUME:    equ 80
GHOST_EAT_DURATION:  equ 150            ; ms — let the chord notes ring

ghost_eat_freq_table:
    .dw 523                          ; index 0: C5  (200 pts)
    .dw 659                          ; index 1: E5  (400 pts)
    .dw 784                          ; index 2: G5  (800 pts)
    .dw 1047                         ; index 3: C6  (1600 pts)

; audio_ghost_eaten — play the next note in the ascending arpeggio,
; based on the current fright_eaten_count. Caller invokes BEFORE
; incrementing the count, so values 0..3 map to the four chained
; ghost-eats correctly.
; Destroys: af, bc, de, hl, ix
audio_ghost_eaten:
    ld a, (fright_eaten_count)
    cp 4
    ret nc                           ; defensive: 5th+ eat does nothing
    add a, a                         ; index * 2 (word entries)
    ld hl, 0
    ld l, a
    ld de, ghost_eat_freq_table
    add hl, de
    ld a, (hl)
    inc hl
    ld h, (hl)
    ld l, a                          ; HL = freq

    ld a, GHOST_EAT_CHANNEL
    ld b, GHOST_EAT_VOLUME
    ld de, GHOST_EAT_DURATION
    jp vdu_audio_play_tone

; --- Fruit-eat jingle ----------------------------------------------------
;
; Quick 3-note ascending flourish when Pac grabs the bonus fruit. Plays
; via the melody player (same channel 0, same drip-feed pattern as the
; death jingle and READY tune).

melody_start_fruit:
    ld hl, melody_data_fruit
    jp melody_start

melody_data_fruit:
    .dw 698,  4                      ; F5  — 67 ms
    .dw 988,  4                      ; B5
    .dw 1318, 8                      ; E6  — held longer for the "tada"
    .dw 0,    0

; --- Bonus-life jingle ---------------------------------------------------
;
; Short 5-note fanfare when the player crosses 10 000 pts. Fires once
; per game (bonus_life_check gates it).

melody_start_bonus_life:
    ld hl, melody_data_bonus_life
    jp melody_start

melody_data_bonus_life:
    .dw 523,  4                      ; C5
    .dw 659,  4                      ; E5
    .dw 784,  4                      ; G5
    .dw 1047, 4                      ; C6
    .dw 1318, 9                      ; E6 — held
    .dw 0,    0

; audio_waka — queue one chomp. Alternates "wa" and "ka" pitches on
; successive calls (arcade-faithful — see header comment above). All
; three layers fire on the same VBLANK so the listener hears one
; chord-like event, not three staggered tones.
;
; The phase flag is read into B, used to dispatch to the wa / ka block,
; then XOR'd and written back so the next call uses the other set.
; Destroys: af, bc, de, hl
audio_waka:
    ld a, (audio_waka_phase)
    ld b, a                              ; remember which half we're playing
    xor 1
    ld (audio_waka_phase), a             ; flip for next call

    ld a, b
    or a
    jp nz, @play_ka

    ; --- "wa" half ---------------------------------------------------------
    ; Install wa ADSR on all three layers, then fire. The per-note
    ; frequency slide and the channel-4 noise burst (originally added
    ; for arcade-faithfulness) sounded too busy in combination — they
    ; can be re-enabled by restoring the `call install_slide_wa` block
    ; and the channel-4 burst at the end of @play_ka.
    ld a, WAKA_CHANNEL_LOW
    call install_adsr_wa
    ld a, WAKA_CHANNEL_HIGH
    call install_adsr_wa
    ld a, WAKA_CHANNEL_TRI
    call install_adsr_wa

    ld a, WAKA_CHANNEL_LOW
    ld b, WA_VOLUME_LOW
    ld hl, WA_FREQ_LOW
    ld de, WA_DURATION
    call vdu_audio_play_tone
    ld a, WAKA_CHANNEL_HIGH
    ld b, WA_VOLUME_HIGH
    ld hl, WA_FREQ_HIGH
    ld de, WA_DURATION
    call vdu_audio_play_tone
    ld a, WAKA_CHANNEL_TRI
    ld b, WA_VOLUME_TRI
    ld hl, WA_FREQ_TRI
    ld de, WA_DURATION
    jp vdu_audio_play_tone

@play_ka:
    ; --- "ka" half ---------------------------------------------------------
    ld a, WAKA_CHANNEL_LOW
    call install_adsr_ka
    ld a, WAKA_CHANNEL_HIGH
    call install_adsr_ka
    ld a, WAKA_CHANNEL_TRI
    call install_adsr_ka

    ld a, WAKA_CHANNEL_LOW
    ld b, KA_VOLUME_LOW
    ld hl, KA_FREQ_LOW
    ld de, KA_DURATION
    call vdu_audio_play_tone
    ld a, WAKA_CHANNEL_HIGH
    ld b, KA_VOLUME_HIGH
    ld hl, KA_FREQ_HIGH
    ld de, KA_DURATION
    call vdu_audio_play_tone
    ld a, WAKA_CHANNEL_TRI
    ld b, KA_VOLUME_TRI
    ld hl, KA_FREQ_TRI
    ld de, KA_DURATION
    jp vdu_audio_play_tone

; --- Credit-insert bleep --------------------------------------------------
;
; Short ascending two-tone "biiip" played when a credit is deposited (via
; the 'C' key today, eventually the coin slot). Shares channels 1+2 with
; the waka because credits are deposited at the title screen or attract
; phases where the waka isn't firing — no realistic collision.

CREDIT_CHANNEL_LO:   equ 1
CREDIT_CHANNEL_HI:   equ 2
CREDIT_WAVEFORM:     equ 2               ; sawtooth (channels are already saw from waka init)
CREDIT_FREQ_LO:      equ 440             ; A4-ish
CREDIT_FREQ_HI:      equ 880             ; one octave up
CREDIT_VOLUME:       equ 60
CREDIT_DURATION:     equ 80              ; ms — quick chirp, not a chime

; audio_credit — queue the credit-insert bleep. Non-blocking; both
; channels fire on the same VBLANK so the listener hears one event.
; Destroys: af, bc, de, hl
audio_credit:
    ld a, CREDIT_CHANNEL_LO
    ld b, CREDIT_VOLUME
    ld hl, CREDIT_FREQ_LO
    ld de, CREDIT_DURATION
    call vdu_audio_play_tone

    ld a, CREDIT_CHANNEL_HI
    ld b, CREDIT_VOLUME
    ld hl, CREDIT_FREQ_HI
    ld de, CREDIT_DURATION
    jp vdu_audio_play_tone

; --- READY-screen melody --------------------------------------------------
;
; The iconic Pac-Man intro tune. Plays during the READY/announce phase
; (no Pac or ghosts on the board); when it finishes, a life is deducted
; and Pac is placed — see state_ready_tick's phase-0 branch.
;
; Source: Robson Couto's Arduino songs port (reference/melody.ino),
; itself transcribed from the arcade. 31 notes at tempo 105 — total
; ~4.5 s.
;
; The Agon's per-channel note queue is short (8 by default), so we
; drip-feed: each call to melody_tick checks if it's time for the next
; note, sends it, and arms the per-note frame timer. ms-to-frames
; conversion uses a flat 16 ms/frame approximation (the real frame is
; 16.67 ms at 60 fps — a ~4 % timing error, ~180 ms slip across the
; whole melody, which the listener won't notice).
;
; Note data is (freq word, duration_frames word). Terminator freq = 0.

MELODY_CHANNEL:      equ 0               ; triangle waka channel — works for melody too
MELODY_VOLUME:       equ 80              ; lead-level, prominent

melody_ptr:          dl 0
melody_timer:        dw 0
melody_active:       db 0

; melody_data_ready — Pac-Man intro tune, ~4.5 s at tempo 105. Per
; entry: 2-byte frequency (Hz), 2-byte duration in 60 Hz frames.
melody_data_ready:
    ; Phrase 1: B Major feel
    .dw 494,  9               ; B4   16th
    .dw 988,  9               ; B5   16th
    .dw 740,  9               ; F#5  16th
    .dw 622,  9               ; D#5  16th
    .dw 988,  9               ; B5   16th
    .dw 740,  9               ; F#5  16th
    .dw 622, 18               ; D#5  8th (The "Landing" note)

    ; Phrase 2: C Major feel
    .dw 523,  9               ; C5   16th
    .dw 1047, 9               ; C6   16th
    .dw 784,  9               ; G5   16th (Corrected from your G6)
    .dw 659,  9               ; E5   16th (Corrected from your E6)
    .dw 1047, 9               ; C6   16th
    .dw 784,  9               ; G5   16th
    .dw 659, 18               ; E5   8th

    ; Phrase 3: Repeat B Major
    .dw 494,  9               ; B4   16th
    .dw 988,  9               ; B5   16th
    .dw 740,  9               ; F#5  16th
    .dw 622,  9               ; D#5  16th
    .dw 988,  9               ; B5   16th
    .dw 740,  9               ; F#5  16th
    .dw 622, 18               ; D#5  8th

    ; The Chromatic Run (The fast "Chirp" up to the end)
    .dw 622,  4               ; D#5  32nd
    .dw 659,  4               ; E5   32nd
    .dw 698,  4               ; F5   32nd
    .dw 740,  4               ; F#5  32nd
    .dw 784,  4               ; G5   32nd
    .dw 831,  4               ; G#5  32nd
    .dw 880,  9               ; A5   16th
    .dw 988, 18               ; B5   8th
    .dw 0,    0               ; Terminator

; melody_data_death — descending chromatic death sequence in four
; semitone sweeps, ending on a very low "drain pop". 17 notes, ~1.7 s
; total. Notes supplied by the user.
melody_data_death:
    ; --- Sweep 1 ---
    .dw 988,  5               ; B5
    .dw 932,  5               ; A#5
    .dw 880,  5               ; A5
    .dw 831,  5               ; G#5

    ; --- Sweep 2 ---
    .dw 784,  5               ; G5
    .dw 740,  5               ; F#5
    .dw 698,  5               ; F5
    .dw 659,  5               ; E5

    ; --- Sweep 3 ---
    .dw 622,  5               ; D#5
    .dw 587,  5               ; D5
    .dw 554,  5               ; C#5
    .dw 523,  5               ; C5

    ; --- Sweep 4 (the final "drain") ---
    .dw 494,  5               ; B4
    .dw 466,  5               ; A#4
    .dw 440,  5               ; A4
    .dw 415,  5               ; G#4

    ; --- The "pop" ---
    .dw 110, 20               ; Very low A2 — mimics the final pop
    .dw 0,    0               ; Terminator

; melody_start_ready / melody_start_death — pick a tune and arm the
; player. The first note fires on the next melody_tick, keeping the
; start aligned with the next animation frame rather than mid-call.
; Destroys: af, hl
melody_start_ready:
    ld hl, melody_data_ready
    jr melody_start

melody_start_death:
    ld hl, melody_data_death
    ; fall through

; melody_start — generic entry. HL = pointer to a (freq, frames) table
; ending in a zero-freq terminator.
melody_start:
    ld (melody_ptr), hl
    ld hl, 0
    ld (melody_timer), hl                ; first tick plays note 0
    ld a, 1
    ld (melody_active), a
    ret

; melody_stop — abort the melody. Mutes the channel immediately (in case
; a note is mid-flight) and clears active so further ticks no-op.
; Destroys: af, bc, hl
melody_stop:
    xor a
    ld (melody_active), a
    ld a, MELODY_CHANNEL
    ld b, 0
    jp vdu_audio_set_volume

; melody_tick — one frame of advancement. Cheap no-op when not active.
; If the per-note timer hits 0, plays the next note and arms the timer
; for that note's duration; on terminator, sets melody_active = 0 (so
; the caller can detect "finished" by checking the flag).
; Destroys: af, bc, de, hl, ix
melody_tick:
    ld a, (melody_active)
    or a
    ret z

    ; Decrement frame timer; if still > 0, just wait.
    ld hl, (melody_timer)
    ld a, h
    or l
    jr z, @play_next
    dec hl
    ld (melody_timer), hl
    ret

@play_next:
    ld ix, (melody_ptr)

    ; Terminator? (freq word == 0 → no more notes)
    ld a, (ix + 0)
    or (ix + 1)
    jr z, @done

    ; Pull duration_frames (ix+2..+3) into HL, save as next-note timer
    ; AND derive ms = frames << 4 for the VDP play_tone call.
    ld l, (ix + 2)
    ld h, (ix + 3)
    ld (melody_timer), hl
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl                           ; HL = frames * 16 ≈ ms
    ex de, hl                            ; DE = duration_ms

    ; Frequency word into HL
    ld l, (ix + 0)
    ld h, (ix + 1)

    ; Advance pointer (4 bytes per entry) before play_tone clobbers regs.
    push hl
    push de
    ld hl, (melody_ptr)
    ld bc, 4
    add hl, bc
    ld (melody_ptr), hl
    pop de
    pop hl

    ; play_tone(channel, vol, freq, ms)
    ld a, MELODY_CHANNEL
    ld b, MELODY_VOLUME
    jp vdu_audio_play_tone

@done:
    xor a
    ld (melody_active), a
    ret
