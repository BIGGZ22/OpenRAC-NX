/* 0 of 2 places match - last line: 12_todano: 80 bytes, expected 84
   One compile. The three-way guard chain is not the problem: this cc1 converts the
   second `if (a2 != 0)` into `beql $a2,$zero` and puts the next block's load in the
   annulled delay slot, where retail keeps a plain `bnez` and a separate load. The
   confirmed annulled-slot wall, so no further shapes were tried. */

extern void func_331870();
extern void func_32F848();
extern void func_32F978(float);

void func_L12_00427C58(int a0, int a1, int a2) {
    if (D_L12_001A8FF0_1 != 0) {
        func_331870();
        return;
    }
    if (a2 != 0) {
        func_32F848();
        return;
    }
    func_32F978(*(float *)(*(int *)(a0 + 0x68) + 0x33C));
}
