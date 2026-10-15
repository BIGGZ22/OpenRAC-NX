/* Near miss from src/levels/11_joba/L11_004465F8.c: the build refuses this body at its
   place — 156 bytes, expected 160. Kept as a starting point;
   not linked, not counted. The artificial volatile store qualifier was removed;
   the refusal above describes the historical unit, not this revised draft. */

/* Standalone declarations recovered from src/levels/11_joba/L11_004465F8.c. */
typedef signed int         s32;

typedef int TI128 __attribute__((mode(TI)));

extern void func_32EB98(void *a0, void *a1);

void func_L11_0044C180(char *a0)
{
    char *s0 = *(char **)(a0 + 0x68);
    char *p;
    char *dst;
    char *src;

    *(s32 *)(s0 + 0x4B8) = 0;
    *(s32 *)(s0 + 0x4B4) = (*(s32 *)(s0 + 0x4B4) + 1) % 4;
    if (*(s32 *)(s0 + 0x4B0) < 4)
        *(s32 *)(s0 + 0x4B0) = *(s32 *)(s0 + 0x4B0) + 1;
    func_32EB98(((*(s32 *)(s0 + 0x4B4) << 4) + 0x370) + s0, a0 + 0xF0);
    p = *(char **)(s0 + 0x4A4);
    if (p != 0) {
        src = p + 0x10;
        dst = s0 + (*(s32 *)(s0 + 0x4B4) << 4) + 0x270;
        *(TI128 *)dst = *(TI128 *)src;
    }
}
