/* func_L14_003D18C8, 200 bytes, 1 placement.  Nearest: 204 bytes, the frame size.
   Reads the 16 bytes at a2, hands them and a3 to func_2B04E0 (whose float return drives
   everything else), fills a stack object with func_2AAA00(obj, s2 + 0xB4, 0, v,
   *(s2 + 0x14), *(s2 + 0x18), 3.15606951713562f = 0x4049FD0B), copies *(s2 + 0xB4) into the
   +8 slot of the threaded buffer through func_2833F0(buf, tmp), runs the a0 + 0xC0 transform
   through func_283738, and returns func_282C48(obj, v) - func_282C48(obj, *(f32 *)(s2 + 0xB4)).
   Measured gaps: (1) retail's local area is 0x50 bytes - buffers at 0x00 and 0x10, the
   func_2AAA00 object at 0x40 - against 0x30 here, so the frame is 0x80 against 0x60 and every
   save slot moves; (2) retail mutates the object pointer in place (`addiu $s1,$s1,0xC0` at
   +0x80, after the func_2AAA00 call) where this cc1 hoists a separate a0 + 0xC0 pseudo into
   $s0 from the entry.  The lq/sq pair sits at offset 0 on both sides, so the wave-6 sq
   folding wall does not apply here and the body is otherwise retail's.  Three frame spellings
   (a 0x30-byte object with its zero at +0x20, a separately zeroed TI128, in-place mutation)
   all stay at 204 bytes. */

#include "include_asm.h"
#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

extern f32 func_002B04E0(char *a0, void *a1, char *a2);

extern void func_002AAA00(void *a0, char *a1, s32 a2, f32 f12, f32 f13, f32 f14, f32 f15);

extern void func_002833F0(void *a0, void *a1);

extern void func_00283738(void *a0, void *a1, void *a2);

extern f32 func_00282C48(char *a0, f32 f12);

f32 func_L14_003D18C8(char *a0, char *a1, TI128 *a2, char *a3)
{
    TI128 tmp = *a2;
    char *s2 = a1;
    char buf1[0x10];
    char buf2[0x10];
    char rot[0x30];
    f32 v;

    *(TI128 *)buf1 = tmp;
    v = func_002B04E0(a0, buf1, a3);
    *(s32 *)(rot + 0x20) = 0;
    func_002AAA00(rot + 0x20, s2 + 0xB4, 0, v, *(f32 *)(s2 + 0x14), *(f32 *)(s2 + 0x18),
                  3.15606951713562f);
    *(f32 *)(buf1 + 8) = *(f32 *)(s2 + 0xB4);
    func_002833F0(buf2, buf1);
    a0 = a0 + 0xC0;
    func_00283738(a0, a0, buf2);
    return func_00282C48(a0, v) - func_00282C48(a0, *(f32 *)(s2 + 0xB4));
}
