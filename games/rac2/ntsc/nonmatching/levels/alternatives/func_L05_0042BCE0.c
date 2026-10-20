/* Retained wave8/w10 draft, rechecked 2026-10-15.
   Compiler rebuilt from master bb4c620; -O2 -G0 -ffunction-sections.
   188 bytes versus 200; 0 of 4 placements match. The saved-register set and frame differ.
   No matching credit or behavioral validation. Original C body retained;
   missing declarations supplied and calls named from the current catalogue.
   Reproduce: copy this file to src/levels/shared/ with the same filename,
   remove its original INCLUDE_ASM marker, and run tools/check.py func_L05_0042BCE0 --diff.
   Reopen when the compiler/source can reproduce the measured differences. */
typedef int TI128 __attribute__((mode(TI)));
extern char *func_0029EFD0(int);
extern void func_00283958(char *, char *);
extern void func_002A0F48(char *);

/* Spawns the 0x1226 object and primes it: two records, the bound copy and a scale. */
char *func_L05_0042BCE0(char *a0, char *a1, float f12, float f13)
{
    char *obj = func_0029EFD0(0x1226);

    if (obj != 0) {
        char *sub = *(char **)(obj + 0x68);
        char *block = obj + 0x10;

        *(unsigned char *)(obj + 0x30) = 0xFF;
        *(unsigned short *)(obj + 0x32) = 0x1FF;
        *(unsigned char *)(obj + 0x31) = 1;
        *(float *)(obj + 0x2C) = *(float *)(*(int *)(obj + 0x24) + 0x24) * f12;
        *(unsigned short *)(obj + 0x34) = (*(unsigned short *)(obj + 0x34) & 0xFFF8) | 0x100;
        *(TI128 *)block = *(TI128 *)a0;
        func_00283958(a1, obj + 0xC0);
        *(float *)(sub + 0x10) = f13;
        func_002A0F48(obj);
    }
    return obj;
}
