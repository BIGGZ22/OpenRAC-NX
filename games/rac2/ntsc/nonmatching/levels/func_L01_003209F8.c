/* func_L01_003209F8 (240 bytes, 2 places) - decoded, no C measured this wave.
 *
 * Guards: the word at 0x001A8F40 must be 2 (absolute lui $v1,0x1B / lw -0x70C0), the byte
 * at 0x001A7A40 nonzero, then lh $s2,0xAA($a0) and movn $s2,$a2,$v1 with $v1 = nor
 * $zero,$a2 - i.e. the caller's third argument conditionally replaces the id when the
 * caller's value is all-ones, the shape cc1 gives a cond ? a : b on a 32-bit value.
 * Loop over the table at 0x001BC080 (lui 0x1C / addiu -0x2E40) with its count in
 * lh 0x44($s3) and the cursor at +0x18C, comparing each entry's +0xAA against $s2 and then
 * against the byte at 0x001B1E61; on a hit func_307048(entry, s4, 0x001B1E60) (a1 is the
 * caller's second argument) and three swc1 $f20 to 0x001B1E80/84/88, each with its own
 * lui $at,0x1B rematerialisation.
 * Nothing here is a known wall; it simply was not attempted in this wave.  Its sibling
 * func_L01_00394148 (same 0x001A8F40/0x001A7A40 guards, same 0x001B1E60/80/84/88 block) is
 * deposited with the loop body byte-identical, so the two rows share a preamble shape.
 */


/* Attempted C (does not match: 272 bytes vs 240). */
extern void func_0029F978(void *a0, int a1, void *a2);

extern int D_L01_001A8F40;
extern unsigned char D_001A7A40;
extern void *D_L01_001BD1C0[];
extern unsigned char D_L01_001B1E61;
extern float D_L01_001B1E80;
extern float D_L01_001B1E84;
extern float D_L01_001B1E88;
extern void *D_L01_001B1E60;

void func_L01_003209F8(void *entry, void *arg1, int id, float value)
{
    short count;
    int i;
    short wanted;
    void **table;
    void **cursor;
    void *p;

    if (D_L01_001A8F40 != 2)
        return;
    if (D_001A7A40 == 0)
        return;
    wanted = *(short *)((char *)entry + 0xAA);
    if (id != -1)
        wanted = id;
    table = D_L01_001BD1C0;
    count = *(short *)((char *)table + 0x44);
    if (count <= 0)
        return;
    i = 0;
    cursor = (void **)((char *)table + 0x18C);
    do {
        p = *cursor;
        if (*(short *)((char *)p + 0xAA) == wanted && D_L01_001B1E61 == 0) {
            func_0029F978(p, arg1, D_L01_001B1E60);
            D_L01_001B1E80 = value;
            D_L01_001B1E84 = value;
            D_L01_001B1E88 = value;
        }
        count = *(short *)((char *)table + 0x44);
        i++;
        cursor++;
    } while (i < count);
}

