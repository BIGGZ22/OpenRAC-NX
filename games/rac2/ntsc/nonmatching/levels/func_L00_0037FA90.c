/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0x88 differs.
   83 of 84 instructions: retail materialises the 16-byte constant at 0x001BF220 with
   `lui $v1,0x1C; addiu $v1,$v1,-0xDE0; lq $v0,0x0($v1); sq $v0,0x0($sp)`, this cc1 keeps
   the address in $v0 and the quadword temp in $v1 (the two registers swap). Every other
   instruction, including the prologue schedule (`lui $s0` between the saves, `sd $s2`
   before the `addiu`) and the whole tail, matches. A swap of the pair is not reachable by
   the declaration/assignment forms tried below (plain, alias __asm__, pointer, array,
   temporary); report the pair as the blocker.
   The pointer D_L00_001B8FF0 must stay a variable that survives the first call and is
   decremented in place, and it is also the third call's first argument. */

typedef unsigned char u8;
typedef int V4_7fa90 __attribute__((mode(TI)));

extern void func_00282F20(void *, void *, float);
extern void func_00282CC0(void *, void *, void *);
extern int func_002761F8(void *, void *, int, void *, int);
extern void func_00282CF0(void *, void *, void *);
extern float func_00282DB0(void *, void *);
extern float func_00283180(float);
extern void func_00282DD8(void *, void *, void *);
extern void func_002ACEC0(void *, void *, void *, float);
extern void func_00282D30(void *, void *, float);

extern V4_7fa90 D_L00_001BF220;
extern char D_L00_001B8FF0[];

extern struct {
    u8 pad000[0x760];
    u8 f760[0x1b30];
    void *f2290;
} D_00189E20;

/* Builds a 16-byte direction from the level's stored vectors, places it, then
   takes the near or the far branch on the reach against the level's limit. */
void func_L00_0037FA90(char *a0, char *a1, float f12, float f13, float f14)
{
    char *p = D_L00_001B8FF0;
    char local[16];
    char local2[16];
    float reach;

    func_00282F20(local, p, f13);
    p -= 0x230;
    func_00282CC0(local, local, p);
    if (func_002761F8(p, local, 0, D_00189E20.f2290, 0) != 0)
        *(V4_7fa90 *)local = D_L00_001BF220;
    func_00282CF0(a0, local, a1);
    func_00282F20(a0, a0, 1.0f);
    reach = func_00282DB0(a0, D_00189E20.f760);
    if (reach < func_00283180(f14)) {
        func_00282DD8(local2, a0, D_00189E20.f760);
        func_002ACEC0(a0, D_00189E20.f760, local2, f14);
        func_00282F20(a0, a0, f12);
    } else {
        func_00282D30(a0, a0, f12);
    }
}
