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

extern void ref_L11_003F2C28_00(void *, void *);
extern void ref_L11_003F2C28_01(void *, s32, s32, s32);
extern void ref_L11_003F2C28_02(void *, void *);
extern void ref_L11_003F2C28_03(void *, void *, s32, s32, f32);
extern void ref_L11_003F2C28_04(void);
extern f32 ref_L11_003F2C28_05 __attribute__((sda));
extern u8 ref_L11_003F2C28_06[];
extern u8 ref_L11_003F2C28_07[];

void func_L11_003F2C28(void *arg0) {
    void *temp_s0 = *(void **)((s8 *)arg0 + 0x68);
    s32 *state = (s32 *)((s8 *)temp_s0 + 0x28C);
    s8 *cursor;
    void *call_buffer;
    s32 remaining;
    u16 count;

    ref_L11_003F2C28_04();
    ref_L11_003F2C28_00(ref_L11_003F2C28_06, arg0);
    ref_L11_003F2C28_00(ref_L11_003F2C28_07, arg0);
    ref_L11_003F2C28_02(arg0, (s8 *)temp_s0 + 0x70);

    if (*(s16 *)((s8 *)temp_s0 + 0x70) == 0) {
        if (*state != 0) {
            cursor = (s8 *)temp_s0 + 0x274;
            goto loop_setup;
        }
        ref_L11_003F2C28_01(arg0,
                      *(s32 *)((s8 *)temp_s0 + 0x280),
                      *(s32 *)((s8 *)temp_s0 + 0x284),
                      *(s32 *)((s8 *)temp_s0 + 0x288));
        *state = 1;
    } else {
        *state = 0;
    }
    cursor = (s8 *)temp_s0 + 0x274;
loop_setup:
    call_buffer = (s8 *)temp_s0 + 0x2A0;
    remaining = 1;
    do {
        if (*(s16 *)cursor < 0x7FFF) {
            *(s16 *)cursor = (u16)*(s16 *)cursor + 1;
        }
        remaining--;
        cursor += 2;
    } while (remaining >= 0);

    count = *(u16 *)((s8 *)arg0 + 0x96);
    if (count <= 0xFFFEU) {
        *(u16 *)((s8 *)arg0 + 0x96) = count + 1;
    }
    ref_L11_003F2C28_03(arg0, call_buffer, 0xC, 1, ref_L11_003F2C28_05);
}
