/* functional: high. Logic is exact: if the flag D_001612E8 is set, hand D_0016130C + 0xD9168 to func_0023E710 and clear the flag, then return 0. Omitted: the retail `sync` and `ei` (memory barrier and interrupt re-enable), which have no C form and nothing to do in the PC port. */
extern void func_0023E710(volatile int *vb);
extern int D_001612E8 MACRO_ADDR;
extern char *D_0016130C MACRO_ADDR;

int func_0023C910(void) {
    if (D_001612E8 != 0) {
        func_0023E710((volatile int *)(D_0016130C + 0xD9168));
        D_001612E8 = 0;
    }
    return 0;
}
