/* functional: medium. Picks the next animation (returned) for an actor from its mode/state bytes (B+0x28/0x29/0x2A) and the buttons, via a state table D_001CC400; sets *outA (animation status), *outI (state id), *outF (B+0x2C speed), advances/reloads the timer at an+0x1C. Static check only; the 0x47 block (chase-toward-D_0013F450 kick) is the least certain part. */
extern f32 D_001600C8_4770[] __asm__("D_001600C8") MACRO_ADDR;
extern u8 D_001CC400_4770[] __asm__("D_001CC400");
extern u8 D_001CC448_4770[] __asm__("D_001CC448");
extern u8 D_0013E633_4770[] __asm__("D_0013E633");
extern u8 D_0013F450_4770[] __asm__("D_0013F450");
extern s32 func_00215048_4770(void *) __asm__("func_00215048");
extern u8 *func_00215078_4770(void *) __asm__("func_00215078");
extern s32 func_001F9938_4770(void *) __asm__("func_001F9938");
extern s32 func_001F98C0_4770(s32) __asm__("func_001F98C0");
extern void func_001F9C30_4770(void *, void *, f32) __asm__("func_001F9C30");
extern void func_001F9DC0_4770(void *, void *, f32) __asm__("func_001F9DC0");
extern void func_001F9BD8_4770(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001F9E10_4770(void *, void *, f32) __asm__("func_001F9E10");
extern f32 func_001F9CB8_4770(void *) __asm__("func_001F9CB8");
extern f32 func_001FA058_4770(f32, f32) __asm__("func_001FA058");
extern f32 func_001F9F90_4770(f32) __asm__("func_001F9F90");
extern f32 func_001F9FA8_4770(f32) __asm__("func_001F9FA8");

s32 func_00214778(u8 *a, u8 *B, u8 *an, u32 btn, s32 *outA, f32 *outF, s32 *outI, s32 k) {
    f32 v[4] __attribute__((aligned(16)));
    f32 w[4] __attribute__((aligned(16)));
    s32 s21;
    s32 a22;
    s32 t;
    s32 u17;
    s32 c;
    s32 a4;
    s32 sw;

    *outA = 0;
    if (outI != 0)
        *outI = -1;

    if (B != 0 && *(u8 **)(B + 0x20) != 0) {
        s32 cls = *(s16 *)(*(u8 **)(B + 0x20) + 0xA6);
        if ((cls == 0x131 || cls == 0xA8) && an != 0)
            *(f32 *)(B + 0x2C) = D_001600C8_4770[an[8]];
    }

    if (a == 0) {
        if (an != 0) {
            *(s16 *)(an + 0x1C) = 0;
            *(s16 *)(an + 0x14) = -1;
            *(s16 *)(an + 0x16) = -1;
        }
        *outA = 0;
        return 0;
    }
    if (an == 0) {
        an = (u8 *)func_00215048_4770(a);
        if (an == 0) {
            *outA = 0;
            return 0;
        }
    }
    if (k == 4)
        k = an[8];

    if (B == 0) {
        func_001F9938_4770(an + 0x1C);
        *(s16 *)(an + 0x16) = -1;
        *outA = 1;
        if (outF != 0)
            *outF = 0;
        return 0xB;
    }

    if (*(f32 *)(B + 0x1C) == 5627.9248f) {
        func_001F9C30_4770(B + 0x10, B + 0x10, *(f32 *)(D_001CC448_4770 + B[0x28] * 4 + k * 32));
        a22 = *(u16 *)(B + 0x2A);
        *(f32 *)(B + 0x1C) = 5627.9248f;
        if (a22 == 0x47) {
            f32 dx = *(f32 *)(a + 0x10) - *(f32 *)(D_0013F450_4770 + 0x80);
            f32 dy = *(f32 *)(a + 0x14) - *(f32 *)(D_0013F450_4770 + 0x84);
            f32 ang = func_001FA058_4770(dx, dy);
            v[0] = func_001F9F90_4770(ang);
            dx = *(f32 *)(a + 0x10) - *(f32 *)(D_0013F450_4770 + 0x80);
            dy = *(f32 *)(a + 0x14) - *(f32 *)(D_0013F450_4770 + 0x84);
            ang = func_001FA058_4770(dx, dy);
            v[2] = 1.0f;
            v[1] = func_001F9FA8_4770(ang);
            func_001F9DC0_4770(v, v, func_001F9CB8_4770(B + 0x10) * 0.7f);
            func_001F9C30_4770(B + 0x10, B + 0x10, 0.3f);
            func_001F9BD8_4770(w, B + 0x10, v);
            qcopy(B + 0x10, w);
            func_001F9E10_4770(B + 0x10, B + 0x10, 1.0f);
        }
    } else {
        if (*(u8 **)(B + 0x20) == 0)
            a22 = 0x47;
        else
            a22 = *(s16 *)(*(u8 **)(B + 0x20) + 0xA6);
        B[0x28] = 8;
    }

    t = 2;
    s21 = 1;
    sw = 0;
    switch (B[0x28]) {
    case 0:
        if ((u32)(*(u16 *)(B + 0x2A) - 1) < 2) {
            s21 = 0;
            sw = 1;
        }
        break;
    case 1:
        c = B[0x29];
        if (c < 2) {
            s21 = 0xB;
            sw = 1;
        } else if (c == 2) {
            *outA = 2;
            s21 = 5;
            sw = 2;
        } else {
            *outA = 2;
            s21 = 9;
            sw = 2;
        }
        break;
    case 2:
        c = B[0x29];
        s21 = (c < 2) ? 8 : 0xD;
        sw = 1;
        break;
    case 3:
        c = B[0x29];
        if (c < 2) {
            s21 = 0xA;
            sw = 1;
        } else if (c < 3) {
            s21 = 0xF;
            sw = 1;
        } else {
            *outA = 2;
            s21 = 0x10;
            *(s16 *)(an + 0x14) = -1;
            *(s16 *)(an + 0x1C) = 0;
            sw = 2;
        }
        break;
    case 4:
        s21 = 0xE;
        sw = 1;
        break;
    case 5:
        c = B[0x29];
        if (c < 2) {
            t = 3;
            s21 = 4;
        } else {
            t = 4;
            s21 = 7;
        }
        sw = 1;
        break;
    case 7:
        c = B[0x29];
        s21 = (c < 2) ? 0xC : 0x11;
        sw = 1;
        break;
    default:
        break;
    }
    if (sw != 2)
        *outA = t;

    if (outI != 0)
        *outI = a22;

    u17 = D_001CC400_4770[an[8] + s21 * 4];
    if (*(s16 *)(an + 0x14) != a22)
        *(s16 *)(an + 0x1C) = 0;

    if (func_001F9938_4770(an + 0x1C) != 0) {
        switch (s21) {
        case 0: a4 = 0x25; break;
        case 1: a4 = 0xF; break;
        case 4: a4 = 0x3C; break;
        case 7: a4 = (D_0013E633_4770[0] != 0) ? 0x2D : 0x3C; break;
        case 11: a4 = 0x3C; break;
        default: a4 = 0x1E; break;
        }
        *(s16 *)(an + 0x1C) = (s16)func_001F98C0_4770(a4);
    } else {
        u32 pad = 0;
        u32 mask = 0;
        s32 have = 1;
        switch (s21) {
        case 0:
        case 1:
            if ((btn & 0x10) == 0)
                u17 = 0xB;
            pad = btn & 0x20;
            break;
        case 4:
            if ((btn & 1) == 0)
                u17 = 0xB;
            pad = btn & 2;
            break;
        case 7:
            if ((btn & 4) == 0)
                u17 = 0xB;
            pad = btn & 8;
            break;
        case 11:
            u17 = 0xC;
            have = 0;
            break;
        default:
            u17 = 0xD;
            have = 0;
            break;
        }
        (void)mask;
        if (have) {
            if (pad == 0)
                *outA = 1;
        }
        if (s21 != 11)
            *(f32 *)(B + 0x2C) = 0.0f;
    }

    *(s16 *)(an + 0x16) = (s16)a22;
    *(s16 *)(an + 0x14) = (s16)a22;
    if (outF != 0)
        *outF = *(f32 *)(B + 0x2C);

    {
        u8 *p = func_00215078_4770(a);
        if (s21 == 4) {
            if (p != 0)
                p[0x2E] = 1;
        } else {
            if (p != 0)
                p[0x2E] = 0;
        }
    }
    return u17;
}
