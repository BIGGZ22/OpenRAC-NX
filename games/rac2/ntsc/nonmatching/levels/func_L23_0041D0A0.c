/* func_L23_0041D0A0 (336 bytes, 1 place: level 23) -- NEAR MISS, not integrated.
   state: 332 bytes against retail's 336. Checked as tools/check.py does, in the -G8 unit
   (the file name *-g8.c gives the flags).
   What matches: the restart loop (random index, the scan of the list for an entry within
   45.0f of D_00189EA0, the predicate test, the second random index), the table load, the
   slots pointer in a local, the call to func_L23_00418650, the return value 1 and the epilogue.
   What differs:
   - the entry copy. Retail materialises the entry pointer (`sll; addu; addiu $v1,$v1,0x10`)
     and loads through offset 0 (`lq $v0,0($v1)`); this cc1 folds the +0x10 into the load's
     displacement (`lq $v1,0x10($v0)`), one instruction short. That is the constant-offset
     folding recorded for quadword copies: a pointer variable, an integer-typed sum and the
     plain `(char *)` sum all fold it the same way.
   - the callee-saved registers. Retail has arg0 in $fp, arg1 in $s7, slots in $s6 and the
     D_00189EA0 base in $s5; this cc1 gives $s5 to arg0, $fp to arg1, $s7 to slots and $s6 to
     the base. Copying the arguments into locals first did not change it.
*/

typedef int TI128 __attribute__((mode(TI)));

extern char *D_L23_001B1484 __attribute__((sda));

extern int *D_L23_001FF300[];

extern char D_00189EA0[];

extern float func_00282E48(char *, char *);

extern int func_002A77E0(int);

extern int func_L23_00419CA0(int);

extern void func_L23_00418650(char *, float);

int func_L23_0041D0A0(char *arg0, int arg1, float arg2) {
    char *state = *(char **)(D_L23_001B1484 + 0x68);
    int *slots = (int *)(state + 0x230);
    float best = 45.0f;
    int found;
    int w;
    int j;
    int *list;
    char *p;
    int k;

    for (;;) {
        found = 0;
        w = func_002A77E0(0x2F);
        list = D_L23_001FF300[slots[w]];
        p = (char *)list + 0x10;
        for (j = 0; j < *list; j++) {
            if (func_00282E48(D_00189EA0, p) < best) {
                found = 1;
                break;
            }
            p += 0x10;
        }
        if (found)
            continue;
        if (arg1 == 0)
            continue;
        if (func_L23_00419CA0(w) == 0)
            continue;
        k = func_002A77E0(*list - 1);
        *(TI128 *)arg0 = *(TI128 *)((char *)list + k * 16 + 0x10);
        func_L23_00418650(arg0, arg2);
        return 1;
    }
}
