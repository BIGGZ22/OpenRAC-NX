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

extern s32 func_002761F8();
extern s32 func_00282CC0();
extern s32 func_00282CF0();
extern s32 func_00282D30(s32, s32, f32);
extern s32 func_00282FB8(s32, s32, f32);

extern s8 D_001B5340[];
s32 func_002E4528(s32 arg0, s32 arg1) {
    s32 temp_ret;
    s32 sp[1];
    func_00282CF0(sp, arg0 + 0x20, ((s32)D_001B5340));
    func_00282D30(sp, sp, 0.75f);
    func_00282FB8(sp, sp, 64.0f);
    func_00282CC0(sp, sp, ((s32)D_001B5340));
    temp_ret = func_002761F8(arg1, sp, 0x82, (*(s32 *)((s8 *)(arg0) + (0x18))), 0);
    return temp_ret;
}
