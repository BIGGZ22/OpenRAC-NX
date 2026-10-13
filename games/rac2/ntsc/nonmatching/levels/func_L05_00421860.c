/* Best attempt, 8 compiles. tools/check.py: "instruction at +0x58 differs" --
   the produced body is 168 bytes, the retail size, with every instruction in
   the retail order; the only difference is the register pair chosen inside the
   two counter blocks: this source gets base=$v0/value=$v1, the retail has
   base=$v1/value=$v0 (`lui $v1,%hi; addiu $v1,%lo; lw $v0,0x5C($v1)` ...).
   Tried without effect: `D[..] + 1` vs `D[..]++`, both increments inside both
   arms vs one shared after the if/else (the shared form comes out 164 bytes and
   shares more of the tail), and the counters reached through a `char[]` base
   with an explicit offset (172 bytes, worse).
   The `i < 0 || ...` shape, the `mult`-based `i * 0x28`, the load order at
   +0x10/+0x14 and the `func_00282D30`-style tail all reproduce exactly. */

extern int func_002ED8A0(int);

extern int D_L05_00221D10[];

/* 40-byte table entry; only the pair at +0x10 is measured. */
typedef struct {
    char pad[0x10];
    int a;
    int b;
    char pad2[0x10];
} Entry_L05_00421860;

extern Entry_L05_00421860 D_L05_002A1800[];

extern void func_L05_0041C118(char *);

extern void func_L05_0041D498(char *);

void func_L05_00421860(char *object)
{
    int i = func_002ED8A0(1);

    if (i < 0 || D_L05_002A1800[i].b == D_L05_002A1800[i].a) {
        func_L05_0041C118(object);
        D_L05_00221D10[0x5C / 4]++;
        D_L05_00221D10[0x70 / 4]++;
    } else {
        func_L05_0041D498(object);
        D_L05_00221D10[0x64 / 4]++;
        D_L05_00221D10[0x70 / 4]++;
    }
}
