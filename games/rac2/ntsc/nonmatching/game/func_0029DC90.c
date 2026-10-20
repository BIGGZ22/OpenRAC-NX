/* from src/game/0029BCC8.c: 60-byte near miss; the candidate is 64 bytes and
   the branch shape differs.  Same wall as func_0029DCD0: the 64-bit result
   (daddu $v0,$zero,$zero in the delay slots) and the three nested tests are
   merged into one bgez + movz/movn chain by this cc1.  The tested globals
   are the boot-resident D_001395C1, D_001A7B05 and D_001A7B0C (bytes). */
long long func_0029DC90(void) {
    long long result;
    if (*(unsigned char *)0x001395C1 != 0) {
        if (*(unsigned char *)0x001A7B05 != 0) {
            if (*(unsigned char *)0x001A7B0C != 0)
                result = 1;
            else
                result = 0;
        } else
            result = 0;
    } else
        result = 0;
    return result;
}
