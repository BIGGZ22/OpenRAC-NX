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

extern s32 func_0029EFD0();
extern void func_0029FCC0();
extern void func_002A0488();
extern void func_002A0F48();
extern void func_002A7470();

typedef int TI __attribute__((mode(TI)));

s32 func_00310960(s32 arg0, f32 fparg0) {
    s32 temp;
    s32 t2;

    temp = func_0029EFD0(0xD1);
    if (temp != 0) {
        t2 = (*(s32 *)((s8 *)(temp) + (0x68)));
        (*(f32 *)((s8 *)(t2) + (4))) = fparg0;
        (*(s32 *)((s8 *)(t2) + (0))) = arg0;
        (*(u8 *)((s8 *)(temp) + (0x30))) = 0xFF;
        (*(s16 *)((s8 *)(temp) + (0x32))) = 0xFF;
        (*(s8 *)((s8 *)(temp) + (0x31))) = 1;
        func_002A0488(temp, 0x80, 0x80, 0x80);
        func_0029FCC0(arg0, 0, temp + 0x10);
        (*(TI *)((s8 *)(temp) + (0xC0))) = (*(TI *)((s8 *)(arg0) + (0xC0)));
        (*(TI *)((s8 *)(temp) + (0xD0))) = (*(TI *)((s8 *)(arg0) + (0xD0)));
        (*(TI *)((s8 *)(temp) + (0xE0))) = (*(TI *)((s8 *)(arg0) + (0xE0)));
        if ((*(u8 *)((s8 *)(temp) + (0x43))) != 0) {
            func_002A7470(temp, 0, 0, 0xA);
        }
        func_002A0F48(temp);
        (*(s8 *)((s8 *)(temp) + (0x20))) = 0;
        (*(u16 *)((s8 *)(temp) + (0x34))) = (*(u16 *)((s8 *)(temp) + (0x34))) | 4;
    }
    return temp;
}
