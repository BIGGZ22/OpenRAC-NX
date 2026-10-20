/* func_L23_003BFE90 (300 bytes, level 23 only) -- NEAR MISS, not integrated.
   Belongs in src/levels/23_damosel_orbit/L23_0035CE90.c (no $gp access).
   state: 296 bytes against retail's 300. The integer argument arg2 must be the third
   integer parameter (the unused int in front of it puts it in $a2, as retail reads it).
   The 1/60 factor is `D * 0.016666668f`, a mul.s by 0x3C888889 in retail.
   The one size difference is the 128-bit zero: retail writes it as
   `por $v0,$zero,$zero` (word 0x700014A9, which rabbitizer prints as INVALID) and then
   `sq $v0,0x50($sp)`. This toolchain folds the single-use zero into `sq $zero,0x50($sp)`
   (the fold in tools/compiler/transformers/fold_zero_ti_store.py). A named zero variable
   folds the same way. The prologue save order also differs (cc1 scheduling).
*/

typedef int TI16 __attribute__((mode(TI)));

struct Vecs {
    TI16 a;
    TI16 b;
    TI16 unused[3];
    TI16 c;
    TI16 d;
};

extern char D_00189E20[];

extern float D_L23_001B370C __attribute__((sda));

extern void func_00282D30(char *, char *, float);

extern void func_00282CC0(char *, char *, char *);

extern void func_0029FC20(char *, int, char *);

extern void func_002830C0(char *, char *, char *);

extern void func_00282CF0(char *, char *, char *);

extern void func_00282F20(char *, char *, float);

extern void func_L23_00429098(char *, char *, char *, int, float);

extern void func_002E5DA0(int, int, char *, int);

void func_L23_003BFE90(char *arg0, float arg1, int unused1, int arg2) {
    char *state = *(char **)(arg0 + 0x68);
    struct Vecs v;

    func_00282D30((char *)&v.a, *(char **)(D_00189E20 + 0x2290) + 0xE0, 3.0f);
    func_00282CC0((char *)&v.a, (char *)&v.a, D_00189E20 + 0x80);
    v.c = 0;
    ((float *)&v.c)[1] = 1.0f;
    ((float *)&v.c)[3] = 1.0f;
    if (arg2 != 0)
        ((float *)&v.c)[1] = -((float *)&v.c)[1];
    func_0029FC20(*(char **)(state + 0x2A0), arg2, (char *)&v.b);
    func_002830C0((char *)&v.c, (char *)&v.c, (char *)&v.b);
    func_00282CF0((char *)&v.d, (char *)&v.a, (char *)&v.c);
    func_00282F20((char *)&v.d, (char *)&v.d, D_L23_001B370C * 0.016666668f);
    func_L23_00429098(arg0, (char *)&v.d, (char *)&v.c, 0, arg1);
    func_002E5DA0(3, 0, arg0, 0x7E5);
}
