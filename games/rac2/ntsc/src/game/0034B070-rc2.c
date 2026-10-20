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

extern s32 func_00335AC8();
extern s32 func_003361A8();
extern s32 func_003366D0();
extern s32 func_00336968();
extern s32 func_00349498();
extern s32 func_00349918();
extern s32 func_00349CB8();

extern s8 D_001AD928[];
extern s8 D_001AD908[];
s32 func_0034B070(s32 arg0) {
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
    s32 sp34;
    s32 sp38;
    s32 var_s1;
    s32 var_s1_2;
    s32 var_s2;
    s32 var_s2_2;

    var_s2 = 7;
    var_s1 = arg0 + 0x358;
    func_003361A8(arg0);
    func_003361A8(arg0 + 0x4C);
    func_003361A8(arg0 + 0x98);
    func_00336968(arg0 + 0x100);
    func_003366D0(arg0 + 0x148);
    func_003361A8(arg0 + 0x184);
    func_003361A8(arg0 + 0x1D0);
    func_00336968(arg0 + 0x238);
    func_003366D0(arg0 + 0x280);
    func_003361A8(arg0 + 0x2C0);
    func_003361A8(arg0 + 0x30C);
    do {
        var_s2 -= 1;
        func_003361A8(var_s1);
        var_s1 += 0x4C;
    } while (var_s2 != -1);
    var_s1_2 = arg0 + 0x604;
    func_003361A8(arg0 + 0x5B8);
    var_s2_2 = 7;
    sp24 = arg0 + 0xE0C;
    sp38 = arg0 + 0xE94;
    sp10 = arg0 + 0xF1C;
    sp2C = arg0 + 0xFB0;
    sp14 = arg0 + 0x1044;
    sp34 = arg0 + 0x10D8;
    sp4 = arg0 + 0x1160;
    sp18 = arg0 + 0x11F4;
    sp8 = arg0 + 0x1288;
    sp1C = arg0 + 0x131C;
    spC = arg0 + 0x13B0;
    sp20 = arg0 + 0x1444;
    sp28 = arg0 + 0x1448;
    sp30 = arg0 + 0x144C;
    sp0 = arg0 + 0x14D4;
    do {
        var_s2_2 -= 1;
        func_003366D0(var_s1_2);
        var_s1_2 += 0x3C;
    } while (var_s2_2 != -1);
    func_00349CB8(arg0 + 0x7E8);
    func_00349498(arg0 + 0xBEC);
    func_00349498(arg0 + 0xC74);
    func_00349498(arg0 + 0xCFC);
    func_00349498(arg0 + 0xD84);
    func_00349498(sp24);
    func_00349498(sp38);
    func_00349918(sp10);
    func_00349918(sp2C);
    func_00349918(sp14);
    func_00349498(sp34);
    func_00349918(sp4);
    func_00349918(sp18);
    func_00349918(sp8);
    func_00349918(sp1C);
    func_00349918(spC);
    func_00335AC8(sp20);
    (*(s32 *)((s8 *)(arg0) + (0x1444))) = ((s32)D_001AD928);
    func_00335AC8(sp28);
    (*(s32 *)((s8 *)(arg0) + (0x1448))) = ((s32)D_001AD908);
    func_00349498(sp30);
    func_00349498(sp0);
    return arg0;
}
