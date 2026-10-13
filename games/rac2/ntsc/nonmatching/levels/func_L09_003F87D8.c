/* attempt kept for the next worker - func_L09_003F87D8
   closest attempt: retail re-materialises the D_L09_001B285C address inside the loop,
   gcc hoists the lui out of it; the loop tail also differs.
   verify with: tools/check.py func_L09_003F87D8 --diff
*/
extern unsigned int D_L09_001FB560[];
extern char *D_L09_001B285C;

void func_L09_003F87D8(char *object, int index)
{
    unsigned short *p;
    unsigned int entry;

    if (index == 0xFF)
        return;
    entry = D_L09_001FB560[index];
    if (entry == 0)
        return;
    p = (unsigned short *)entry;
    do {
        char *q = (char *)(D_L09_001B285C + (*p & 0x7FFF) * 256);

        if (*(signed char *)(q + 0x20) >= 0 && *(short *)(q + 0xAA) == 0xD0F)
            *(unsigned char *)(*(char **)(q + 0x68) + 0x2EA) = 1;
        ++p;
    } while (*(short *)p >= 0);
}
