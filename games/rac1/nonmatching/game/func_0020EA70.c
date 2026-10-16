/*
 * UpdateMobyGrids(moby, rect): moves a moby in the moby grid from the cells of its current rectangle
 * (moby + 0xA0: bytes x0, y0, x1, y1; negative = in no cell) to those of `rect` (MobyBuildMatrix's new
 * one; DeleteMoby's 0x80807F7F = none). As ReRAC's MobyGrid::update (crates/rc-game/src/
 * collision_query/mobys.rs; ISC License, Copyright (c) 2026 ReRAC contributors): it leaves the old
 * cells outside the new rectangle (the entry replaced by the cell's last one) and joins the new cells
 * outside the old one (appended), x inner, y outer. Here on the game's own storage: the grid D_001B7A60
 * is 64 x 64 cells of {u16 block, u8 count, u8 blocks} (cell x, y at + x * 4 + y * 0x100), each cell's
 * moby indices (moby + 0xAC, 16-bit) in its blocks of the list storage at D_001B7A60 + 0x4000 (32 bytes,
 * 16 entries, a block); a cell grows by doubling its blocks and shrinks by halving them (func_0020E9F0
 * allocates, func_0020E990 frees), its entries copied 32 bytes at a time. Retail traps (teq) when a
 * moby is missing from a cell it should be in; this returns there instead.
 */
extern unsigned char D_001B7A60[];
extern void func_0020E990(int block, int count);
extern int func_0020E9F0(int count);

#define GRID_OUTSIDE(r, x, y)                                                       \
    ((x) < (signed char)((r) & 0xFF) || (y) < (signed char)(((r) >> 8) & 0xFF) ||   \
     (x) > (signed char)(((r) >> 16) & 0xFF) || (y) > (signed char)(((r) >> 24) & 0xFF))

void func_0020EA70(unsigned char *m, unsigned int rect) {
    unsigned char *lists = D_001B7A60 + 0x4000;
    unsigned int idx = *(unsigned int *)(m + 0xAC);
    unsigned int old = *(unsigned int *)(m + 0xA0);
    int x, y;

    if ((int)old >= 0) {
        for (y = m[0xA1];; y++) {
            for (x = old & 0xFF;; x++) {
                if (GRID_OUTSIDE(rect, x, y)) {
                    unsigned char *cell = D_001B7A60 + x * 4 + y * 0x100;
                    unsigned short *p = (unsigned short *)(lists + *(unsigned short *)cell * 32);
                    unsigned short *end = p + cell[2];
                    int count, blocks;

                    while (p != end && *p != idx) {
                        p++;
                    }
                    if (p == end) {
                        return;
                    }
                    *p = end[-1];
                    end[-1] = 0;
                    count = cell[2] - 1;
                    blocks = cell[3];
                    cell[2] = (unsigned char)count;
                    if (count == 0 || (blocks != 1 && blocks * 8 - 4 - count > 0)) {
                        /* Halve its blocks: freed first, then the smaller run allocated and the
                           entries copied out of the freed blocks. */
                        int from = *(short *)cell;

                        func_0020E990(*(unsigned short *)cell, blocks);
                        blocks >>= 1;
                        cell[3] = (unsigned char)blocks;
                        *(unsigned short *)cell = 0;
                        if (blocks != 0) {
                            int to = func_0020E9F0(blocks);
                            unsigned int *src = (unsigned int *)(lists + from * 32);
                            unsigned int *dst = (unsigned int *)(lists + to * 32);
                            int n;

                            *(unsigned short *)cell = (unsigned short)to;
                            for (n = (signed char)cell[3] * 8; n > 0; n--) {
                                *dst++ = *src++;
                            }
                        }
                    }
                }
                if (x == m[0xA2]) {
                    break;
                }
            }
            if (y == m[0xA3]) {
                break;
            }
        }
    }

    *(unsigned int *)(m + 0xA0) = rect;
    if ((int)rect < 0) {
        return;
    }
    for (y = (rect >> 8) & 0xFF;; y++) {
        for (x = rect & 0xFF;; x++) {
            if (GRID_OUTSIDE(old, x, y)) {
                unsigned char *cell = D_001B7A60 + x * 4 + y * 0x100;
                int blocks = cell[3];

                if (blocks == 0 || blocks * 16 - cell[2] <= 0) {
                    /* Double its blocks (at least one): the new run allocated, then the old one
                       freed and its entries copied over. */
                    int grown = blocks * 2 > 1 ? blocks * 2 : 1;
                    int to, from;

                    cell[3] = (unsigned char)grown;
                    to = func_0020E9F0(grown);
                    from = *(unsigned short *)cell;
                    *(unsigned short *)cell = (unsigned short)to;
                    if (blocks != 0) {
                        unsigned int *src = (unsigned int *)(lists + from * 32);
                        unsigned int *dst = (unsigned int *)(lists + to * 32);
                        int n;

                        func_0020E990(from, blocks);
                        for (n = (cell[3] >> 1) * 8; n > 0; n--) {
                            *dst++ = *src++;
                        }
                    }
                }
                {
                    int count = cell[2];

                    cell[2] = (unsigned char)(count + 1);
                    *(unsigned short *)(lists + *(unsigned short *)cell * 32 + count * 2) = (unsigned short)idx;
                }
            }
            if (x == m[0xA2]) {
                break;
            }
        }
        if (y == m[0xA3]) {
            break;
        }
    }
}
