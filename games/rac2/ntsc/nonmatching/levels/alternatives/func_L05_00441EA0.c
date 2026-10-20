/* Retained wave8/w10 draft, rechecked 2026-10-15.
   Compiler rebuilt from master bb4c620; -O2 -G0 -ffunction-sections.
   140 bytes versus 140; 0 of 4 placements match. First instruction difference at +0x4 (parameter copy versus table load).
   No matching credit or behavioral validation. Original C body retained;
   missing declarations supplied and calls named from the current catalogue.
   Reproduce: copy this file to src/levels/shared/ with the same filename,
   remove its original INCLUDE_ASM marker, and run tools/check.py func_L05_00441EA0 --diff.
   Reopen when the compiler/source can reproduce the measured differences. */
typedef int TI128 __attribute__((mode(TI)));
typedef struct { float x, y, z; unsigned int w; } Vec4_L05;
extern char *D_L05_001B1EC4;
extern void func_002833F0(char *, char *);

/* Copies the bound record's two float triples into the object's 0x20 block and a
   stack vector. */
void func_L05_00441EA0(char *object)
{
    Vec4_L05 local;
    char *table = D_L05_001B1EC4;
    char *record = *(char **)(object + 0x70);
    Vec4_L05 *src = (Vec4_L05 *)(table + (*(short *)(object + 0x84) << 5));
    Vec4_L05 *dst = (Vec4_L05 *)(record + 0x20);

    dst->x = src->x;
    dst->y = src->y;
    dst->w = 0;
    dst->z = src->z;
    *(TI128 *)(object + 0x30) = *(TI128 *)dst;
    {
        Vec4_L05 *second = (Vec4_L05 *)(table + (*(short *)(object + 0x84) << 5) + 0x10);

        local.x = second->x;
        local.y = second->y;
        local.w = 0;
        local.z = second->z;
    }
    func_002833F0(object, (char *)&local);
}
