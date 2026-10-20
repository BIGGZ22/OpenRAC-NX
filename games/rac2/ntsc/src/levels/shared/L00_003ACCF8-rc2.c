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

extern void ref_L00_003ACCF8_00(void *, void *);
extern void ref_L00_003ACCF8_01(void *, s32);
extern void ref_L00_003ACCF8_02(void *, s32, f32, s32, s32, f32);
extern s32 ref_L00_003ACCF8_03 __attribute__((sda));
extern s8 ref_L00_003ACCF8_04[];

void func_L00_003ACCF8(void *arg0) {
    s8 buf[0x10];
    s8 *s0;

    s0 = *(s8 **)((s8 *)arg0 + 0x68);
    ref_L00_003ACCF8_00(arg0, buf);
    ref_L00_003ACCF8_01(buf, ref_L00_003ACCF8_03);
    ref_L00_003ACCF8_02(buf, (*(s32 *)(s0 + 0x2C) << 6) + (s32)ref_L00_003ACCF8_04, 0.0f, ref_L00_003ACCF8_03, 4, 0.25f);
}
