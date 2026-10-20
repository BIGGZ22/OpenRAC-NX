/* func_002EE988: 368 bytes compiled with the project compiler (-O2 -G0), retail 372; 41 words differ.
   With a cc1 whose cse follows the rule below (scratch patch, see the end of this comment) the
   same C is 372 bytes and 10 words differ, so the remaining wall is compiler-side.

   What is solved here (and was not in the first attempt): the table pointer (`D_001B1C54` is
   `extern ... __attribute__((sda))`, which gives `lui $s0` / `lw $s0,%lo($s0)`), the saves
   ($s4 first, owner in $s4, the object pointer in $a0 and dead after `pos`, `pos` in $s3), the
   stack slots (d at 0x30, e at 0x40: five float[4] arrays declared in order), and the pointers to
   d and e living in callee-saved registers like retail's ($s2 = &d, $s0 = &e created late).
   `object = 0;` after `pos = ...` is a dead assignment: it stops this cse from rewriting the later
   `*(V4 *)pos = ...` stores to `0x60(object)`, which would keep `object` alive and shift every
   register (without it: 376 bytes).

   What differs with the project compiler: retail never lets cse rewrite the address of a
   TImode store from a register to register-plus-constant, and never rewrites a TImode address
   to a frame-pointer-based one (`sq $v0,0($s2)` with $s2 = $sp+0x30, `lq $v0,0($s0)` with
   $s0 = $sp+0x40); this cc1 does both (find_best_addr in cse.c), so d and e are accessed as
   `0x30($sp)` / `0x40($sp)` and no pointer register is shared. The rule that fits retail
   (cse.c: no find_best_addr for a TImode destination, and no frame-based candidate for any TImode
   address) changes no byte of any of the 535 game-compiled files of src/ (it is TImode-only) and
   is in /home/vbess/rac2-gc-decomp/work/p2/build/scratch/cc1build/cse-ti-address.diff.
   Under it the 10 remaining words are scheduling: retail keeps `lq`/`sq` adjacent (the first copy
   is `lq $v0,0($a1)` `sq $v0,0($s2)` before `addiu $s0,$sp,0x10`, this cc1 puts the `sq` in the
   delay slot of func_00282D30) and loads the last vector as `lq $v0,0($v1)` where this cc1 picks
   `lq $v1,0($v0)`. (Not the load latency: a one-cycle TImode load delay in the machine
   description leaves this function and func_002EE920 unchanged and changes 8 matched files;
   -fno-schedule-insns / -fno-schedule-insns2 make it worse.) */
typedef int V4 __attribute__((mode(TI)));
extern int func_002ED688(unsigned char *owner);
extern unsigned int func_002ED870(void);
extern void func_00282CF0(void *a0, void *a1, void *a2);
extern float func_00282DF0(char *a0);
extern void func_00282D30(char *a0, char *a1, float f);
extern void func_00282F20_10(V4 *, char *, float);
extern void func_00282CC0_26(char *, V4 *, char *);
extern int func_002761F8(char *a0, char *a1, int mode, char *self, int flag);
extern int func_00277258(char *a0, int mode, char *self, float scale);
extern V4 D_001BAFA0;
extern V4 D_001BAFB0;
extern char *D_001B1C54 __attribute__((sda));
extern int D_00214F90[];

void func_002EE988(char *owner)
{
    float a[4];
    float b[4];
    float c[4];
    float d[4];
    float e[4];
    char *object;
    char *pos;
    char *table;
    float len;

    if (func_002ED870() != 0)
        return;
    object = (char *)func_002ED688((unsigned char *)owner);
    if (object == 0)
        return;
    pos = object + 0x60;
    object = 0;
    table = D_001B1C54 + D_00214F90[0] * 128;
    func_00282CF0(a, pos, table + 0x30);
    len = func_00282DF0((char *)a);
    if (*(float *)table < len) {
        func_00282F20_10((V4 *)a, (char *)a, len - (len - *(float *)table));
        func_00282CC0_26(pos, (V4 *)a, table + 0x30);
    }
    if (func_002761F8(owner + 0x10, pos, 2, owner, 0)) {
        V4 *pd = (V4 *)d;
        V4 *pe;
        *pd = D_001BAFA0;
        func_00282D30((char *)b, (char *)&D_001BAFA0 + 0x20, 0.00097656f);
        func_00282F20_10((V4 *)c, (char *)b, 5.0f);
        pe = (V4 *)e;
        func_00282CC0_26((char *)pe, pd, (char *)c);
        *(V4 *)pos = *pe;
    }
    if (func_00277258(pos, 2, owner, 5.0f)) {
        *(V4 *)pos = D_001BAFB0;
    }
}
