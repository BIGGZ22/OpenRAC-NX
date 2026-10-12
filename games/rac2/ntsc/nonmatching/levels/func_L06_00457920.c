/* func_L06_00457920 (168 bytes) -- ABANDON (172 bytes; LICM wall).
 * retail materialises the constant argument address INSIDE the loop
 *   lui t2,0x1B ; addiu t2,t2,-0x70E0   (D_L06_001A8F20)
 * and hoists only the function pointer into $s3:
 *   lui v0,0x45 ; addiu s3,v0,0x79C8    (func_L06_004579C8)
 * gcc hoists the address constant too (into $s3, with the pointer in $s4),
 * which shifts the whole frame and adds one instruction, and it also fills the
 * loop back-edge delay slot with a hoisted `lh a2,0xAA(s1)` (bnel) instead of
 * the retail plain `bnez`. Tried both `for` and `do { } while` loop shapes.
 */
extern char D_L06_001A9020[];
extern char *func_31B0D0(char *, int, int, int, float, float, char *, char *, char *, int);
extern void func_32D8A8(char *, char *);
extern void func_L06_004579C8(void);

void func_L06_00457920(char *a0)
{
    int i;

    for (i = 0; i < 0xF; i++) {
        char *p = func_31B0D0(a0, 2, *(short *)(a0 + 0xAA), i, 0.0f, 10.0f,
                             a0 + 0x10, a0 + 0xF0, D_L06_001A9020, 0xFF);

        if (p != 0) {
            char *q = *(char **)(p + 0x68);

            func_32D8A8(q, q + 0x20);
            *(char **)(p + 0x64) = (char *)func_L06_004579C8;
        }
    }
}
