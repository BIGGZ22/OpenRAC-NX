/* func_L00_003CA650, 52 bytes, 27 placements.  ABANDON - catalogue defect, the real body is
   eight bytes further on.
   The recorded extent opens on this lot's `addiu $sp,$sp,+N; nop` family: the first two words
   are `addiu $sp,$sp,0x60; nop`, which is a stray epilogue fragment, not a prologue. From +8
   (0x003CA658) the words are a coherent body, so the C below is the reconstruction of that
   body and the entry should be re-derived by the maintainers (the defect is the same one the
   brief lists, measured here on four more functions of this lot).
   The body: while the object's byte +0x20 is 1, call func_3CA688; nothing else. It has no
   caller-visible result and no frame of its own beyond the return address. */

#include "include_asm.h"

extern void func_3CA688(void);

void func_L00_003CA650(char *object)
{
    if (*(unsigned char *)(object + 0x20) == 1)
        func_3CA688();
}
