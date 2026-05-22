# Pac-Man Attract Mode Turn-by-Turn Route

This document contains the exact cabinet-space turn sequence and Z80 playfield coordinates extracted from the Z80 ROM disassembly (`pac-man.asm`) by simulating the game's actual target-seeking AI.

## Z80 Coordinate Transposition Mapping
- `col_cabinet = row_vram - 2`
- `row_cabinet = col_vram + 2`

## Complete Step-by-Step Trajectory Table

| Step | Cabinet Col | Cabinet Row | VRAM Col | VRAM Row | Direction | Action / Turn |
| :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| 000 | 14 | 26 | 24 | 16 | LEFT |  |
| 001 | 13 | 26 | 24 | 15 | LEFT |  |
| 002 | 12 | 26 | 24 | 14 | LEFT |  |
| 003 | 11 | 26 | 24 | 13 | LEFT |  |
| 004 | 10 | 26 | 24 | 12 | LEFT |  |
| 005 | 9 | 26 | 24 | 11 | LEFT |  |
| 006 | 8 | 26 | 24 | 10 | LEFT |  |
| 007 | 7 | 26 | 24 | 9 | LEFT | **Turn DOWN** |
| 008 | 7 | 27 | 25 | 9 | DOWN |  |
| 009 | 7 | 28 | 26 | 9 | DOWN |  |
| 010 | 7 | 29 | 27 | 9 | DOWN | **Turn RIGHT** |
| 011 | 8 | 29 | 27 | 10 | RIGHT |  |
| 012 | 9 | 29 | 27 | 11 | RIGHT |  |
| 013 | 10 | 29 | 27 | 12 | RIGHT | **Turn DOWN** |
| 014 | 10 | 30 | 28 | 12 | DOWN |  |
| 015 | 10 | 31 | 29 | 12 | DOWN |  |
| 016 | 10 | 32 | 30 | 12 | DOWN | **Turn LEFT** |
| 017 | 9 | 32 | 30 | 11 | LEFT |  |
| 018 | 8 | 32 | 30 | 10 | LEFT |  |
| 019 | 7 | 32 | 30 | 9 | LEFT |  |
| 020 | 6 | 32 | 30 | 8 | LEFT |  |
| 021 | 5 | 32 | 30 | 7 | LEFT |  |
| 022 | 4 | 32 | 30 | 6 | LEFT |  |
| 023 | 3 | 32 | 30 | 5 | LEFT |  |
| 024 | 2 | 32 | 30 | 4 | LEFT |  |
| 025 | 1 | 32 | 30 | 3 | LEFT |  |
| 026 | 0 | 32 | 30 | 2 | LEFT |  |
| 027 | -1 | 32 | 30 | 1 | LEFT | **Turn UP** |
| 028 | -1 | 31 | 29 | 1 | UP |  |
| 029 | -1 | 30 | 28 | 1 | UP |  |
| 030 | -1 | 29 | 27 | 1 | UP | **Turn RIGHT** |
| 031 | 0 | 29 | 27 | 2 | RIGHT |  |
| 032 | 1 | 29 | 27 | 3 | RIGHT | **Turn UP** |
| 033 | 1 | 28 | 26 | 3 | UP |  |
| 034 | 1 | 27 | 25 | 3 | UP |  |
| 035 | 1 | 26 | 24 | 3 | UP | **Turn LEFT** |
| 036 | 0 | 26 | 24 | 2 | LEFT |  |
| 037 | -1 | 26 | 24 | 1 | LEFT | **Turn UP** |
| 038 | -1 | 25 | 23 | 1 | UP |  |
| 039 | -1 | 24 | 22 | 1 | UP |  |
| 040 | -1 | 23 | 21 | 1 | UP | **Turn RIGHT** |
| 041 | 0 | 23 | 21 | 2 | RIGHT |  |
| 042 | 1 | 23 | 21 | 3 | RIGHT |  |
| 043 | 2 | 23 | 21 | 4 | RIGHT |  |
| 044 | 3 | 23 | 21 | 5 | RIGHT |  |
| 045 | 4 | 23 | 21 | 6 | RIGHT | **Turn DOWN** |
| 046 | 4 | 24 | 22 | 6 | DOWN |  |
| 047 | 4 | 25 | 23 | 6 | DOWN |  |
| 048 | 4 | 26 | 24 | 6 | DOWN |  |
| 049 | 4 | 27 | 25 | 6 | DOWN |  |
| 050 | 4 | 28 | 26 | 6 | DOWN |  |
| 051 | 4 | 29 | 27 | 6 | DOWN | **Turn LEFT** |
| 052 | 3 | 29 | 27 | 5 | LEFT |  |
| 053 | 2 | 29 | 27 | 4 | LEFT |  |
| 054 | 1 | 29 | 27 | 3 | LEFT |  |
| 055 | 0 | 29 | 27 | 2 | LEFT |  |
| 056 | -1 | 29 | 27 | 1 | LEFT | **Turn DOWN** |
| 057 | -1 | 30 | 28 | 1 | DOWN |  |
| 058 | -1 | 31 | 29 | 1 | DOWN |  |
| 059 | -1 | 32 | 30 | 1 | DOWN | **Turn RIGHT** |
| 060 | 0 | 32 | 30 | 2 | RIGHT |  |
| 061 | 1 | 32 | 30 | 3 | RIGHT |  |
| 062 | 2 | 32 | 30 | 4 | RIGHT |  |
| 063 | 3 | 32 | 30 | 5 | RIGHT |  |
| 064 | 4 | 32 | 30 | 6 | RIGHT |  |
| 065 | 5 | 32 | 30 | 7 | RIGHT |  |
| 066 | 6 | 32 | 30 | 8 | RIGHT |  |
| 067 | 7 | 32 | 30 | 9 | RIGHT |  |
| 068 | 8 | 32 | 30 | 10 | RIGHT |  |
| 069 | 9 | 32 | 30 | 11 | RIGHT |  |
| 070 | 10 | 32 | 30 | 12 | RIGHT |  |
| 071 | 11 | 32 | 30 | 13 | RIGHT |  |
| 072 | 12 | 32 | 30 | 14 | RIGHT |  |
| 073 | 13 | 32 | 30 | 15 | RIGHT |  |
| 074 | 14 | 32 | 30 | 16 | RIGHT |  |
| 075 | 15 | 32 | 30 | 17 | RIGHT |  |
| 076 | 16 | 32 | 30 | 18 | RIGHT |  |
| 077 | 17 | 32 | 30 | 19 | RIGHT |  |
| 078 | 18 | 32 | 30 | 20 | RIGHT |  |
| 079 | 19 | 32 | 30 | 21 | RIGHT |  |
| 080 | 20 | 32 | 30 | 22 | RIGHT |  |
| 081 | 21 | 32 | 30 | 23 | RIGHT |  |
| 082 | 22 | 32 | 30 | 24 | RIGHT |  |
| 083 | 23 | 32 | 30 | 25 | RIGHT |  |
| 084 | 24 | 32 | 30 | 26 | RIGHT | **Turn UP** |
| 085 | 24 | 31 | 29 | 26 | UP |  |
| 086 | 24 | 30 | 28 | 26 | UP |  |
| 087 | 24 | 29 | 27 | 26 | UP | **Turn LEFT** |
| 088 | 23 | 29 | 27 | 25 | LEFT |  |
| 089 | 22 | 29 | 27 | 24 | LEFT |  |
| 090 | 21 | 29 | 27 | 23 | LEFT |  |
| 091 | 20 | 29 | 27 | 22 | LEFT |  |
| 092 | 19 | 29 | 27 | 21 | LEFT | **Turn UP** |
| 093 | 19 | 28 | 26 | 21 | UP |  |
| 094 | 19 | 27 | 25 | 21 | UP |  |
| 095 | 19 | 26 | 24 | 21 | UP | **Turn LEFT** |
| 096 | 18 | 26 | 24 | 20 | LEFT |  |
| 097 | 17 | 26 | 24 | 19 | LEFT |  |
| 098 | 16 | 26 | 24 | 18 | LEFT | **Turn DOWN** |
| 099 | 16 | 27 | 25 | 18 | DOWN |  |
| 100 | 16 | 28 | 26 | 18 | DOWN |  |
| 101 | 16 | 29 | 27 | 18 | DOWN | **Turn LEFT** |
| 102 | 15 | 29 | 27 | 17 | LEFT |  |
| 103 | 14 | 29 | 27 | 16 | LEFT |  |
| 104 | 13 | 29 | 27 | 15 | LEFT | **Turn DOWN** |
| 105 | 13 | 30 | 28 | 15 | DOWN |  |
| 106 | 13 | 31 | 29 | 15 | DOWN |  |
| 107 | 13 | 32 | 30 | 15 | DOWN | **Turn LEFT** |
| 108 | 12 | 32 | 30 | 14 | LEFT |  |
| 109 | 11 | 32 | 30 | 13 | LEFT |  |
| 110 | 10 | 32 | 30 | 12 | LEFT |  |
| 111 | 9 | 32 | 30 | 11 | LEFT | **Captured / Loop Detected** |

## Summary of Unique Turns

- **LEFT** at Cabinet `(col=14, row=26)`
- **DOWN** at Cabinet `(col=7, row=27)`
- **RIGHT** at Cabinet `(col=8, row=29)`
- **DOWN** at Cabinet `(col=10, row=30)`
- **LEFT** at Cabinet `(col=9, row=32)`
- **UP** at Cabinet `(col=-1, row=31)`
- **RIGHT** at Cabinet `(col=0, row=29)`
- **UP** at Cabinet `(col=1, row=28)`
- **LEFT** at Cabinet `(col=0, row=26)`
- **UP** at Cabinet `(col=-1, row=25)`
- **RIGHT** at Cabinet `(col=0, row=23)`
- **DOWN** at Cabinet `(col=4, row=24)`
- **LEFT** at Cabinet `(col=3, row=29)`
- **DOWN** at Cabinet `(col=-1, row=30)`
- **RIGHT** at Cabinet `(col=0, row=32)`
- **UP** at Cabinet `(col=24, row=31)`
- **LEFT** at Cabinet `(col=23, row=29)`
- **UP** at Cabinet `(col=19, row=28)`
- **LEFT** at Cabinet `(col=18, row=26)`
- **DOWN** at Cabinet `(col=16, row=27)`
- **LEFT** at Cabinet `(col=15, row=29)`
- **DOWN** at Cabinet `(col=13, row=30)`
- **LEFT** at Cabinet `(col=12, row=32)`