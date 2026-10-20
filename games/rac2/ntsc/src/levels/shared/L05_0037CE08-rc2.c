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

extern s32 ref_L05_0037CE08_00(s32);
extern s32 ref_L05_0037CE08_01(s32);
extern s32 ref_L05_0037CE08_02;
extern u16 *ref_L05_0037CE08_03[];

s32 func_L05_0037CE08(void *arg0, s32 arg1) {
    s32 var_s2;
    s32 temp_s0;
    s32 t;
    s32 inc;
    s32 r;
    u16 *temp_v0;
    u16 *var_s1;

    var_s2 = 0;
    temp_v0 = ref_L05_0037CE08_03[*(u8 *)((s8 *)arg0 + 0x21)];
    if (temp_v0 == 0) {
        return 0;
    }
    var_s1 = temp_v0;
    do {
        temp_s0 = (*var_s1 & 0x7FFF) << 8;
        if (ref_L05_0037CE08_00(ref_L05_0037CE08_02 + temp_s0) == arg1) {
            r = ref_L05_0037CE08_01(ref_L05_0037CE08_02 + temp_s0);
            inc = var_s2 + 1;
            var_s2 = (r != 0) ? inc : var_s2;
        }
        t = (s16)*var_s1;
        var_s1 += 1;
    } while (t >= 0);
    return var_s2;
}
