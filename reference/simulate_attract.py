import re
import os

# Coordinate system notes:
# In the original Pac-Man hardware, the screen is rotated 90 degrees CCW.
# The Z80 logic tracks coordinates relative to the rotated screen (VRAM coordinates):
# VRAM columns (0 to 31) map to cabinet rows: row_cabinet = col_vram + 2
# VRAM rows (0 to 31) map to cabinet columns: col_cabinet = row_vram - 2
#
# Direction codes in VRAM map to physical Cabinet directions as follows:
# 0 = VRAM UP (Y-1, X+0)     -> Cabinet LEFT (col_cab-1, row_cab+0)
# 1 = VRAM RIGHT (Y+0, X+1)  -> Cabinet DOWN (col_cab+0, row_cab+1)
# 2 = VRAM DOWN (Y+1, X+0)   -> Cabinet RIGHT (col_cab+1, row_cab+0)
# 3 = VRAM LEFT (Y+0, X-1)    -> Cabinet UP (col_cab+0, row_cab-1)

DIRECTIONS = {
    0: (0, -1, "LEFT"),
    1: (1, 0, "DOWN"),
    2: (0, 1, "RIGHT"),
    3: (-1, 0, "UP")
}

def parse_and_decode_vram():
    asm_path = "pac-man.asm"
    if not os.path.exists(asm_path):
        raise FileNotFoundError(f"Pac-Man assembly file not found at {asm_path}")

    # Parse lines to construct a dictionary of ROM addresses and bytes
    # Example: 3435  40        ld      b,b
    line_re = re.compile(r"^\s*([0-9a-fA-F]{4})\s+([0-9a-fA-F]+)\s+")
    rom = {}
    with open(asm_path, "r") as f:
        for line in f:
            m = line_re.match(line)
            if m:
                addr_str, bytes_str = m.groups()
                addr = int(addr_str, 16)
                num_bytes = len(bytes_str) // 2
                for i in range(num_bytes):
                    val = int(bytes_str[i*2:(i+1)*2], 16)
                    rom[addr + i] = val

    vram = [0x40] * 1024
    
    # In Z80, HL starts at 0x4000, and BC starts at 0x3435.
    # We will use 0-based offsets for VRAM (HL - 0x4000).
    hl = 0
    bc = 0x3435

    while True:
        a = rom.get(bc, 0)
        if a == 0:
            break

        if a >= 0x80:
            hl += 1
            vram[hl] = a
            # Mirroring
            col = hl % 32
            hl_mirror = 0x3e0 - hl + 2 * col
            vram[hl_mirror] = a ^ 0x01
            bc += 1
        else:
            hl += a
            bc += 1
            a = rom.get(bc, 0)
            vram[hl] = a
            # Mirroring
            col = hl % 32
            hl_mirror = 0x3e0 - hl + 2 * col
            vram[hl_mirror] = a ^ 0x01
            bc += 1

    return vram

def is_wall(vram, col_vram, row_vram):
    # The active playfield in VRAM is shifted by 64 bytes (2 rows of 32 tiles)
    vaddr = row_vram * 32 + col_vram + 64
    if not (0 <= vaddr < 1024):
        return True
    tile = vram[vaddr]
    return tile >= 0xc0

def get_opposite_direction(direction):
    return direction ^ 2

def target_seeking_ai(vram, current_col, current_row, current_dir, target_col, target_row):
    """
    Simulates Z80 routine #2966:
    Finds the next direction for an actor based on straight-line distance to a target.
    Tie-breaking priority order: LEFT (3) > DOWN (2) > RIGHT (1) > UP (0)
    """
    opposite_dir = get_opposite_direction(current_dir)
    best_dist = 0xffffffff
    best_dir = current_dir

    # Candidate directions evaluated in order 0, 1, 2, 3
    # Any ties are won by the later evaluated direction because of the <= comparison
    for cand_dir in [0, 1, 2, 3]:
        if cand_dir == opposite_dir:
            continue

        dx, dy, _ = DIRECTIONS[cand_dir]
        cand_col = current_col + dx
        cand_row = current_row + dy

        if is_wall(vram, cand_col, cand_row):
            continue

        # Squared Euclidean distance
        dist2 = (target_col - cand_col) ** 2 + (target_row - cand_row) ** 2

        # Z80 comparison: if best_dist >= dist2, overwrite (since 'sbc hl,de' sets carry only if best_dist < dist2)
        if dist2 <= best_dist:
            best_dist = dist2
            best_dir = cand_dir

    return best_dir

def cabinet_to_vram(col_cab, row_cab):
    col_vram = row_cab - 2
    row_vram = col_cab + 2
    return col_vram, row_vram

def vram_to_cabinet(col_vram, row_vram):
    col_cab = row_vram - 2
    row_cab = col_vram + 2
    return col_cab, row_cab

def main():
    try:
        vram = parse_and_decode_vram()
        print("Playfield successfully decoded from ROM disassembly.")
    except Exception as e:
        print(f"Error decoding playfield: {e}")
        return

    # Attract mode gameplay demo starting positions (VRAM coordinates)
    # Pac-Man starts at col 24, row 16 (cabinet: col 14, row 26)
    pac_col, pac_row = 24, 16
    pac_dir = 0  # Starts moving LEFT (VRAM UP, which maps to Cabinet LEFT)

    # Pinky is inactive in the ghost house at col 15, row 16 (cabinet: col 14, row 17)
    pinky_col, pinky_row = 15, 16

    print("\n--- Starting Pac-Man Attract Mode Simulator ---")
    print(f"Pac-Man starts at Cabinet (col={14}, row={26}) [VRAM: col={pac_col}, row={pac_row}], moving LEFT")
    
    path = []
    visited = set()
    
    step = 0
    max_steps = 150  # Limit simulation steps to prevent infinite loop
    
    current_col, current_row = pac_col, pac_row
    current_dir = pac_dir

    print("\nTracing Pac-Man's route (cabinet coordinates):")
    
    md_lines = [
        "# Pac-Man Attract Mode Turn-by-Turn Route",
        "",
        "This document contains the exact cabinet-space turn sequence and Z80 playfield coordinates extracted from the Z80 ROM disassembly (`pac-man.asm`) by simulating the game's actual target-seeking AI.",
        "",
        "## Z80 Coordinate Transposition Mapping",
        "- `col_cabinet = row_vram - 2`",
        "- `row_cabinet = col_vram + 2`",
        "",
        "## Complete Step-by-Step Trajectory Table",
        "",
        "| Step | Cabinet Col | Cabinet Row | VRAM Col | VRAM Row | Direction | Action / Turn |",
        "| :--- | :---: | :---: | :---: | :---: | :---: | :--- |"
    ]
    
    steps_history = []
    
    while step < max_steps:
        cab_col, cab_row = vram_to_cabinet(current_col, current_row)
        dir_name = DIRECTIONS[current_dir][2]
        path.append((cab_col, cab_row, dir_name))
        
        # Determine Pac-Man's target in Attract Mode (Pinky is inactive)
        # Target = 2 * Pac-Man - Pinky
        target_col = 2 * current_col - pinky_col
        target_row = 2 * current_row - pinky_row
        
        # Decide next direction at the upcoming tile center
        next_dir = target_seeking_ai(vram, current_col, current_row, current_dir, target_col, target_row)
        
        # Move Pac-Man to the next tile in the chosen direction
        dx, dy, _ = DIRECTIONS[next_dir]
        next_col = current_col + dx
        next_row = current_row + dy
        
        cab_next_col, cab_next_row = vram_to_cabinet(next_col, next_row)
        turn_str = ""
        if next_dir != current_dir:
            turn_str = f"**Turn {DIRECTIONS[next_dir][2]}**"
        
        md_lines.append(f"| {step:03d} | {cab_col} | {cab_row} | {current_col} | {current_row} | {dir_name} | {turn_str} |")
        steps_history.append((step, cab_col, cab_row, current_col, current_row, dir_name, turn_str))
        
        print(f"Step {step:03d}: Cabinet (col={cab_col}, row={cab_row}) -> Turn {DIRECTIONS[next_dir][2]} to (col={cab_next_col}, row={cab_next_row})")
        
        # Detect loops/capture
        state = (next_col, next_row, next_dir)
        if state in visited:
            print(f"\nCaptured / Loop detected at Step {step} at Cabinet (col={cab_next_col}, row={cab_next_row})")
            md_lines.append(f"| {step+1:03d} | {cab_next_col} | {cab_next_row} | {next_col} | {next_row} | {DIRECTIONS[next_dir][2]} | **Captured / Loop Detected** |")
            break
        visited.add(state)
        
        current_col, current_row = next_col, next_row
        current_dir = next_dir
        step += 1

    md_lines.append("")
    md_lines.append("## Summary of Unique Turns")
    md_lines.append("")
    
    print("\n--- Summary of Pac-Man's turns ---")
    turn_list = []
    for i, (col, row, d) in enumerate(path):
        if i == 0 or path[i-1][2] != d:
            turn_list.append(f"- **{d}** at Cabinet `(col={col}, row={row})`")
            print(f"- {d} at Cabinet (col={col}, row={row})")
            
    md_lines.extend(turn_list)
    
    # Write to attract_mode_route.md in the artifacts directory
    output_path = "attract_mode_route.md"
    try:
        with open(output_path, "w") as f:
            f.write("\n".join(md_lines))
        print(f"\nSuccessfully wrote detailed route document to: {output_path}")
    except Exception as e:
        print(f"Error writing markdown route file: {e}")

if __name__ == "__main__":
    main()
