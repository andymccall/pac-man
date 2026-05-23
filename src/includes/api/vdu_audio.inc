; VDP audio primitives — VDU 23, 0, $85.
;
; The Agon VDP exposes 3 default audio channels (channel 0..2) with PSG-
; style tone generation. We just need the "play tone" command:
;
;   VDU 23, 0, $85, channel, 0, volume, freq_lo, freq_hi, duration_lo, duration_hi
;
; Volume:   0..127 (full byte range; 64 is comfortable mid-volume)
; Frequency: 16-bit Hz, little-endian
; Duration: 16-bit milliseconds, little-endian
;
; The command is queued into the channel's note queue — return is
; immediate, the VDP handles playback asynchronously. Notes overlapping
; on the same channel are queued in order; sending faster than playback
; will eventually drop notes when the queue fills.

; vdu_audio_set_volume — change the volume of a channel immediately,
; affecting the currently-playing note rather than queueing a new one.
; Setting volume to 0 is the correct way to "stop" an in-progress
; long-duration note: a vol=0 *play* command would instead enqueue a
; silent note that only plays *after* the previous one expires.
; Inputs:  A = channel id, B = volume (0..127)
; Destroys: af, bc, hl
vdu_audio_set_volume:
    ld (@sv_channel), a
    ld a, b
    ld (@sv_value), a
    ld hl, @sv_cmd
    ld bc, @sv_end - @sv_cmd
    rst.lil $18
    ret

@sv_cmd:        .db 23, 0, vdp_audio
@sv_channel:    .db 0
                .db 2                ; command 2 = set volume
@sv_value:      .db 0
@sv_end:

; vdu_audio_set_frequency — change the frequency of a channel without
; restarting the note. Use this to glide an already-playing tone (e.g.
; the siren's pitch sweep) without retriggering the envelope.
; Inputs:  A  = channel id, HL = frequency in Hz
; Destroys: af, bc, hl
vdu_audio_set_frequency:
    ld (@sf_channel), a
    ld a, l
    ld (@sf_freq_lo), a
    ld a, h
    ld (@sf_freq_hi), a
    ld hl, @sf_cmd
    ld bc, @sf_end - @sf_cmd
    rst.lil $18
    ret

@sf_cmd:        .db 23, 0, vdp_audio
@sf_channel:    .db 0
                .db 3                ; command 3 = set frequency
@sf_freq_lo:    .db 0
@sf_freq_hi:    .db 0
@sf_end:

; vdu_audio_enable_channel — enable a channel (channels 3..31 must be
; explicitly enabled before they accept notes; 0..2 are pre-enabled).
; Inputs:  A = channel id
; Destroys: af, bc, hl
vdu_audio_enable_channel:
    ld (@en_channel), a
    ld hl, @en_cmd
    ld bc, @en_end - @en_cmd
    rst.lil $18
    ret

@en_cmd:        .db 23, 0, vdp_audio
@en_channel:    .db 0
                .db 8                ; command 8 = enable channel
@en_end:

; vdu_audio_set_waveform — set the waveform shape for a channel.
;   0 = square, 1 = triangle, 2 = sawtooth, 3 = sine (default), 4 = noise,
;   5 = VIC noise, 8 = sample. Square / sawtooth give the harmonics-rich
;   "chiptune rough" timbre; sine is the smooth default.
; Inputs:  A = channel id (0..2), B = waveform id
; Destroys: af, bc, hl
vdu_audio_set_waveform:
    ld (@wf_channel), a
    ld a, b
    ld (@wf_value), a
    ld hl, @wf_cmd
    ld bc, @wf_end - @wf_cmd
    rst.lil $18
    ret

@wf_cmd:        .db 23, 0, vdp_audio
@wf_channel:    .db 0
                .db 4                ; command 4 = set waveform
@wf_value:      .db 0
@wf_end:

; vdu_audio_play_tone — queue a single tone on the given channel.
; Inputs:  A  = channel id (0..2)
;          B  = volume (0..127)
;          HL = frequency in Hz
;          DE = duration in milliseconds
; Destroys: af, bc, hl
vdu_audio_play_tone:
    ld (@channel), a
    ld a, b
    ld (@volume), a
    ld a, l
    ld (@freq_lo), a
    ld a, h
    ld (@freq_hi), a
    ld a, e
    ld (@dur_lo), a
    ld a, d
    ld (@dur_hi), a

    ld hl, @cmd
    ld bc, @end - @cmd
    rst.lil $18
    ret

@cmd:       .db 23, 0, vdp_audio
@channel:   .db 0
            .db 0                    ; command 0 = play note
@volume:    .db 0
@freq_lo:   .db 0
@freq_hi:   .db 0
@dur_lo:    .db 0
@dur_hi:    .db 0
@end:
