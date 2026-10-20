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

extern s32 func_00282948(s8 *);
extern void func_00282CC0(s8 *, s8 *, s8 *);
extern void func_00282D30(s8 *, s8 *, f32);
extern f32 func_00283B98(f32, f32);
extern f32 func_00283CE0(s32);
extern s32 func_00283CF0(f32);
extern void func_002FE8D8(s32, s8 *);
extern s8 D_002329A0[];
extern s8 D_00189E20[];

void func_0032AF50(s8 *arg0) {
    s8 *base ;
    s8 *s0;
    s8 *s1;
    s8 *s3;
    s8 *s6;
    s32 s2;
    s32 s5;
    s32 temp_v0;
    f32 v[3];
    f32 t;
    f32 t20;

    s5 = 0;
    base = D_002329A0;
    s3 = base + 0x3C;
    s1 = base + 0x10;
    s0 = base;
    s6 = D_00189E20;
    s2 = *(s32 *)(arg0 + 0x68);
    do {
        s5++;
        if ((s8 *)(s64)*(s32 *)(s0 + 0x44) == arg0) {
            t = func_00283CE0(*(s32 *)(s0 + 0x3C));
            t20 = t / func_00283CE0(*(s32 *)(s0 + 0x40));
            *(s32 *)(s0 + 0x34) = func_00283CF0(func_00283CE0(*(s32 *)(s0 + 0x30)) * t20);
            *(f32 *)(s0 + 0x2C) = func_00283B98(*(f32 *)(s0 + 0x2C), *(f32 *)(s0 + 0x28));
            func_00282CC0(s0, s0, s1);
            func_002FE8D8(*(s32 *)(s6 + 0x2290) + 0x10, v);
            func_00282D30(v, v, -0.00089999998454f);
            func_00282CC0(s1, s1, v);
            if (func_00282948(s3) != 0) {
                temp_v0 = *(s32 *)(s0 + 0x34);
                if (temp_v0 > 0) {
                    *(s32 *)(s0 + 0x34) = temp_v0 - 1;
                } else {
                    *(s32 *)(s0 + 0x44) = 0;
                    *(s32 *)s2 -= 1;
                }
            }
        }
        s3 += 0x50;
        s1 += 0x50;
        s0 += 0x50;
    } while (s5 < 0x80);
}
