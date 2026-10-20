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

extern void func_00115484(s8 *, s32, s32);
extern s32 func_00288FB8(s32);
extern s32 *func_00335E20(s8 *);
extern void func_00335F88(s8 *, f32, f32, f32, f32);
extern void func_003361E0(s8 *, s8 *, s8 *);
extern void func_00336390(s8 *, s32, s32);
extern void func_003363C0(s8 *, f32);
extern void func_00336710(s8 *, s8 *, s8 *);
extern void func_00336920(s8 *, s32, s32);
extern void func_003369A8(s8 *, s8 *, s8 *);
extern void func_00336A38(s8 *, s32);
extern s8 *func_00336DA8(s32, s32);
extern s32 func_00336E40(s8 *);
extern void func_00347EC8(s8 *, s8 *);
extern s32 D_001A8CFC;
extern s8 D_001ADC10[];
extern s8 D_001ADC18[];
extern s8 D_001ADC20[];
extern s8 D_001ADC28[];
extern s8 D_001ADC30[];
extern s8 D_001ADFC0[];
extern s8 D_001AE0C0[];

void func_00341220(s8 *arg0, s8 *arg1) {
    s8 *p;
    s8 *s5;
    s8 *s2;
    s8 *s3;
    s8 *s4;
    s8 *s6;
    s8 *l0;

    (*(s8 **)((s8 *)(arg0) + (0x21C))) = arg1;
    if (arg1 != 0) {
        p = func_00336DA8(0x10, func_00336E40(arg1));
        (*(s8 **)((s8 *)(arg0) + (0x218))) = p;
        (*(s32 *)((s8 *)(p) + (4))) = 0;
        (*(s32 *)((s8 *)(p) + (8))) = 0;
        (*(s32 *)((s8 *)(p) + (0xC))) = 0;
        (*(s32 *)((s8 *)(p) + (0))) = 0;
    }
    (*(s32 *)((s8 *)(arg0) + (0x220))) = 1;
    (*(f32 *)((s8 *)((*(s8 **)((s8 *)(arg0) + (0x218)))) + (0))) = 251.0f;
    s5 = arg0 + 0x98;
    s2 = arg0 + 0x4C;
    s3 = arg0 + 0xE4;
    (*(f32 *)((s8 *)((*(s8 **)((s8 *)(arg0) + (0x218)))) + (4))) = 170.0f;
    s4 = arg0 + 0x130;
    func_003361E0(arg0, D_001ADC10, arg1);
    s6 = arg0 + 0x17C;
    l0 = arg0 + 0x1D0;
    func_003361E0(s5, D_001ADC18, arg1);
    func_003361E0(s2, D_001ADC20, arg1);
    func_003361E0(s3, D_001ADC28, arg1);
    func_003361E0(s4, D_001ADFC0, arg1);
    func_003369A8(l0, D_001ADC30, arg1);
    func_00336710(s6, D_001AE0C0, arg1);
    func_00347EC8(arg0 + 0x228, arg1);
    *func_00335E20(arg0) = 0x60442D00;
    *func_00335E20(s5) = 0x80FFDE8D;
    *func_00335E20(s2) = 0x55F0C070;
    *func_00335E20(s3) = 0x55F0C070;
    *func_00335E20(s4) = 0x70FFFEED;
    *func_00335E20(l0) = 0x80F0F0F0;
    *func_00335E20(s6) = 0x60F0F0B0;
    func_003363C0(arg0, 1.0f);
    func_003363C0(s2, 1.0f);
    func_003363C0(s3, 1.0f);
    func_00336390(arg0, D_001A8CFC + 0x8710, 0x91);
    func_00336390(s5, D_001A8CFC + 0x8710, 0x94);
    func_00336390(s2, D_001A8CFC + 0x8710, 0x92);
    func_00336390(s3, D_001A8CFC + 0x8710, 0x93);
    func_00336390(s4, D_001A8CFC + 0x8710, 0x95);
    func_00335F88(s4, 1.0f, 1.05f, 0.0f, 0.0f);
    func_00336A38(l0, func_00288FB8(0x2D55));
    func_00336920(s6, 0xEA9D, 0);
    func_00115484(arg0 + 0x1B8, 0, 12);
    func_00335F88(s6, 32.0f, 32.0f, 0.0f, 0.0f);
}
