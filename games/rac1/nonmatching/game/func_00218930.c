/* CreatePart's allocator (hand-written): takes particle slot `cur` (gp-0x6B50) when it is below 0x800,
   marks it in the use bitmap D_001CDB00, then moves the cursor on: one past it when it was above the
   high-water mark (gp-0x6B4C), else to the first free bit from the start of the byte holding cur + 1
   (stopping at 0x800). Stores the cursor, raises the high-water mark to the slot taken, counts the
   particle (gp-0x6B48), clears the slot's first 32 bytes (retail stores the second quadword three
   times and never the last two) and puts the type byte at +0. Returns the slot (base gp-0x6B54 +
   slot * 64), or 0 when the cursor is at 0x800. The second argument is not read. */
extern int D_001601B0_i[] __asm__("D_001601B0");
extern int D_001601B4_i[] __asm__("D_001601B4");
extern int D_001601B8_i[] __asm__("D_001601B8");
extern char *D_001601AC_p[] __asm__("D_001601AC");
extern unsigned char D_001CDB00_b[] __asm__("D_001CDB00");

void *func_00218930(int type, int unused) {
    int cur = D_001601B0_i[0];
    int hw = D_001601B4_i[0];
    int slot = cur;
    int byte;
    int bit;
    char *p;

    if (cur >= 0x800) {
        return 0;
    }
    D_001CDB00_b[slot >> 3] |= (unsigned char)(1 << (slot & 7));
    cur = cur + 1;
    if (slot - hw <= 0) {
        byte = cur >> 3;
        cur = byte << 3;
        for (;;) {
            if (cur >= 0x800) {
                break;
            }
            bit = 1;
            while (bit & 0xFF) {
                if ((D_001CDB00_b[byte] & bit) == 0) {
                    goto found;
                }
                bit <<= 1;
                cur++;
            }
            byte++;
        }
    }
found:
    D_001601B0_i[0] = cur;
    D_001601B4_i[0] = hw > slot ? hw : slot;
    D_001601B8_i[0] = D_001601B8_i[0] + 1;
    p = D_001601AC_p[0] + slot * 64;
    ((long long *)p)[0] = 0;
    ((long long *)p)[1] = 0;
    ((long long *)p)[2] = 0;
    ((long long *)p)[3] = 0;
    *p = (char)type;
    return p;
}
