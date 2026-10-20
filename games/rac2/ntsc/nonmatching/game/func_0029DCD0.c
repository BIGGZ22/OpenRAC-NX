/* from src/game/0029BCC8.c: 44-byte near miss; the candidate is 40 bytes and
   the branch shape differs.  Retail keeps the redundant 64-bit zeroings
   (daddu $v0,$zero,$zero) of the result in the delay slots of the two
   branches, so the result is a 64-bit value; this cc1 merges the two tests
   into one (bgezl + movz $v1,$zero,$v0) and returns a 32-bit value.  The
   tested globals are the boot-resident D_001397C4 (int) and D_001395D5
   (byte). */
long long func_0029DCD0(void) {
    long long result;
    if (*(int *)0x001397C4 < 0) {
        if (*(unsigned char *)0x001395D5 != 0)
            result = 1;
        else
            result = 0;
    } else
        result = 0;
    return result;
}
