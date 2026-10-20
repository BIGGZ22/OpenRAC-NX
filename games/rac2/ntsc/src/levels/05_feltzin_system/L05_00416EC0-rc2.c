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

extern s32 ref_L05_00416EC0_00(void *, s32, s32 *, s8 *);
extern s32 ref_L05_00416EC0_01(s8 *, s8 *, f32);
extern s32 ref_L05_00416EC0_02(s8 *, s8 *, void *);
extern s32 ref_L05_00416EC0_03(s8 *, s8 *, s8 *);
extern s32 ref_L05_00416EC0_04(void *, void *, s8 *);

void func_L05_00416EC0(void *arg0, s32 arg1) {
    s8 frame[0x80];
    s8 *base = (s8 *)arg0;
    void *temp_s0;

    *(s32 *)(frame + 0x70) = arg1;
    temp_s0 = *(void **)(base + 0x68);
    ref_L05_00416EC0_00(arg0, 1, (s32 *)(frame + 0x70), frame);
    ref_L05_00416EC0_01(frame + 0x40, frame + 0x30,
        *(f32 *)(base + 0x2C) * 0.00097656f);
    ref_L05_00416EC0_02(frame + 0x50, frame + 0x40, temp_s0);
    ref_L05_00416EC0_02(frame + 0x60, frame + 0x40, base + 0xC0);
    ref_L05_00416EC0_03(frame + 0x60, frame + 0x50, frame + 0x60);
    ref_L05_00416EC0_04(base + 0x10, base + 0x10, frame + 0x60);
}
