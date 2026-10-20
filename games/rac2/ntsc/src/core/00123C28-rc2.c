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

extern s32 func_00115544(s32, s32);
extern s32 func_001157AC(s32);
extern s32 func_00123530(s32, s32);
extern s32 func_00123578(s32, s32);

extern s8 D_0013B060[];
extern s8 D_0013B068[];

typedef struct {
    s32 v;
} __attribute__((packed)) UA32;

s8 *func_00123C28(s32 arg0, s32 arg1) {
    s32 sp[4];
    s32 temp_v0;
    s32 temp_s2;
    s32 var_s0;

    temp_v0 = (arg0 - *(s32 *)(arg0 + 4)) + 0xD;
    *(s32 *)arg1 = temp_v0;
    if (func_00115544(temp_v0, (s32) D_0013B060) != 0 && func_00115544(*(s32 *)arg1, (s32) D_0013B068) != 0 && *(s8 *)*(s32 *)arg1 != 0x7A) {
        return 0;
    }
    temp_s2 = *(s32 *)arg1 + func_001157AC(*(s32 *)arg1);
    var_s0 = temp_s2 + 1;
    if (func_00115544(*(s32 *)arg1, (s32) D_0013B068) == 0) {
        *(s32 *)(arg1 + 4) = ((UA32 *)var_s0)->v;
        var_s0 = temp_s2 + 5;
    } else {
        *(s32 *)(arg1 + 4) = 0;
    }
    var_s0 = func_00123578(func_00123530(var_s0, arg1 + 8), arg1 + 0xC);
    *(s32 *)(arg1 + 0x10) = *(u8 *)var_s0;
    var_s0 = var_s0 + 1;
    if (*(s8 *)*(s32 *)arg1 == 0x7A) {
        var_s0 = func_00123530(var_s0, (s32) sp) + sp[0];
    }
    return (s8 *) var_s0;
}
