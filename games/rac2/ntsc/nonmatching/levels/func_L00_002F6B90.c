/* from src/levels/shared/L00_002F4400.c: runs the callback at D_L00_0027DD00[a0 & 0xF].f10
 * and returns 1, else 0. Best of 3 compiles (f1/f3 identical): exact 80 bytes, only three
 * words differ - this cc1 reuses the pointer register for the loaded callback
 * (`lw $a0,0x10($a0)` + `jalr $a0`, the pointer dying at the load) where retail loads it
 * into `$v0` (`lw $v0,0x10($a0)` + `jalr $v0`, $a0 dead since the address add). Same wall
 * as the tour-1 allocation pairs: the local-alloc tie, no source shape tried (nested if,
 * early returns, `void (*fn)(void)` local, ternary select) moves it. */
typedef struct {
    char pad_00[0x10];
    void (*f10)(void);
    char pad_14[0x7C];
} Slot_r002F6B90;

extern Slot_r002F6B90 D_L00_0027DD00[];

int func_L00_002F6B90(unsigned int a0) {
    Slot_r002F6B90 *p = &D_L00_0027DD00[a0 & 0xF];
    int r = 0;
    if (p != 0 && p->f10 != 0) {
        p->f10();
        r = 1;
    }
    return r;
}
