/* func_L00_002B9CA8 (184 bytes, 27 places, shared, default profile -O2 -G0)
   -- measured 0 of 27: candidate is 180 bytes, ONE instruction short.

   Walks eight 8-byte entries from D_0018C210: for each, calls func_00282978 and,
   when it returns 2, reads the entry's halfword at -2 (compared with 0x16) and
   clears the one at -4; the 0x16 case calls func_L00_002A61C8(*(u8 *)(D_00189E20
   + 0x149D), 1, 1, 0x32), otherwise func_L00_002B9B88(h, *(s16 *)(q + 0)).

   Register assignment is what is left. Retail keeps the base in `$s3` and derives
   three things from it - `$s0 = $s3 + 2` (the field pointer), `daddu $s2,$s3,$zero`
   (the call walker) and `addiu $v1,$s3,-0x23F0` (the D_00189E20 read, the two
   globals share their high half). This cc1 instead lets the base register walk
   (`addiu $s1,$s1,0x8`), drops the `$s2` copy by copy propagation, and so has no
   live base left to derive `$v1` from: it materialises D_00189E20 with its own
   `lui` in the preheader. One instruction, and it flips the whole allocation.

   Measured, one run each: the walker copy is NOT kept by spelling it explicitly
   (`char *p = b;`), nor by routing the u8 read through the base (`b[-0xF53]`, the
   same address), nor by a struct field for it, nor by reordering the declarations;
   the return of func_00282978 must stay `int` (an `s16` local adds a
   `sll`/`sra` pair, 188 bytes) and the countdown loop must be do/while or for
   (the index form is 168). Wave 7's nested-call lever was tried as well (both
   halfword reads inlined into func_L00_002B9B88, and the read folded into the
   comparison): still 180. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;

extern char D_0018C210[];
extern char D_00189E20[];
extern int func_00282978(void *);
extern void func_L00_002A61C8(u8, int, int, int);
extern void func_L00_002B9B88(s16, s16);

void func_L00_002B9CA8(void) {
    char *b = D_0018C210;
    char *p = b;
    char *q = b + 2;
    int n = 7;
    int r;
    s16 h;

    do {
        r = func_00282978(p);
        n--;
        p += 8;
        if (r == 2) {
            h = *(s16 *)(q - 4);
            *(u16 *)(q - 6) = 0;
            if (h == 0x16)
                func_L00_002A61C8(*(u8 *)(D_00189E20 + 0x149D), 1, 1, 0x32);
            else
                func_L00_002B9B88(h, *(s16 *)(q + 0));
        }
        q += 8;
    } while (n >= 0);
}
