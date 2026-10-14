/* functional: medium. Sky shell draw loop (variant of func_0022C5A0): per shell scrolls its float by a gp global, sets a rotation, rebuilds the sky transform, scales the 3 rows, copies D_00160560 into the translation and calls func_0022C9A8. */
extern char *D_0016055C_2BEB0 __asm__("D_0016055C") MACRO_ADDR;
extern f32 D_00160500_2BEB0 SDATA(D_00160500);
extern u8 D_001D9A70_2BEB0[] __asm__("D_001D9A70");
extern u8 D_00160560_2BEB0[] __asm__("D_00160560") MACRO_ADDR;
extern void func_001FA190_2BEB0(void *) __asm__("func_001FA190");
extern void func_001F9BC0_2BEB0(void *) __asm__("func_001F9BC0");
extern void func_001FA238_2BEB0(void *, void *) __asm__("func_001FA238");
extern void func_001F9C48_2BEB0(void *, void *, f32) __asm__("func_001F9C48");
extern void func_0022C9A8_2BEB0(s32) __asm__("func_0022C9A8");

void func_0022BEB0(void) {
    f32 rot[4];
    f32 scale;
    f32 *p;
    f32 d;
    u8 *t;
    s32 i;

    *(s16 *)(D_0016055C_2BEB0 + 4) = 0;
    func_001FA190_2BEB0(D_001D9A70_2BEB0);
    func_001F9BC0_2BEB0(rot);
    for (i = 0; i < *(s16 *)(D_0016055C_2BEB0 + 6); i++) {
        p = (f32 *)(*(u8 **)(D_0016055C_2BEB0 + 0x20 + i * 4) + 8);
        scale = 1.0f;
        switch (i) {
        case 1:
            rot[1] = 0.02f;
            d = D_00160500_2BEB0 * 0.55f;
            scale = 3.0f;
            goto add;
        case 2:
            rot[1] = -0.02f;
            d = D_00160500_2BEB0 * 0.6f;
            scale = 2.5f;
            goto add;
        case 3:
            rot[1] = 0.01f;
            d = D_00160500_2BEB0 * 0.7f;
            scale = 2.0f;
            goto add;
        case 4:
            rot[1] = -0.01f;
            d = D_00160500_2BEB0 * 0.75f;
            scale = 1.5f;
        add:
            *p = *p + d;
            break;
        case 5:
            d = D_00160500_2BEB0 * 0.8f;
            scale = 1.25f;
            *p = *p + d;
            break;
        default:
            func_001FA190_2BEB0(D_001D9A70_2BEB0);
            break;
        }
        if (i > 0) {
            f32 v = (f32)i * 2000.0f;
            s32 n = (s32)(*p + v);
            rot[2] = (f32)(n % 10000) * 0.00062831853f - 3.1415927f;
            func_001FA238_2BEB0(D_001D9A70_2BEB0, rot);
            t = D_001D9A70_2BEB0;
            func_001F9C48_2BEB0(t, t, scale);
            func_001F9C48_2BEB0(t + 0x10, t + 0x10, scale);
            func_001F9C48_2BEB0(t + 0x20, t + 0x20, scale);
            qcopy(t + 0x30, D_00160560_2BEB0);
        }
        func_0022C9A8_2BEB0(i);
    }
}
