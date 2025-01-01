display_1up:
    ;Print the 1UP text
    ld hl, up1_txt
    ld bc, up1_txt_end - up1_txt
    rst.lil VDU_OUTPUT_TO_VDP
    ret

display_2up:
    ; Print the 2UP text
    ld hl, up2_txt
    ld bc, up2_txt_end - up2_txt
    rst.lil VDU_OUTPUT_TO_VDP
    ret

display_high_score:
    ; Print the high score
    ld hl, high_score_data
    ld bc, high_score_data_end - high_score_data
    rst.lil VDU_OUTPUT_TO_VDP
    ret