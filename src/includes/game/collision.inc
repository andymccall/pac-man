; Collision detection against the maze wall map.
;
; The maze is drawn with the graphics origin set to (origin_left+8, origin_top+8)
; i.e. screen pixel (146, 68). Each tile is 8x8, the grid is 28 cols x 31 rows.
; Tile value 06 in maze_wall_map is the walkable/empty tile; anything else is wall.
;
; Pac-Man is a 16x16 sprite whose top-left is tracked in (sprite_x, sprite_y).
; We test the tile covering the sprite's center (sx+8, sy+8).

maze_play_origin_x: equ 146       ; origin_left (138) + draw offset (8)
maze_play_origin_y: equ 68        ; origin_top  (60)  + draw offset (8)
maze_play_cols:     equ 28
maze_play_rows:     equ 31
maze_play_tile:     equ 8
maze_wall_floor:    equ 6         ; tile id that means "walkable"

sprite_to_tile_col: db 0
sprite_to_tile_row: db 0

; sprite_to_tile
; Convert sprite top-left (bc, de) into grid (col, row) at the sprite centre.
; Inputs:  bc = sprite top-left x (24-bit clean)
;          de = sprite top-left y (24-bit clean)
; Outputs: on success, carry=0, c=col (0..27), b=row (0..30)
;          on off-grid, carry=1
; Destroys: af, hl, bc, de
sprite_to_tile:
    ; hl = (sprite_x + 8) - maze_play_origin_x
    ld hl, 8
    add hl, bc
    ld bc, maze_play_origin_x
    or a
    sbc hl, bc
    jr c, @off_grid
    srl h
    rr  l
    srl h
    rr  l
    srl h
    rr  l
    ld a, h
    or a
    jr nz, @off_grid
    ld a, l
    cp maze_play_cols
    jr nc, @off_grid
    ld (sprite_to_tile_col), a

    ; hl = (sprite_y + 8) - maze_play_origin_y
    push de
    pop hl
    ld bc, 8
    add hl, bc
    ld bc, maze_play_origin_y
    or a
    sbc hl, bc
    jr c, @off_grid
    srl h
    rr  l
    srl h
    rr  l
    srl h
    rr  l
    ld a, h
    or a
    jr nz, @off_grid
    ld a, l
    cp maze_play_rows
    jr nc, @off_grid
    ld b, a                   ; b = row

    ld a, (sprite_to_tile_col)
    ld c, a                   ; c = col
    or a                      ; clear carry
    ret
@off_grid:
    scf
    ret

; tile_row_col_to_offset
; Compute row*28 + col as a 24-bit offset into a 28-column map.
; Inputs:  b = row, c = col
; Outputs: hl = offset
; Destroys: af, de, hl
tile_row_col_to_offset:
    ; ADL gotcha: ld h,0 only clears the middle byte, leaving HLU dirty
    ; from whatever the caller had in HL. ld hl,0 is a 24-bit immediate
    ; that zeroes HLU, H, and L. Without this, blinky_decide (which sets
    ; HL = $FFFFFF for bd_best_dist) leaves HLU = $FF on entry, so the
    ; offset calculation produces garbage and the wall lookup reads from
    ; the wrong memory.
    ld hl, 0
    ld l, b
    add hl, hl                ; row*2
    add hl, hl                ; row*4
    ld d, h
    ld e, l                   ; de = row*4
    add hl, hl                ; row*8
    add hl, hl                ; row*16
    add hl, hl                ; row*32
    or a
    sbc hl, de                ; row*28
    ld d, 0
    ld e, c
    add hl, de                ; + col
    ret

; tile_is_wall
; Inputs:  bc = candidate sprite top-left x
;          de = candidate sprite top-left y
; Outputs: Z=1 -> walkable, Z=0 -> wall or off-grid
; Destroys: af, hl, bc, de
tile_is_wall:
    call sprite_to_tile
    jr c, @wall
    call tile_row_col_to_offset
    ld de, maze_wall_map
    add hl, de
    ld a, (hl)
    cp maze_wall_floor
    ret
@wall:
    or 1
    ret
