/* CollCellLookup: finds the leaf word of a collision cell. The mesh holds three
   levels indexed by z, then y, then x: each level has a base and a count (both
   16-bit) and an array of offsets (16-bit at the root and slab levels, 32-bit
   at the row level). A cell outside a level's range, or with a zero offset,
   gives 0 (empty). The leaf word is returned as stored. */
unsigned int func_L00_001EFF50(unsigned char *mesh, int x, int y, int z) {
    int i;
    unsigned int off;
    unsigned char *slab;
    unsigned char *row;

    i = z - *(unsigned short *)(mesh + 0);
    if (i < 0 || *(unsigned short *)(mesh + 2) - i <= 0) {
        return 0;
    }
    off = *(unsigned short *)(mesh + 4 + i * 2);
    if (off == 0) {
        return 0;
    }
    slab = mesh + (off << 2);

    i = y - *(unsigned short *)(slab + 0);
    if (i < 0 || *(unsigned short *)(slab + 2) - i <= 0) {
        return 0;
    }
    off = *(unsigned int *)(slab + 4 + i * 4);
    if (off == 0) {
        return 0;
    }
    row = mesh + off;

    i = x - *(unsigned short *)(row + 0);
    if (i < 0 || *(unsigned short *)(row + 2) - i <= 0) {
        return 0;
    }
    return *(unsigned int *)(row + 4 + i * 4);
}
