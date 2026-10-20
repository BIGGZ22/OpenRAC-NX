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

extern s32 func_00288FB8();
extern s32 func_00335E20();
extern s32 func_00335F88(s32, f32, f32, f32, f32);
extern s32 func_003361E0();
extern s32 func_00336390();
extern s32 func_00336408();
extern s32 func_00336498();
extern s32 func_003364B0();
extern s32 func_00336710();
extern s32 func_003369A8();
extern s32 func_00336A30();
extern s32 func_00336A38();
extern s32 func_00336B90();
extern s32 func_00336B98();
extern s32 func_00336CC0();
extern s32 func_00336CC8();
extern s32 func_00336CE0();
extern s32 func_00336DA8();
extern s32 func_00336E40();
extern s32 func_00346C60();
extern s32 D_001A8CFC;
extern s8 D_001AE4D8[];
extern s8 D_001ADFC0[];
extern s8 D_001ADFA0[];
extern s8 D_001ADFD8[];
extern s8 D_001ADC18[];
extern s8 D_001ADC10[];
extern s8 D_001ADFC8[];
extern s8 D_001ADC28[];
extern s8 D_001ADFD0[];
extern s8 D_001AE4D0[];
extern s8 D_001AE040[];
extern s8 D_001ADC30[];
extern s8 D_001AE4E8[];
extern s8 D_001AE4F8[];

void func_00346710(s32 arg0, s32 arg1) {
    s32 sp0;
    s32 sp4;
    s32 sp8;
    s32 spC;
    s32 sp10;
    s32 sp14;
    s32 sp18;
    s32 sp1C;
    s32 sp20;
    s32 sp24;
    s32 sp28;
    s32 sp2C;
    s32 sp30;
    s32 *temp_v0_2;
    s32 *temp_v0_3;
    s32 temp_s0;
    s32 temp_s0_2;
    s32 temp_v0;

    (*(s32 *)((s8 *)(arg0) + (0x49C))) = arg1;
    if (arg1 != 0) {
        temp_v0 = func_00336DA8(0x10, func_00336E40(arg1));
        (*(s32 *)((s8 *)(arg0) + (0x498))) = temp_v0;
        (*(s32 *)((s8 *)(temp_v0) + (4))) = 0;
        (*(s32 *)((s8 *)(temp_v0) + (8))) = 0;
        (*(s32 *)((s8 *)(temp_v0) + (0xC))) = 0;
        (*(s32 *)((s8 *)(temp_v0) + (0))) = 0;
    }
    (*(s32 *)((s8 *)(arg0) + (0x4F8))) = 1;
    (*(f32 *)((s8 *)((*(s32 *)((s8 *)(arg0) + (0x498)))) + (0))) = 250.0f;
    sp0 = arg0 + 0x4C;
    temp_s0 = arg0 + 0x1C8;
    sp4 = arg0 + 0x130;
    sp8 = arg0 + 0x98;
    (*(f32 *)((s8 *)((*(s32 *)((s8 *)(arg0) + (0x498)))) + (4))) = 207.0f;
    spC = arg0 + 0x400;
    func_003361E0(arg0, D_001ADC10, arg1);
    sp10 = arg0 + 0x44C;
    func_003361E0(sp0, D_001ADC18, arg1);
    func_003361E0(sp4, D_001ADFC8, arg1);
    sp14 = arg0 + 0xE4;
    func_003361E0(sp8, D_001ADC28, arg1);
    sp18 = arg0 + 0x17C;
    func_003361E0(spC, D_001ADFD0, arg1);
    func_003361E0(sp10, D_001AE4D0, arg1);
    func_00336408(temp_s0, 0x20, 0, D_001AE040, arg1);
    func_00336CC8(temp_s0, 5);
    func_00336B90(temp_s0, 0x64);
    func_00336B98(temp_s0, 0);
    func_00336498(temp_s0, 0x6049C1FF, 0x60001EFF);
    func_003364B0(temp_s0, 0x50F0C070, 0x50F0C070);
    sp1C = arg0 + 0x378;
    func_00336CE0(temp_s0, 0x80000000);
    func_00336CC0(temp_s0, 0);
    temp_s0_2 = arg0 + 0x3B4;
    func_003361E0(sp14, D_001ADFC0, arg1);
    sp20 = arg0 + 0x210;
    func_003361E0(sp18, D_001AE4D8, arg1);
    sp28 = arg0 + 0x2A0;
    sp24 = arg0 + 0x258;
    temp_v0_2 = func_00335E20(arg0);
    sp2C = arg0 + 0x2E8;
    *temp_v0_2 = 0x60442D00;
    temp_v0_3 = func_00335E20(sp0);
    sp30 = arg0 + 0x330;
    *temp_v0_3 = 0x55F0C070;
    *(s32 *)func_00335E20(sp4) = 0x80FFDE8D;
    *(s32 *)func_00335E20(sp8) = 0x55F0C070;
    *(s32 *)func_00335E20(sp14) = 0x80FFDE8D;
    *(s32 *)func_00335E20(sp18) = 0x60241700;
    *(s32 *)func_00335E20(spC) = 0x60442D00;
    *(s32 *)func_00335E20(sp10) = 0x55F0C070;
    func_00336390(arg0, D_001A8CFC + 0x8710, 0x21);
    func_00336390(sp0, D_001A8CFC + 0x8710, 0x22);
    func_00336390(sp4, D_001A8CFC + 0x8710, 0x28);
    func_00336390(sp8, D_001A8CFC + 0x8710, 0x23);
    func_00336390(sp14, D_001A8CFC + 0x8710, 0x27);
    func_00336390(sp18, D_001A8CFC + 0x8710, 0x2A);
    func_00336390(spC, D_001A8CFC + 0x8710, 0x2C);
    func_00336390(sp10, D_001A8CFC + 0x8710, 0x2B);
    (*(s32 *)((s8 *)(arg0) + (0x4FC))) = 0;
    (*(s32 *)((s8 *)(arg0) + (0x500))) = 0;
    func_00336710(sp1C, D_001ADFD8, arg1);
    *(s32 *)func_00335E20(sp1C) = 0x60F0F0B0;
    func_00335F88(sp1C, 32.0f, 32.0f, 0.0f, 0.0f);
    func_003361E0(temp_s0_2, D_001ADFA0, arg1);
    *(s32 *)func_00335E20(temp_s0_2) = 0x60241700;
    func_00336390(temp_s0_2, D_001A8CFC + 0x8710, 0x29);
    func_00335F88(temp_s0_2, 1.0f, 1.0f, 0.0f, 0.0f);
    func_003369A8(sp20, D_001ADC30, arg1);
    func_003369A8(sp24, D_001ADC30, arg1);
    func_00336A30(sp24, 1);
    func_003369A8(sp28, D_001ADC30, arg1);
    func_00336A30(sp28, 1);
    func_003369A8(sp2C, D_001AE4E8, arg1);
    func_00336A30(sp2C, 1);
    func_003369A8(sp30, D_001AE4F8, arg1);
    func_00336A30(sp30, 1);
    *(s32 *)func_00335E20(sp20) = 0x80F0F0F0;
    *(s32 *)func_00335E20(sp24) = 0x80F0F0F0;
    *(s32 *)func_00335E20(sp28) = 0x80F0F0F0;
    *(s32 *)func_00335E20(sp2C) = 0x80F0F0F0;
    *(s32 *)func_00335E20(sp30) = 0x80F0F0F0;
    func_00336A38(sp20, func_00288FB8(0x2D51));
    func_00336A38(sp24, arg0 + 0x4A0);
    func_00336A38(sp28, arg0 + 0x4B8);
    func_00346C60(arg0, 0);
}
