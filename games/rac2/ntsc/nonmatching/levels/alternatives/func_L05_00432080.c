/* Retained wave8/w10 draft, rechecked 2026-10-15.
   Compiler rebuilt from master bb4c620; -O2 -G0 -ffunction-sections.
   200 bytes versus 208; 0 of 4 placements match. Field-store scheduling and loop control differ.
   No matching credit or behavioral validation. Original C body retained;
   missing declarations supplied and calls named from the current catalogue.
   Reproduce: copy this file to src/levels/shared/ with the same filename,
   remove its original INCLUDE_ASM marker, and run tools/check.py func_L05_00432080 --diff.
   Reopen when the compiler/source can reproduce the measured differences. */
typedef int TI128 __attribute__((mode(TI)));
extern char *func_0029EFD0(int);
extern void func_002A0F48(char *);

/* Spawns the 0x10E3 object: three field stores, a bound block copy and a scale. */
char *func_L05_00432080(char *a0, float f12)
{
    char *obj = func_0029EFD0(0x10E3);

    if (obj != 0) {
        int i;

        *(unsigned char *)(obj + 0x20) = 0;
        *(unsigned short *)(obj + 0x34) = *(unsigned short *)(obj + 0x34) | 0x108;
        *(unsigned char *)(obj + 0x23) = 0x60;
        *(unsigned char *)(obj + 0x31) = 1;
        *(unsigned short *)(obj + 0x32) = 0xFF;
        *(unsigned char *)(obj + 0x30) = 0xFF;
        *(char **)(*(char **)(obj + 0x68)) = a0;
        *(TI128 *)(obj + 0x10) = *(TI128 *)(a0 + 0x10);
        for (i = 0; i < 3; i++)
            *(TI128 *)(obj + 0xC0 + i * 16) = *(TI128 *)(a0 + 0xC0 + i * 16);
        *(float *)(obj + 0x2C) = *(float *)(*(int *)(obj + 0x24) + 0x24) * f12;
        func_002A0F48(obj);
    }
    return obj;
}
