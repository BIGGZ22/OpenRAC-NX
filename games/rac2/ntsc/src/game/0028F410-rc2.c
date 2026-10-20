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

extern s32 func_00282CC0(s8 *, s8 *, s8 *);
extern s32 func_00282CF0(s8 *, s8 *, s8 *);
extern f32 func_00283180(f32);
extern f32 func_00283198(f32);
extern s32 D_001A7350;
extern s32 D_001A7354;
extern s32 D_001B187C;
extern s8 *volatile D_001B22A8;

void func_0028F410(s32 arg0, s32 arg1, s64 arg2, f32 fparg0, f32 fparg1, f32 fparg2, f32 fparg3, f32 fparg4) {
    s8 sp[0x68];
    s64 rt1 ;
    s64 ra1 ;
    s64 rt0 ;
    s64 r154 ;
    s32 s3;
    s32 s4;
    s32 t2;
    s8 *p;
    s8 *q;
    s64 c1;
    s64 c2;
    s64 c3;
    s64 c4;

    *(f32 *)(sp + 0x20) = fparg0;
    *(f32 *)(sp + 0x24) = fparg1;
    *(f32 *)(sp + 0x0) = fparg3 * func_00283198(fparg4);
    *(f32 *)(sp + 0x4) = fparg3 * func_00283180(fparg4);
    *(f32 *)(sp + 0x10) = -fparg2 * func_00283180(fparg4);
    *(f32 *)(sp + 0x14) = fparg2 * func_00283198(fparg4);
    func_00282CC0(sp + 0x30, sp + 0x20, sp);
    func_00282CF0(sp + 0x30, sp + 0x30, sp + 0x10);
    func_00282CC0(sp + 0x40, sp + 0x20, sp);
    func_00282CC0(sp + 0x40, sp + 0x40, sp + 0x10);
    func_00282CF0(sp + 0x50, sp + 0x20, sp);
    func_00282CF0(sp + 0x50, sp + 0x50, sp + 0x10);
    func_00282CF0(sp + 0x60, sp + 0x20, sp);
    func_00282CC0(sp + 0x60, sp + 0x60, sp + 0x10);

    (*(s32 *)((s8 *)(D_001B22A8) + (0))) = 0x10000007;
    rt1 = ((s64)0xB400 << 48) | 0x8001;
    s3 = arg0 << 4;
    (*(s32 *)((s8 *)(D_001B22A8) + (4))) = 0;
    ra1 = ((((s64)0xA6A6 << 16) | 0xA6A6) << 11) | 0x106;
    r154 = 0x154;
    rt0 = ((s64)0x807F << 16) | 0x7F7F;
    s4 = arg1 << 20;
    t2 = s4 + s3;
    (*(s32 *)((s8 *)(D_001B22A8) + (8))) = 0;
    (*(s32 *)((s8 *)(D_001B22A8) + (0xC))) = 0x50000007;
    p = D_001B22A8;
    q = p + 0x10;
    D_001B22A8 = q;
    (*(s64 *)((s8 *)(p) + (0x10))) = rt1;
    (*(s64 *)((s8 *)(q) + (0x10))) = arg2;
    (*(s64 *)((s8 *)(q) + (0x28))) = s3;
    (*(s64 *)((s8 *)(q) + (8))) = ra1;
    (*(s64 *)((s8 *)(q) + (0x18))) = r154;
    (*(s64 *)((s8 *)(q) + (0x20))) = rt0;
    c1 = ((s64)((s32)*(f32 *)(sp + 0x30) + D_001A7350 - 8)) | ((s64)((s32)*(f32 *)(sp + 0x34) + D_001A7354 - 8) << 16) | ((s64)D_001B187C << 32);
    (*(s64 *)((s8 *)(q) + (0x38))) = t2;
    (*(s64 *)((s8 *)(q) + (0x30))) = c1;
    c2 = ((s64)((s32)*(f32 *)(sp + 0x40) + D_001A7350 - 8)) | ((s64)((s32)*(f32 *)(sp + 0x44) + D_001A7354 - 8) << 16) | ((s64)D_001B187C << 32);
    (*(s64 *)((s8 *)(q) + (0x48))) = 0;
    (*(s64 *)((s8 *)(q) + (0x40))) = c2;
    c3 = ((s64)((s32)*(f32 *)(sp + 0x50) + D_001A7350 - 8)) | ((s64)((s32)*(f32 *)(sp + 0x54) + D_001A7354 - 8) << 16) | ((s64)D_001B187C << 32);
    (*(s64 *)((s8 *)(q) + (0x50))) = c3;
    (*(s64 *)((s8 *)(q) + (0x58))) = s4;
    c4 = ((s64)((s32)*(f32 *)(sp + 0x60) + D_001A7350 - 8)) | ((s64)((s32)*(f32 *)(sp + 0x64) + D_001A7354 - 8) << 16) | ((s64)D_001B187C << 32);
    (*(s64 *)((s8 *)(q) + (0x68))) = 0;
    (*(s64 *)((s8 *)(q) + (0x60))) = c4;
    D_001B22A8 = (s8 *)D_001B22A8 + 0x70;
}
