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

typedef int TI __attribute__((mode(TI)));

extern void ref_L07_003F8BB0_00(s32, s32, s8 *);
extern void ref_L07_003F8BB0_01(s8 *, s32, TI *);
extern void ref_L07_003F8BB0_02(s8 *, TI *);
extern void ref_L07_003F8BB0_03(s8 *, s32, TI *, TI *, f32);

void func_L07_003F8BB0(s8 *arg0) {
    TI loc;
    TI sp10;
    s8 *s2;

    s2 = *(s8 **)(arg0 + 0x68);
    ref_L07_003F8BB0_00(3, 0, arg0);
    ref_L07_003F8BB0_01(arg0, 0, &loc);
    ref_L07_003F8BB0_02(arg0, &sp10);
    ref_L07_003F8BB0_03(arg0, *(s32 *)(s2 + 0x418), &loc, &sp10, (f32)*(u8 *)(s2 + 0x35));
}
