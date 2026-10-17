/* functional: medium. One-shot creation of the three cd semaphores: while D_001313E8 or D_001313EC is still -1, builds a CreateSema parameter block (fields 1 and 2 = 1, field 5 = 0), stores D_001313E8 and D_001313EC from two calls, then field 2 = 0 and D_001313E0 from a third, and clears D_001313F0. The block's other fields are zeroed here (retail leaves stack garbage), which cannot matter to the callee's documented fields. */
extern s32 D_001313E0_0B28 __asm__("D_001313E0");
extern s32 D_001313E8_0B28 __asm__("D_001313E8");
extern s32 D_001313EC_0B28 __asm__("D_001313EC");
extern s32 D_001313F0_0B28 __asm__("D_001313F0");
extern s32 func_00118C70_0B28(void *) __asm__("func_00118C70");

void func_00120B28(void) {
    s32 buf[8] = {0};
    if (D_001313E8_0B28 != -1 && D_001313EC_0B28 != -1) {
        return;
    }
    buf[5] = 0;
    buf[1] = 1;
    buf[2] = 1;
    D_001313E8_0B28 = func_00118C70_0B28(buf);
    D_001313EC_0B28 = func_00118C70_0B28(buf);
    buf[2] = 0;
    D_001313E0_0B28 = func_00118C70_0B28(buf);
    D_001313F0_0B28 = 0;
}
