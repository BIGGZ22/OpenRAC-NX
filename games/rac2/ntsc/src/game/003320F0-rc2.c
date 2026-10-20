/* SPDX-License-Identifier: MIT
 * MIT License
 *
 * Copyright (c) 2026 RC2 decomp contributions
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "common.h"

extern s32 D_001AFC6C;
extern s32 g_001AFC6C __attribute__((sda));
extern f32 D_001B0A80 __attribute__((sda));
extern f32 D_001B0A84 __attribute__((sda));
extern s8 D_001B5200[];

struct Vec4 { f32 x; f32 y; f32 z; f32 w; };
struct Obj { s8 pad[0x10]; struct Vec4 pos; };
struct Tab { s8 pad[0x140]; f32 f140; f32 f144; };

f32 func_002A7878(f32 a, f32 b);
f32 func_002A7940(void);
void func_002AF000(struct Vec4 *p, f32 x, f32 y, f32 z);
s32 func_002A7820(s32 a, s32 b);
void func_002BDBC0(struct Vec4 *a, struct Vec4 *b, u32 c, s32 d, s32 e, f32 f);
void func_00282CC0(struct Vec4 *a, struct Vec4 *b, struct Vec4 *c);
f32 func_00283248(f32 a, f32 b);
f32 func_002A78D8(f32 a, f32 b);
void func_00283CF0(f32 a);
f32 func_00283B98(f32 a, f32 b);
f32 func_00283180(f32 a);
f32 func_00283198(f32 a);
void func_002C7E28(struct Vec4 *a, struct Vec4 *b, s32 c);
void func_003323F0(struct Obj *a);
void func_0029F338(struct Obj *a);

void func_003320F0(struct Obj *arg0)
{
    struct Vec4 b;
    struct Vec4 a;
    struct Tab *tb;
    s32 i;
    s32 j;
    f32 ra;
    f32 rb;
    f32 rc;
    f32 rd;
    f32 re;
    f32 rf;
    f32 rg;
    s32 rh;
    f32 p1;
    f32 p2;
    f32 r3;
    f32 r4;
    f32 r5;
    f32 r6;
    f32 pa;
    f32 pb;
    f32 d80;

    i = 0;
    if (0x21 - D_001AFC6C > 0) {
        do {
            i++;
            ra = func_002A7878(0.0f, 0.3700000047683716f);
            rb = func_002A7940();
            rc = func_002A7940();
            func_002AF000(&a, ra, rb, rc);
            rd = func_002A7878(0.3499999940395355f, 0.6000000238418579f);
            a.z = a.z + rd;
            func_00282CC0(&a, &a, &arg0->pos);
            re = func_002A7878(0.0016666668234393f, 0.01666666753590107f);
            rf = func_002A7940();
            rg = func_002A7940();
            func_002AF000(&b, re, rf, rg);
            rh = func_002A7820(0x1E, 0x39);
            func_002BDBC0(&a, &b, 0x4F20507F, 0x102F, rh, 150000.0f);
        } while (i < 0x21 - D_001AFC6C);
    }
    j = 0;
    if (0x32 - D_001AFC6C > 0) {
        tb = (struct Tab *)D_001B5200;
        do {
            j++;
            p1 = func_00283248(tb->f140 - arg0->pos.x, tb->f144 - arg0->pos.y);
            func_00283CF0(func_002A78D8(1.0f, 2.0f));
            r3 = func_002A78D8(0.0f, 0.15000000596046448f);
            b.x = arg0->pos.x + r3;
            r4 = func_002A78D8(0.0f, 0.15000000596046448f);
            b.y = arg0->pos.y + r4;
            r5 = func_002A78D8(0.0f, 0.15000000596046448f);
            b.z = arg0->pos.z + r5;
            d80 = D_001B0A80;
            r6 = func_002A78D8(0.0f, -1.5707964897155762f);
            p2 = func_00283B98(p1, r6);
            pa = func_00283180(p2);
            a.x = pa * 0.01666666753590107f * d80;
            pb = func_00283198(p2);
            a.y = pb * 0.01666666753590107f * d80;
            d80 = D_001B0A84;
            a.z = d80 * 0.01666666753590107f;
            func_002C7E28(&b, &a, 0x12C);
        } while (j < 0x32 - D_001AFC6C);
    }
    g_001AFC6C = D_001AFC6C - 1;
    func_003323F0(arg0);
    func_0029F338(arg0);
}
