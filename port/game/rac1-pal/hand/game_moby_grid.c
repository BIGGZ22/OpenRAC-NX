/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native-only PAL 0020E990..0020ED40. The executable grid updater includes
 * the register-only tails at 0020EB50/0020EBE8 and its resize paths.
 * Reconstructed from retail assembly; not a matching PS2 decompilation.
 * Cells: u16 first block, u8 population, u8 capacity in 32-byte blocks.
 * A 64x64 cell table precedes 1536 pool blocks and a 192-byte bitmap. */
extern unsigned char exe_grid[] __asm__("D_001B7A60");
extern unsigned int exe_grid_used[] __asm__("D_001C7A60");
extern void exe_grid_trap(void) __asm__("func_001F9978");

void func_0020E990(int first, int blocks) {
    int i;
    /* Invalid ranges are fatal instead of accessing past the native pool. */
    if (first < 0 || blocks <= 0 || first > 1536 - blocks) {
        exe_grid_trap();
        return;
    }
    for (i = first; i < first + blocks; ++i) {
        unsigned int bit = 1u << (i & 31);
        if (!(exe_grid_used[i >> 5] & bit)) {
            exe_grid_trap();
            return;
        }
        exe_grid_used[i >> 5] &= ~bit;
    }
}

int func_0020E9F0(int blocks) {
    unsigned int mask;
    int word, shift;
    /* The byte-sized populations use powers of two through 16 blocks.
     * Retail scans indefinitely on exhaustion; the port must fail loudly. */
    if (blocks < 1 || blocks > 16 || (blocks & (blocks - 1))) {
        exe_grid_trap();
        return 0;
    }
    mask = (1u << blocks) - 1u;
    for (word = 0; word < 48; ++word) {
        for (shift = 0; shift < 32; shift += blocks) {
            unsigned int bits = mask << shift;
            if ((exe_grid_used[word] & bits) == 0) {
                exe_grid_used[word] |= bits;
                return word * 32 + shift;
            }
        }
    }
    exe_grid_trap();
    return 0;
}

static void exe_grid_copy(int to, int from, int blocks) {
    unsigned char* pool = exe_grid + 0x4000;
    unsigned char block[32];
    int i, j;
    /* Retail loads both quadwords before writing a block; source and
     * destination may coincide after a shrink frees the old allocation. */
    for (i = 0; i < blocks; ++i) {
        for (j = 0; j < 32; ++j) block[j] = pool[(from + i) * 32 + j];
        for (j = 0; j < 32; ++j) pool[(to + i) * 32 + j] = block[j];
    }
}

static int exe_grid_contains(unsigned int rect, int x, int y) {
    /* Equivalent to the signs of retail's saturated signed-byte deltas
     * for the valid 0..63 cell coordinates and the negative sentinel. */
    return !(rect & 0x80000000u) && x >= (int)(rect & 255)
        && y >= (int)((rect >> 8) & 255) && x <= (int)((rect >> 16) & 255)
        && y <= (int)(rect >> 24);
}

static int exe_grid_valid(unsigned int rect) {
    /* Retail checks no range. Its callers (func_0020ED48, func_L00_00251E30) drop a rect whose
     * near corner is off the grid, not one whose far column is 64: a moby whose sphere reaches
     * past x 1024 (on Hoven, one knocked back off the world's edge by func_L00_0025D6F0, which
     * moves it once more after flagging it 0x100 at 1021) reaches column 64 of its rows, which
     * retail's cell address (x * 4 + y * 256) makes column 0 of the next row. That stays in the
     * table and is kept; a cell past the table's end (row 64, or column 64 of row 63) is fatal. */
    return (rect & 0x80000000u) || ((rect & 255) <= ((rect >> 16) & 255)
        && ((rect >> 8) & 255) <= (rect >> 24)
        && ((rect >> 16) & 255) <= 64 && (rect >> 24) * 64 + ((rect >> 16) & 255) < 4096);
}

void func_0020EA70(unsigned char* moby, unsigned int next) {
    unsigned int old = *(unsigned int*)(moby + 0xA0);
    unsigned int id = *(unsigned int*)(moby + 0xAC);
    unsigned short* pool = (unsigned short*)(exe_grid + 0x4000);
    int x, y, i, first, capacity, fresh;
    if (!exe_grid_valid(old) || !exe_grid_valid(next)) {
        exe_grid_trap();
        return;
    }
    if (!(old & 0x80000000u)) {
        for (y = (old >> 8) & 255; y <= (int)(old >> 24); ++y) {
            for (x = old & 255; x <= (int)((old >> 16) & 255); ++x) {
                unsigned char* cell = exe_grid + (y * 64 + x) * 4;
                unsigned short* list;
                if (exe_grid_contains(next, x, y)) continue;
                first = *(unsigned short*)cell;
                list = pool + first * 16;
                for (i = 0; i < cell[2] && (unsigned int)list[i] != id; ++i) {}
                if (i == cell[2]) {
                    exe_grid_trap();
                    return;
                }
                list[i] = list[cell[2] - 1];
                list[cell[2] - 1] = 0;
                --cell[2];
                capacity = cell[3];
                if (cell[2] == 0 || (capacity != 1 && cell[2] < capacity * 8 - 4)) {
                    func_0020E990(first, capacity);
                    capacity >>= 1;
                    *(unsigned short*)cell = 0;
                    cell[3] = (unsigned char)capacity;
                    if (capacity) {
                        fresh = func_0020E9F0(capacity);
                        *(unsigned short*)cell = (unsigned short)fresh;
                        exe_grid_copy(fresh, first, capacity);
                    }
                }
            }
        }
    }
    *(unsigned int*)(moby + 0xA0) = next;
    if (next & 0x80000000u) return;
    for (y = (next >> 8) & 255; y <= (int)(next >> 24); ++y) {
        for (x = next & 255; x <= (int)((next >> 16) & 255); ++x) {
            unsigned char* cell = exe_grid + (y * 64 + x) * 4;
            if (exe_grid_contains(old, x, y)) continue;
            capacity = cell[3];
            if (capacity == 0 || cell[2] >= capacity * 16) {
                cell[3] = (unsigned char)(capacity ? capacity * 2 : 1);
                fresh = func_0020E9F0(cell[3]);
                first = *(unsigned short*)cell;
                *(unsigned short*)cell = (unsigned short)fresh;
                if (capacity) {
                    func_0020E990(first, capacity);
                    exe_grid_copy(fresh, first, capacity);
                }
            }
            pool[*(unsigned short*)cell * 16 + cell[2]] = (unsigned short)id;
            ++cell[2];
        }
    }
}
