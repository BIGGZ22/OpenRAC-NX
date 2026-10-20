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
typedef struct {
    TI sp0;
    TI sp10;
} Scratch;

extern void ref_L02_00321C30_00(void *, void *, void *);
extern void ref_L02_00321C30_01(void *, void *, void *);
extern f32 ref_L02_00321C30_02(void *);
extern void ref_L02_00321C30_03(void *, void *, f32);

f32 func_L02_00321C30(void *arg0, void *arg1, f32 fparg0) {
    Scratch scratch;
    f32 temp_f22;
    f32 var_f20;
    f32 neg_limit;

    ref_L02_00321C30_01(&scratch.sp0, arg1, arg0);
    var_f20 = ref_L02_00321C30_02(&scratch.sp0);
    temp_f22 = var_f20;
    if (fparg0 < var_f20) {
        var_f20 = fparg0;
    } else {
        neg_limit = -fparg0;
        if (var_f20 < neg_limit) {
            var_f20 = neg_limit;
        }
    }
    ref_L02_00321C30_03(&scratch.sp0, &scratch.sp0, var_f20);
    ref_L02_00321C30_00(&scratch.sp10, arg0, &scratch.sp0);
    return temp_f22 - var_f20;
}
