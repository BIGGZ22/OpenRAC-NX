/* Near miss: 172 bytes / 27 places (retail 180).
   Last check.py line: "00_aranos_tutorial: instruction at +0x8 differs"
   (func_L00_003B69D8: 0 of 27 places match)

   Structure is right (two calls, then a two-iteration loop over the pointer array at
   a0->f68 walking q = f68+0x10 by 0x10).  What is wrong:
     - the callee-saved allocation: retail p -> s0, q -> s1, base -> s2, i -> s3;
       this source gives p -> s1, q -> s2 (both declaration orders tried);
     - one instruction short, so the loop body is not finalised.
   The single-base form "D_L00_001B8FF0" + "D_L00_001B8FF0 - 0x230" is required: two
   distinct D_ symbols make cc1 materialise both addresses (two registers + an extra
   instruction, 184 bytes). */

typedef struct NODE {
    char pad0[0x68];
    void *f68;
} NODE;

extern void func_3B6B70();

extern void func_3B6F88();

extern void func_2EE5B0();

extern void func_2EE108();

extern void func_2EE950();

extern void func_312B78();

extern char D_L00_001B8FF0[];

void func_L00_003B69D8(NODE *a0)
{
    char *q = (char *)a0->f68 + 0x10;
    void **p = a0->f68;
    int i;

    func_3B6B70();
    func_3B6F88(a0);
    for (i = 1; i >= 0; i--) {
        func_2EE5B0((char *)*p + 0x10, q, D_L00_001B8FF0);
        q += 0x10;
        func_2EE108((char *)*p + 0x10, (char *)*p + 0x10, D_L00_001B8FF0 - 0x230);
        func_2EE950((char *)*p + 0xC0, (char *)*p + 0xF0);
        func_312B78(*p);
        p++;
    }
}
