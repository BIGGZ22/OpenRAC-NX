/* Draft for the shared chain-walk family (168 bytes each, 07_siberius):
 *
 *   func_L07_003E5AC8   stores 1 at +0x244 of each walked object
 *   func_L07_003E5AC8   stores 1 at +0x244
 *   (this file)
 *   func_L07_003E5B70 (260 bytes) is the same walk with a return value
 *
 * The body walks a two-byte chain rooted at D_L07_001FAD60[id]:
 *
 *   id = *(unsigned char *)(arg + 0x21);
 *   if (id == 0xFF) return;
 *   p = D_L07_001FAD60[id];
 *   while (id != 0xFF && D_L07_001B22F8 >= id && p != 0 &&
 *          (*(short *)(p - 2) >= 0 || p == D_L07_001FAD60[id])) {
 *       o = (char *)D_L07_001B22DC + ((*(unsigned short *)p & 0x7FFF) << 8);
 *       if (o == 0) return;
 *       *(int *)(*(int *)(o + 104) + 0x2D0) = 1;
 *       p += 2;
 *   }
 *
 * Two measured source shapes, neither exact:
 *
 *  A) `while (cond) { work }` written with the goto rotation spelled out,
 *     plus `extern int D_L07_001B22DC __attribute__((sda));` and the same on
 *     D_L07_001B22F8: 168 bytes, the loop body and the whole test block come
 *     out byte-identical; the loop preheader at +0x10..+0x20 differs in
 *     register allocation ($v0/$v1 swapped between the byte and the table
 *     address) and retail keeps one extra dead `nop` at +0x2C, right after
 *     the `b` delay slot and before the loop label (mine has none).
 *     check.py says: "instruction at +0x10 differs".
 *
 *  B) the same with the byte read into a local `unsigned char id` before the
 *     table lookup: 164 bytes (four short), same preheader swap, no nop.
 *
 * The version kept below is B; A is the one whose size is exact.
 *
 * Never tried: reaching the table through a local pointer (would force
 * lui+addiu and lose the shared %hi), and a `for` loop with the increment in
 * the for-header.
 */

extern int D_L07_001B22DC __attribute__((sda));

extern int D_L07_001B22F8 __attribute__((sda));

extern char *D_L07_001FAD60[];

void func_L07_003E5AC8(char *arg)
{
    unsigned char id = *(unsigned char *)(arg + 0x21);
    char *p;

    if (id == 0xFF)
        return;
    p = D_L07_001FAD60[id];
    goto test;
body:
    {
        char *o = (char *)D_L07_001B22DC + ((*(unsigned short *)p & 0x7FFF) << 8);

        if (o == 0)
            return;
        *(int *)(*(int *)(o + 104) + 0x2D0) = 1;
        p += 2;
    }
test:
    id = *(unsigned char *)(arg + 0x21);
    if (id == 0xFF)
        return;
    if (D_L07_001B22F8 < id)
        return;
    if (p == 0)
        return;
    if (*(short *)(p - 2) >= 0)
        goto body;
    if (p == D_L07_001FAD60[id])
        goto body;
}
