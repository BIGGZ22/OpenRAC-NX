/* functional: medium. Per-frame state machine of the vendor's purchase prompt (states 1..4 at menu+0x5C): state 1 starts the buy animation on the helper moby at menu+0x20+0x400 and decides whether the item is affordable; state 2 lets d-pad up/down (auto-repeating) change the ammo count and square/circle choose buy or cancel; state 3 waits for the animation; state 4 performs the purchase (money, ammo counts, unlock, messages, sounds) or plays the refusal sound, then resets the state to 0. Written from the asm; the NTSC decomp draft differs in the pad-edge test, which the PAL code does not have. Statically checked only. */
extern void f_setup_29CDE0(void *, int) __asm__("func_0020D440");
extern void f_add_29CDE0(void *, void *, void *) __asm__("func_001F9BD8");
extern void f_init_29CDE0(void *, int, int) __asm__("func_00213D28");
extern void f_stop_29CDE0(void *) __asm__("func_0020D6D0");
extern void f_pose_29CDE0(void *) __asm__("func_L00_00251E30");
extern void f_color_29CDE0(void *, int, int, int, int) __asm__("func_0020E340");
extern int f_snd_29CDE0(int, int, int) __asm__("func_0022ED80");
extern void f_anim_29CDE0(void *, int, int, int) __asm__("func_00213DE0");
extern void f_adv_29CDE0(void *) __asm__("func_L00_002514B8");
extern int f_refund_29CDE0(int, int) __asm__("func_L00_002346C0");
extern void f_refresh_29CDE0(int) __asm__("func_L00_0029C840");
extern void f_cheer_29CDE0(int) __asm__("func_L00_0029B770");
extern int f_text_29CDE0(int) __asm__("func_001FE540");
extern void f_msg_29CDE0(int) __asm__("func_002391A8");
extern void f_unlock_29CDE0(int, int) __asm__("func_L00_002618D8");
extern void f_dropmeter_29CDE0(int, int) __asm__("func_001FFDA0");
extern int f_meter_29CDE0(int, int, void *, void *, void *, void *, int) __asm__("func_001FFB38");
extern void f_cb1_29CDE0(void) __asm__("func_L00_0023B0F8");
extern void f_cb2_29CDE0(void) __asm__("func_L00_00236830");
extern void f_cb3_29CDE0(void) __asm__("func_L00_0023B140");
extern void f_next_29CDE0(void) __asm__("func_L00_0029C9E8");
extern char V_29CDE0[] __asm__("D_L00_001CA7C0");
extern char T_29CDE0[] __asm__("D_L00_001C43B0");
extern char D_0013A5E0_29CDE0[] __asm__("D_0013A5E0");
extern unsigned char D_0013D50F_29CDE0[] __asm__("D_0013D50F");
extern unsigned char D_0013D5EB_29CDE0 __asm__("D_0013D5EB");
extern unsigned char D_0013DE6E_29CDE0[] __asm__("D_0013DE6E");
extern unsigned char D_0013E633_29CDE0[] __asm__("D_0013E633");
extern int D_0015EE98_29CDE0 __asm__("D_0015EE98") MACRO_ADDR;
extern int D_L00_00161F0C_29CDE0 __asm__("D_L00_00161F0C") MACRO_ADDR;
extern int D_L00_00161F10_29CDE0 __asm__("D_L00_00161F10") MACRO_ADDR;
extern int D_L00_00161F28_29CDE0 __asm__("D_L00_00161F28") MACRO_ADDR;
extern int D_L00_00161F2C_29CDE0 __asm__("D_L00_00161F2C") MACRO_ADDR;
extern int D_L00_00161F30_29CDE0 __asm__("D_L00_00161F30") MACRO_ADDR;
typedef u32 u128_29CDE0 __attribute__((mode(TI), aligned(16)));
void func_L00_0029CDE0_impl(void) __asm__("func_L00_0029CDE0");

#define V_F1C (*(char **)(V_29CDE0 + 0x1C))
#define V_F20 (*(char **)(V_29CDE0 + 0x20))
#define V_F40 (*(int *)(V_29CDE0 + 0x40))
#define V_F48 (*(int *)(V_29CDE0 + 0x48))
#define V_F50 (*(int *)(V_29CDE0 + 0x50))
#define V_F58 (*(int *)(V_29CDE0 + 0x58))
#define V_F5C (*(int *)(V_29CDE0 + 0x5C))
#define V_F210 (*(int *)(V_29CDE0 + 0x210))
#define ITEM (V_29CDE0 + 0xD0 + V_F58 * 0x14)
#define ITEM_ID (*(int *)ITEM)
#define ITEM_TYPE (*(int *)(ITEM + 4))
#define TB (T_29CDE0 + ITEM_ID * 0x18)
#define TB_F0 (*(int *)(TB + 0))
#define TB_F4 (*(int *)(TB + 4))
#define TB_H8 (*(unsigned short *)(TB + 8))
#define TB_HA (*(unsigned short *)(TB + 0xA))
#define TB_HE (*(unsigned short *)(TB + 0xE))
#define OWNED(id) (*(int *)(D_0013D50F_29CDE0 + 0x21 + (id) * 4))
#define PAD_HELD (*(int *)(D_0013A5E0_29CDE0 + 0x2460 + 0x1C0))
#define PAD_PRESSED (*(int *)(D_0013A5E0_29CDE0 + 0x2460 + 0x1C4))
#define PRICE (V_F40 ? TB_HA : TB_H8)

void func_L00_0029CDE0_impl(void) {
    int st = V_F5C;
    char *p;
    u128_29CDE0 lv;
    int need, k, n, i;

    if (st == 0)
        return;
    p = V_F20 + 0x400;
    if (st == 1) {
        D_L00_00161F30_29CDE0 = 1;
        D_L00_00161F0C_29CDE0 = 0;
        f_setup_29CDE0(V_F20 + 0x400, 0x471);
        *(short *)(V_F20 + 0x432) = 0x40;
        lv = 0;
        f_add_29CDE0(V_F20 + 0x410, &lv, V_F1C + 0x10);
        *(u128_29CDE0 *)(V_F20 + 0x440) = *(u128_29CDE0 *)(V_F1C + 0x40);
        f_init_29CDE0(V_F20 + 0x400, 0, 0);
        if (*(unsigned char *)(*(char **)(*(char **)(V_F20 + 0x424) + 0x48) + 0x10) < 2) {
            *(int *)(V_F20 + 0x458) = 0;
            *(char *)(V_F20 + 0x451) = 0;
            f_stop_29CDE0(V_F20 + 0x400);
        }
        f_pose_29CDE0(V_F20 + 0x400);
        f_color_29CDE0(V_F20 + 0x400, 0x101010, 0xE, 0xE, 0);
        D_L00_00161F10_29CDE0 = 0;
        if (ITEM_TYPE == 1 && !(OWNED(ITEM_ID) < TB_HE)) {
            f_snd_29CDE0(2, 0, (int)V_F1C);
        } else if (ITEM_TYPE == 1) {
            int nn = V_F40 ? TB_HA : TB_H8;
            if (D_0015EE98_29CDE0 < nn)
                f_snd_29CDE0(2, 0, (int)V_F1C);
            else
                D_L00_00161F10_29CDE0 = 1;
        } else {
            int nn = D_0013D5EB_29CDE0 ? TB_F4 : TB_F0;
            if (D_0015EE98_29CDE0 < nn)
                f_snd_29CDE0(2, 0, (int)V_F1C);
        }
        V_F5C = 2;
        return;
    }
    if (st == 2) {
        if (D_L00_00161F10_29CDE0 == 1) {
            if (PAD_HELD & 0x8000)
                D_L00_00161F28_29CDE0 = D_L00_00161F28_29CDE0 + 1;
            else
                D_L00_00161F28_29CDE0 = 0;
            if (PAD_HELD & 0x2000)
                D_L00_00161F2C_29CDE0 = D_L00_00161F2C_29CDE0 + 1;
            else
                D_L00_00161F2C_29CDE0 = 0;
            if ((PAD_PRESSED & 0x8000) ||
                (D_L00_00161F28_29CDE0 >= 0x10 && (D_L00_00161F28_29CDE0 & 7) == 0) ||
                D_L00_00161F28_29CDE0 >= 0x30) {
                if (D_L00_00161F30_29CDE0 >= 2) {
                    D_L00_00161F30_29CDE0 = D_L00_00161F30_29CDE0 - 1;
                    if ((D_L00_00161F28_29CDE0 & 3) == 0 || D_L00_00161F28_29CDE0 < 0x10)
                        f_snd_29CDE0(1, 0, (int)V_F1C);
                }
            }
            if ((PAD_PRESSED & 0x2000) ||
                (D_L00_00161F2C_29CDE0 >= 0x10 && (D_L00_00161F2C_29CDE0 & 7) == 0) ||
                D_L00_00161F2C_29CDE0 >= 0x30) {
                if (PRICE > 0) {
                    int cnt = D_0015EE98_29CDE0 / PRICE;
                    int left = TB_HE - OWNED(ITEM_ID);
                    if (left < cnt)
                        cnt = left;
                    if (D_L00_00161F30_29CDE0 < cnt) {
                        D_L00_00161F30_29CDE0 = D_L00_00161F30_29CDE0 + 1;
                        if ((D_L00_00161F2C_29CDE0 & 3) == 0 || D_L00_00161F2C_29CDE0 < 0x10)
                            f_snd_29CDE0(1, 0, (int)V_F1C);
                    }
                }
            }
        }
        if (PAD_PRESSED & 0x10) {
            V_F5C = 3;
            D_L00_00161F0C_29CDE0 = 0;
            f_anim_29CDE0(V_F20 + 0x400, 0,
                          *(unsigned char *)(*(char **)(*(char **)(V_F20 + 0x424) + 0x48) + 0x10) - 1, 1);
            f_pose_29CDE0(V_F20 + 0x400);
            return;
        }
        if (PAD_PRESSED & 0x40) {
            V_F5C = 3;
            D_L00_00161F0C_29CDE0 = 1;
            f_anim_29CDE0(V_F20 + 0x400, 0,
                          *(unsigned char *)(*(char **)(*(char **)(V_F20 + 0x424) + 0x48) + 0x10) - 1, 8);
            f_pose_29CDE0(V_F20 + 0x400);
            return;
        }
        f_adv_29CDE0(V_F20 + 0x400);
        if (!*(unsigned char *)(V_F20 + 0x453) && (*(unsigned char *)(V_F20 + 0x470) & 2))
            f_anim_29CDE0(V_F20 + 0x400, 1, 0, 1);
        f_pose_29CDE0(V_F20 + 0x400);
        return;
    }
    if (st == 3) {
        f_adv_29CDE0(V_F20 + 0x400);
        if (!*(unsigned char *)(V_F20 + 0x453) && (*(unsigned char *)(V_F20 + 0x470) & 2)) {
            *(float *)(V_F20 + 0x458) = -1.0f;
            V_F5C = 4;
        }
        f_pose_29CDE0(V_F20 + 0x400);
        return;
    }
    if (st != 4)
        return;
    f_adv_29CDE0(V_F20 + 0x400);
    if (*(unsigned char *)(V_F20 + 0x470) & 2) {
        if (D_L00_00161F0C_29CDE0 == 1) {
            if (ITEM_TYPE == D_L00_00161F0C_29CDE0)
                need = PRICE * D_L00_00161F30_29CDE0;
            else if (D_0013D5EB_29CDE0)
                need = TB_F4;
            else
                need = TB_F0;
            if (D_0015EE98_29CDE0 < need && !(OWNED(ITEM_ID) < TB_HE)) {
                f_snd_29CDE0(0, 0, (int)V_F1C);
            } else if (ITEM_TYPE == 1) {
                if (OWNED(ITEM_ID) < TB_HE) {
                    n = PRICE * D_L00_00161F30_29CDE0;
                    if (D_0015EE98_29CDE0 >= n) {
                        i = ITEM_ID;
                        if (OWNED(i) > *(unsigned short *)(T_29CDE0 + i * 0x18 + 0xE))
                            OWNED(i) = *(unsigned short *)(T_29CDE0 + i * 0x18 + 0xE);
                        k = D_L00_00161F30_29CDE0;
                        k -= f_refund_29CDE0(ITEM_ID, k);
                        n = PRICE * k;
                        *(int *)(D_0013DE6E_29CDE0 + 0xA + ITEM_ID * 4) += k;
                        f_refresh_29CDE0(3);
                        D_0015EE98_29CDE0 = D_0015EE98_29CDE0 - n;
                        f_cheer_29CDE0(V_F40 != 0);
                        f_msg_29CDE0(f_text_29CDE0(0x4F5F));
                        f_snd_29CDE0(7, 0, (int)V_F1C);
                    } else {
                        f_msg_29CDE0(f_text_29CDE0(0x4F60));
                        f_snd_29CDE0(0, 0, (int)V_F1C);
                    }
                } else {
                    f_snd_29CDE0(0, 0, (int)V_F1C);
                }
            } else {
                n = D_0013D5EB_29CDE0 ? TB_F4 : TB_F0;
                if (D_0015EE98_29CDE0 >= n) {
                    f_refresh_29CDE0(2);
                    f_unlock_29CDE0(ITEM_ID, 1);
                    V_F48 = ITEM_ID;
                    if (ITEM_ID == 0x18)
                        D_0013E633_29CDE0[0x2E12] = 1;
                    D_0015EE98_29CDE0 = D_0015EE98_29CDE0 - n;
                    f_cheer_29CDE0(V_F40 != 0);
                    if (V_F50 != -1)
                        f_dropmeter_29CDE0(V_F50, 0);
                    V_F50 = -1;
                    if (ITEM_TYPE == 1) {
                        i = ITEM_ID;
                        V_F50 = f_meter_29CDE0(0x30, 0xEA60 + i, (void *)f_cb1_29CDE0, (void *)f_cb2_29CDE0,
                                               (void *)f_cb3_29CDE0, &OWNED(i),
                                               *(unsigned short *)(T_29CDE0 + i * 0x18 + 0xE));
                    } else {
                        int m = V_F210 - 1;
                        if (m < V_F58)
                            V_F58 = m;
                        f_next_29CDE0();
                    }
                    f_msg_29CDE0(f_text_29CDE0(0x4F5F));
                    f_snd_29CDE0(7, 0, (int)V_F1C);
                } else {
                    f_msg_29CDE0(f_text_29CDE0(0x4F60));
                    f_snd_29CDE0(0, 0, (int)V_F1C);
                }
            }
        } else {
            f_snd_29CDE0(0, 0, (int)V_F1C);
        }
        V_F5C = 0;
    }
    f_pose_29CDE0(V_F20 + 0x400);
}
