/* Carried from the PAL decompilation's nonmatching/functional/func_L01_002EF078.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* functional: medium. Same reads, writes, calls and arguments in the same order as retail (no COP2/VU0 code in it); differs only in compiler structure (registers, folded offsets, merged tails, a switch for the jump table), and the 0x161A50/0x161A40/0x1619AC gp-relative globals are written as constants. */
/* Level 1 / 11 (Novalis) moby update: a character-like moby with a state record at moby[0x78]
 * (hero pointer at rec[0x200], mode word rec[0x204], flags rec[0x22C]/0x230/0x258/0x25C), a jump table over
 * the moby's state byte m[0x20] (0..16, 10..12 do nothing). Before the dispatch: state 12 only sets bit 0 of
 * m[0x34] and returns; every other state runs the record's per-frame call func_L00_00263AC8(2.9, m, 0, rec+0x270),
 * and when m[0x31] is set and the moby is within 28 of D_L01_00167240 it calls func_L00_0025A120 and sets m[0x7F]
 * to 0x16; then func_L01_002EEC68(m).
 *   0  spawn: copies the position to rec+0x210, picks speed/sound bytes by the moby class m[0xA6]
 *      (0x23C, 0x361, 0x362), randomises rec[0x254], sets the record's tables (rec[0x130]), the walk speed
 *      and goes to state 12/13 (or back to 1 when rec[0x230]);
 *   1  idle wait: the path search (func_L01_00276FE8) toward the hero; on success state 4 with a random
 *      yaw offset, on a hero request (rec[0x22C]) state 0xF, else state 2;
 *   2  turns toward the hero, steers by rec+0x170 and searches the path (state 3 on success, 0xF on request),
 *      then func_L01_002EF560;
 *   3, 7  wait for the animation end flag (m[0x70] bit 1) and go to 4; 4 chase with a probe/second probe,
 *      state 6 when near the limit, 5 when the mode word is 2; 5 walk back to rec+0x210; 6 attack/turn with a
 *      hit effect (func_L00_00259BC8) and state 7; 8 reacts to the contact bits of func_L00_0025C698 (state 9
 *      or the death calls); 9 end of an animation, back to 4; 13 respawn (needs func_00214720 on the hero
 *      block at 0x13F420); 14 returns to 1 when func_L00_002DC9F0 says so; 15 chase with hit tests, states
 *      5/6/0x10; 16 waits for the animation end and goes to 5.
 * The record's float constants come straight from the assembly (hex bits in the comments where the decimal
 * is not exact). */
extern float D_0015EE6C_2EF078 __asm__("D_0015ED6C") MACRO_ADDR;
extern float D_0015EE70_2EF078 __asm__("D_0015ED70") MACRO_ADDR;
extern char D_L01_001672C0_2EF078[] __asm__("D_L01_00167240");
extern char D_L01_0015F660_2EF078[] __asm__("D_L01_0015F580");
typedef int u128_2EF078_c2edca0 __attribute__((mode(TI)));

extern void func_L00_00264B40_2EF078(float, void *, int, void *) __asm__("func_L00_00263AC8");
extern float func_001F9D10_2EF078(void *, void *) __asm__("func_001F9B48");
extern float func_001F9D48_2EF078(void *, void *) __asm__("func_001F9B80");
extern void func_L00_0025B178_2EF078(void *) __asm__("func_L00_0025A120");
extern void func_L01_002F0040_2EF078(void *) __asm__("func_L01_002EEC68");
extern int func_001FA898_2EF078(float) __asm__("func_001FA6D0");
extern float func_00214158_2EF078(void) __asm__("func_00213308");
extern void func_L01_0026E8E0_2EF078(void *) __asm__("func_L01_0026D930");
extern int func_002140B0_2EF078(int) __asm__("func_00213260");
extern int func_L01_00277FD8_2EF078(void *, int, int, void *, void *, void *, float) __asm__("func_L01_00276FE8");
extern void func_00213DE0_2EF078(void *, int, int, int) __asm__("func_00212F90");
extern int func_001F9850_2EF078(int) __asm__("func_001F9688");
extern void func_001F9BC0_2EF078(void *) __asm__("func_001F99F8");
extern float func_002140F8_2EF078(float, float) __asm__("func_002132A8");
extern float func_L00_001FF860_2EF078(float, float) __asm__("func_L00_001FF8B0");
extern float func_L00_0025CE58_2EF078(void *, void *, float, float, float, float) __asm__("func_L00_0025BE00");
extern float func_001F9F90_2EF078(float) __asm__("func_001F9DC8");
extern float func_001F9FA8_2EF078(float) __asm__("func_001F9DE0");
extern int func_L00_00259B88_2EF078(void *, void *, void *, void *, float) __asm__("func_L00_00258B50");
extern int func_L00_00262BC0_2EF078(int, void *, void *, void *) __asm__("func_L00_00261B48");
extern float func_001FA748_2EF078(float, float) __asm__("func_001FA580");
extern float func_0020D830_2EF078(void *) __asm__("func_0020C9E0");
extern int func_00215B18_2EF078(void *, float) __asm__("func_00214CC8");
extern float func_001F9B88_2EF078(float) __asm__("func_001F99C0");
extern void func_L00_0025AC00_2EF078(void *, void *, int, void *, void *, float) __asm__("func_L00_00259BC8");
extern void func_L01_002F0938_2EF078(void *) __asm__("func_L01_002EF560");
extern int func_L00_0025D6F0_2EF078(void *, void *) __asm__("func_L00_0025C698");
extern int func_0022ED80_2EF078(int, int, void *) __asm__("func_0022DA68");
extern void func_L01_002F0B48_2EF078(void *, void *) __asm__("func_L01_002EF770");
extern void func_L00_002584A8_2EF078(void *, int, int) __asm__("func_L00_00257470");
extern void func_0020D678_2EF078(void *) __asm__("func_0020C828");
extern int func_00215570_2EF078(void *, int) __asm__("func_00214720");
extern int func_L00_002DDEA0_2EF078(void *, void *) __asm__("func_L00_002DC9F0");

#define F(p, o) (*(float *)((p) + (o)))
#define I(p, o) (*(int *)((p) + (o)))
#define U8(p, o) (*(unsigned char *)((p) + (o)))
#define U16(p, o) (*(unsigned short *)((p) + (o)))
#define PI_F 3.1415927410125732f

void FUN_L01_002edca0(char *m) {
    char *rec = *(char **)(m + 0x78);
    char *hero;
    float S[8] __attribute__((aligned(16)));
    float f, a, ang;
    int r, v, flag, state;

    if (U8(m, 0x20) == 12) {
        U16(m, 0x34) |= 1;
        return;
    }
    func_L00_00264B40_2EF078(2.9000000953674316f, m, 0, rec + 0x270);
    if (U8(m, 0x31) != 0) {
        if (func_001F9D10_2EF078(m + 0x10, D_L01_001672C0_2EF078) < 28.0f) {
            func_L00_0025B178_2EF078(m);
            U8(m, 0x7F) = 0x16;
        }
    }
    func_L01_002F0040_2EF078(m);
    state = U8(m, 0x20);
    hero = *(char **)(rec + 0x200);
    if (state > 16)
        return;

    switch (state) {
    case 0:
        *(u128_2EF078_c2edca0 *)(rec + 0x210) = *(u128_2EF078_c2edca0 *)(m + 0x10);
        switch (*(short *)(m + 0xA6)) {
        case 0x23C:
            U8(rec, 0x58) = func_001FA898_2EF078(14.0f);
            U8(rec, 0x5A) = func_001FA898_2EF078(10.399999618530273f);
            F(rec, 0x250) = 1.0f;
            f = F(rec, 0x250);
            break;
        case 0x361:
            U8(rec, 0x58) = func_001FA898_2EF078(14.0f);
            U8(rec, 0x5A) = func_001FA898_2EF078(5.599999904632568f);
            F(rec, 0x250) = 0.699999988079071f;
            f = F(rec, 0x250);
            break;
        case 0x362:
            U8(rec, 0x58) = func_001FA898_2EF078(14.0f);
            U8(rec, 0x5A) = func_001FA898_2EF078(4.0f);
            F(rec, 0x250) = 0.49000000953674316f;
            f = F(rec, 0x250);
            break;
        default:
            f = F(rec, 0x250);
            break;
        }
        F(rec, 0x30) = f;
        F(rec, 0x254) = func_00214158_2EF078();
        if (*(short *)(m + 0xA6) == 0x362)
            I(rec, 0x130) = 0x161A50;
        else
            I(rec, 0x130) = 0x161A40;
        U8(rec, 0x29) = 1;
        if (I(rec, 0x258) != 0) {
            f = 1.0f;
            I(rec, 0x230) = 1;
        } else {
            f = 3.0f;
        }
        F(rec, 0x20) = f;
        if (F(rec, 0x224) == 0.0f)
            F(rec, 0x224) = 15.0f;
        func_L01_0026E8E0_2EF078(rec + 0x170);
        F(rec, 0x178) = 2.0f;
        F(rec, 0x17C) = 2.0f;
        a = F(rec, 0x250) * 0.800000011920929f * 1024.0f;
        I(rec, 0x170) = func_001FA898_2EF078(a);
        F(rec, 0x194) = F(rec, 0x23C) * D_0015EE6C_2EF078;
        if (I(rec, 0x230) != 0) {
            U8(m, 0x20) = 1;
            U16(m, 0x34) |= 0x1000;
            return;
        }
        U8(m, 0x20) = (I(rec, 0x25C) == -1) ? 12 : 13;
        F(m, 0x18) = F(m, 0x18) - 20.0f;
        I(m, 0x94) = 0;
        I(m, 0x58) = 0;
        U8(m, 0x31) = 0;
        U16(m, 0x34) = (U16(m, 0x34) & 0xEFFF) | 1;
        return;

    case 1:
        if (I(rec, 0x204) != 2) {
            if (func_002140B0_2EF078(4) == 0) {
                if (func_L01_00277FD8_2EF078(rec + 0x240, 1, I(rec, 0x244), m + 0x10, hero + 0x10, S,
                                             F(rec, 0x250) * 0.800000011920929f)) {
                    U8(m, 0x20) = 4;
                    if (U8(m, 0x53) != 1)
                        func_00213DE0_2EF078(m, 1, 0, func_001F9850_2EF078(10));
                    func_001F9BC0_2EF078(rec + 0x40);
                    F(rec, 0x234) = func_002140F8_2EF078(-0.3490658402442932f, 0.3490658402442932f);
                } else {
                    I(rec, 0x204) = 2;
                }
            }
        }
        if (I(rec, 0x22C) != 0 && I(rec, 0x204) == 2) {
            U8(m, 0x20) = 0xF;
            if (U8(m, 0x53) == 1)
                return;
            func_00213DE0_2EF078(m, 1, 0, 5);
            return;
        }
        F(rec, 0x194) = F(rec, 0x23C) * 0.5f * D_0015EE6C_2EF078;
        if (U8(m, 0x53) != 1)
            func_00213DE0_2EF078(m, 1, 0, 5);
        U8(m, 0x20) = 2;
        return;

    case 2:
        ang = func_L00_001FF860_2EF078(F(rec, 0x210) - F(m, 0x10), F(rec, 0x214) - F(m, 0x14));
        func_L00_0025CE58_2EF078(m + 0x48, rec + 0x220, ang, D_0015EE70_2EF078 * 1.0471975803375244f,
                                 D_0015EE70_2EF078 * 1.0471975803375244f, D_0015EE6C_2EF078 * PI_F);
        S[0] = func_001F9F90_2EF078(F(m, 0x48));
        S[1] = func_001F9FA8_2EF078(F(m, 0x48));
        S[2] = 0;
        func_L00_00259B88_2EF078(m, rec + 0x170, S, rec + 0x40, 1.0f);
        if (I(rec, 0x204) != 2) {
            if (func_002140B0_2EF078(4) == 0) {
                if (func_L01_00277FD8_2EF078(rec + 0x240, 1, I(rec, 0x244), m + 0x10, hero + 0x10, S + 4,
                                             F(rec, 0x250) * 0.800000011920929f)) {
                    U8(m, 0x20) = 3;
                    if (U8(m, 0x53) != 0)
                        func_00213DE0_2EF078(m, 0, 0, func_001F9850_2EF078(10));
                    func_001F9BC0_2EF078(rec + 0x40);
                    F(rec, 0x234) = func_002140F8_2EF078(-0.3490658402442932f, 0.3490658402442932f);
                    F(rec, 0x194) = F(rec, 0x23C) * D_0015EE6C_2EF078;
                } else {
                    I(rec, 0x204) = 2;
                }
            }
        }
        if (I(rec, 0x22C) != 0 && I(rec, 0x204) == 2) {
            U8(m, 0x20) = 0xF;
            if (U8(m, 0x53) != 1)
                func_00213DE0_2EF078(m, 1, 0, 5);
        }
        func_L01_002F0938_2EF078(m);
        return;

    case 15:
        flag = 0;
        ang = func_L00_001FF860_2EF078(F(hero, 0x10) - F(m, 0x10), F(hero, 0x14) - F(m, 0x14));
        func_L00_0025CE58_2EF078(m + 0x48, rec + 0x220, ang, D_0015EE70_2EF078 * 1.0471975803375244f,
                                 D_0015EE70_2EF078 * 1.0471975803375244f, D_0015EE6C_2EF078 * PI_F);
        S[0] = func_001F9F90_2EF078(F(m, 0x48));
        S[1] = func_001F9FA8_2EF078(F(m, 0x48));
        S[2] = 0;
        func_L00_00259B88_2EF078(m, rec + 0x170, S, rec + 0x40, 1.0f);
        if (func_L00_00262BC0_2EF078(I(rec, 0x240), m + 0x10, hero + 0x10, S + 4)) {
            if (func_001F9D10_2EF078(m + 0x10, S + 4) < 1.5f)
                flag = 1;
        }
        if (I(rec, 0x22C) == 0) {
            U8(m, 0x20) = 5;
            return;
        }
        if (flag) {
            U8(m, 0x20) = 0x10;
            if (U8(m, 0x53) == 0)
                return;
            v = func_001F9850_2EF078(10);
            func_00213DE0_2EF078(m, 0, 0, v);
            return;
        }
        if (func_001F9D48_2EF078(m + 0x10, hero + 0x10) < 2.0f) {
            U8(m, 0x20) = 6;
            if (U8(m, 0x53) == 2)
                return;
            v = func_001F9850_2EF078(10);
            func_00213DE0_2EF078(m, 2, 5, v);
            return;
        }
        return;

    case 16:
        if (!(U8(m, 0x70) & 2))
            return;
        if (I(rec, 0x22C) != 0)
            return;
        U8(m, 0x20) = 5;
        if (U8(m, 0x53) == 1)
            return;
        v = func_001F9850_2EF078(0x14);
        func_00213DE0_2EF078(m, 1, 0, v);
        return;

    case 4:
        if (func_001F9D48_2EF078(m + 0x10, hero + 0x10) < *(float *)0x1619AC) {
            U8(m, 0x20) = 6;
            F(rec, 0x194) = (F(rec, 0x23C) + F(rec, 0x23C)) * D_0015EE6C_2EF078;
            if (U8(m, 0x53) == 2)
                return;
            v = func_001F9850_2EF078(10);
            func_00213DE0_2EF078(m, 2, 0, v);
            return;
        }
        r = func_L01_00277FD8_2EF078(rec + 0x240, 1, I(rec, 0x244), m + 0x10, hero + 0x10, S + 4,
                                     F(rec, 0x250) * 0.800000011920929f);
        if (r == 0)
            r = func_L01_00277FD8_2EF078(rec + 0x240, 1, I(rec, 0x244), m + 0x10, hero + 0x10, S + 4, 0.0f) != 0;
        if (r) {
            ang = func_L00_001FF860_2EF078(S[4] - F(m, 0x10), S[5] - F(m, 0x14));
            ang = func_001FA748_2EF078(ang, F(rec, 0x234));
            func_L00_0025CE58_2EF078(m + 0x48, rec + 0x220, ang, D_0015EE70_2EF078 * 12.566370964050293f,
                                     D_0015EE70_2EF078 * 12.566370964050293f, D_0015EE6C_2EF078 * 25.132741928100586f);
            S[0] = func_001F9F90_2EF078(F(m, 0x48));
            S[0] = S[0] + S[0];
            S[1] = func_001F9FA8_2EF078(F(m, 0x48));
            S[1] = S[1] + S[1];
            S[2] = 0;
            func_L00_00259B88_2EF078(m, rec + 0x170, S, rec + 0x40, 1.0f);
        } else {
            I(rec, 0x204) = 2;
        }
        if (I(rec, 0x204) == 2)
            U8(m, 0x20) = 5;
        func_L01_002F0938_2EF078(m);
        return;

    case 6:
        ang = func_L00_001FF860_2EF078(F(hero, 0x10) - F(m, 0x10), F(hero, 0x14) - F(m, 0x14));
        func_L00_0025CE58_2EF078(m + 0x48, rec + 0x220, ang, D_0015EE70_2EF078 * 12.566370964050293f,
                                 D_0015EE70_2EF078 * 12.566370964050293f, D_0015EE6C_2EF078 * 25.132741928100586f);
        f = func_0020D830_2EF078(m);
        if (U8(m, 0x52) == U8(m, 0x53) && 10.0f <= f && f <= 16.0f) {
            S[0] = func_001F9F90_2EF078(F(m, 0x48));
            S[0] = S[0] + S[0];
            S[1] = func_001F9FA8_2EF078(F(m, 0x48));
            S[1] = S[1] + S[1];
            S[2] = 0;
            func_L00_00259B88_2EF078(m, rec + 0x170, S, rec + 0x40, 1.0f);
        }
        if (func_00215B18_2EF078(m, 15.5f)) {
            if (func_001F9D48_2EF078(m + 0x10, hero + 0x10) < 2.0f) {
                if (func_001F9B88_2EF078(F(hero, 0x18) - F(m, 0x18)) < F(rec, 0x250) + F(rec, 0x250)) {
                    S[0] = func_001F9F90_2EF078(F(m, 0x48));
                    S[1] = func_001F9FA8_2EF078(F(m, 0x48));
                    S[3] = 5627.9248046875f;
                    S[2] = 0;
                    *(u128_2EF078_c2edca0 *)(S + 4) = *(u128_2EF078_c2edca0 *)(hero + 0x10);
                    S[6] = S[6] + 0.75f;
                    func_L00_0025AC00_2EF078(hero, m, 0x10001, S + 4, S, 1.0f);
                    return;
                }
            }
        }
        if (!(U8(m, 0x70) & 2))
            return;
        U8(m, 0x20) = 7;
        if (U8(m, 0x53) != 3) {
            v = func_001F9850_2EF078(10);
            func_00213DE0_2EF078(m, 3, 0, v);
        }
        F(rec, 0x194) = F(rec, 0x23C) * D_0015EE6C_2EF078;
        return;

    case 3:
    case 7:
        if (!(U8(m, 0x70) & 2))
            return;
        U8(m, 0x20) = 4;
        if (U8(m, 0x53) == 1)
            return;
        v = func_001F9850_2EF078(10);
        func_00213DE0_2EF078(m, 1, 0, v);
        return;

    case 5:
        flag = 0;
        if (func_001F9D48_2EF078(m + 0x10, rec + 0x210) < 1.0f) {
            U8(m, 0x20) = 1;
        } else {
            if (func_L01_00277FD8_2EF078(rec + 0x240, 1, I(rec, 0x244), m + 0x10, rec + 0x210, S + 4,
                                         F(rec, 0x250) * 0.800000011920929f)) {
                flag = 1;
            } else {
                /* retail passes rec+0x210 for both position arguments here */
                flag = func_L01_00277FD8_2EF078(rec + 0x240, 1, I(rec, 0x244), rec + 0x210, rec + 0x210, S + 4, 0.0f) != 0;
            }
        }
        if (flag) {
            ang = func_L00_001FF860_2EF078(S[4] - F(m, 0x10), S[5] - F(m, 0x14));
            ang = func_001FA748_2EF078(ang, F(rec, 0x234));
            func_L00_0025CE58_2EF078(m + 0x48, rec + 0x220, ang, D_0015EE70_2EF078 * 12.566370964050293f,
                                     D_0015EE70_2EF078 * 12.566370964050293f, D_0015EE6C_2EF078 * 25.132741928100586f);
            S[0] = func_001F9F90_2EF078(F(m, 0x48));
            S[0] = S[0] + S[0];
            S[1] = func_001F9FA8_2EF078(F(m, 0x48));
            S[1] = S[1] + S[1];
            S[2] = 0;
            func_L00_00259B88_2EF078(m, rec + 0x170, S, rec + 0x40, 1.0f);
        }
        if (I(rec, 0x204) != 2) {
            U8(m, 0x20) = 4;
            if (U8(m, 0x53) != 1) {
                v = func_001F9850_2EF078(3);
                func_00213DE0_2EF078(m, 1, 0, v);
            }
        }
        func_L01_002F0938_2EF078(m);
        return;

    case 8:
        r = func_L00_0025D6F0_2EF078(m, rec + 0x60);
        if (r & 1) {
            if (func_L01_00277FD8_2EF078(rec + 0x240, 1, I(rec, 0x244), m + 0x10, rec + 0x210, S,
                                         F(rec, 0x250) * 0.800000011920929f)) {
                func_0022ED80_2EF078(4, 0, m);
                F(rec, 0x194) = F(rec, 0x23C) * D_0015EE6C_2EF078;
                U8(m, 0x20) = 9;
                if (U8(m, 0x53) != 5) {
                    v = func_001F9850_2EF078(10);
                    func_00213DE0_2EF078(m, 5, 4, v);
                }
                *(u128_2EF078_c2edca0 *)(rec + 0x210) = *(u128_2EF078_c2edca0 *)(m + 0x10);
                return;
            }
            func_L01_002F0B48_2EF078(m, D_L01_0015F660_2EF078);
            func_L00_002584A8_2EF078(m, 0, -1);
        } else if (!(r & 0x120)) {
            return;
        }
        func_0020D678_2EF078(m);
        return;

    case 9:
        if (!(U8(m, 0x70) & 2))
            return;
        U8(m, 0x20) = 4;
        if (U8(m, 0x53) != 1) {
            v = func_001F9850_2EF078(10);
            func_00213DE0_2EF078(m, 1, 0, v);
        }
        func_001F9BC0_2EF078(rec + 0x40);
        F(rec, 0x234) = func_002140F8_2EF078(-0.3490658402442932f, 0.3490658402442932f);
        F(rec, 0x194) = F(rec, 0x23C) * D_0015EE6C_2EF078;
        return;

    case 13:
        U16(m, 0x34) |= 1;
        if (!func_00215570_2EF078((void *)0x13F420, I(rec, 0x25C)))
            return;
        F(m, 0x18) = F(m, 0x18) + 20.0f;
        v = U16(m, 0x34) | 0x1000;
        U8(m, 0x20) = 1;
        U16(m, 0x34) = v;
        F(m, 0x58) = 1.0f;
        I(m, 0x94) = I(*(char **)(m + 0x24), 0x10);
        U8(m, 0x31) = 1;
        U16(m, 0x34) = v & 0xFFFE;
        return;

    case 14:
        if (func_L00_002DDEA0_2EF078(m, rec + 0x60)) {
            U8(m, 0x20) = 1;
            *(short *)(rec + 0x128) = 0;
        }
        return;

    default:
        return;
    }
}
