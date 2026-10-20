#include "common.h"

/*
 * Follow the chain of entries of the same kind as `index`: the byte table at
 * +0x29C marks each entry 2 (another link), 1 (a final one) or something else
 * (not a chain). Walk it while the kind is 2, then, for a kind-1 entry, add the
 * entry's own value to the object's first word.
 */
extern s32 func_00115E28(void);

s32 func_00120A30(s32 index, u8 *object) {
    s8 *kind = (s8 *) (object + 0x29C);
    u64 *chain = (u64 *) (object + 0x18);

    for (;;) {
        if (kind[index] != 2) {
            break;
        }
        index = (s32) chain[index];
    }
    if (kind[index] == 1) {
        return *(s32 *) object + *(s32 *) (object + 0x18 + index * 8);
    }
    return func_00115E28();
}
/* Near miss: the body is exact, the allocator is not.
   Loop and both index computations come out right; the candidate is 132 bytes
   where retail is 136 because this cc1 keeps the `index` parameter in $a0 and
   loads the value byte into $v0, while retail copies `index` to $a2 at entry
   (daddu $a2,$a0,$zero) and uses $a0 as the scratch. Rewording the loop as
   do/while, while and for gives the same 132 bytes three times; the game
   compiler does copy the parameter to $a2 but reaches 136 with another order.
   Unblocking it needs a source shape that makes the parameter's pseudo
   conflict with $a0, which no wording tried here did. */
