/* Retained wave8/w10 draft, rechecked 2026-10-15.
   Compiler rebuilt from master bb4c620; -O2 -G0 -ffunction-sections.
   168 bytes versus 168; 0 of 4 placements match. First instruction difference at +0x14; object/block register allocation differs.
   No matching credit or behavioral validation. Original C body retained;
   missing declarations supplied and calls named from the current catalogue.
   Reproduce: copy this file to src/levels/shared/ with the same filename,
   remove its original INCLUDE_ASM marker, and run tools/check.py func_L05_004424B8 --diff.
   Reopen when the compiler/source can reproduce the measured differences. */
typedef int TI128 __attribute__((mode(TI)));
extern char *D_L05_001B1EC4;
extern char *D_L05_001B251C;
extern TI128 D_L05_001B3620;
extern TI128 D_L05_001BFB30;
extern void func_00282D30(char *, char *, char *, float);
extern void func_00282CC0(char *, char *, char *);
extern int func_00277F48(char *, int, char *, int, float);

/* Builds the object's 0x30 block from a bound record and, when the lookup succeeds,
   its colour constant. */
void func_L05_004424B8(char *object, float f12)
{
    float local[4];
    char *entry = D_L05_001B1EC4 + (*(short *)(object + 0x84) << 5);
    char *sub = *(char **)(entry + 0x1C);
    char *record = D_L05_001B251C + (*(int *)(sub + 0x20) << 8);

    func_00282D30((char *)local, object, entry, -f12);
    func_00282CC0((char *)&D_L05_001B3620, (char *)&D_L05_001B3620, (char *)local);
    if (func_00277F48(object + 0x30, 0x10, record, 0, 0.25f) != 0)
        *(TI128 *)(object + 0x30) = *(TI128 *)&D_L05_001BFB30;
}
