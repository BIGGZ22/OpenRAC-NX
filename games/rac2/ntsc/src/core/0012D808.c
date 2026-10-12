

typedef int s32;

typedef unsigned char u8;

typedef unsigned int u32;

/* Mechanical source-first reuse of Lombyte SDK/library/cpr8.c and its
 * project-owned type header. Current upstream2c4452dd/source snapshot/object
 * proofs are retained in the private source packet. All original source
 * expressions, goto/loop order, field widths and hardware operations remain.
 * Type/function identifiers are namespaced, and existing EE scalar types are
 * reused. Four original helper calls use independently full-byte-identified
 * RAC2 targets; these privileged helpers remain original, with zero C credit.
 * RAC2 acceptance requires an independent current whole-unit qualification.
 */
/*
MIT License

Copyright (c) 2026 Mateusz Kłysz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/


struct Rac1Cpr8Input {
    u8 pad_0[0xD8];
    s32 unkD8;
};

struct Rac1Cpr8Output {
    s32 unk0;
    u8 pad_4[0x8];
    s32 unkC;
    s32 unk10;
};

struct Rac1Cpr8InputView {
    u8 pad_0[0xE0];
    s32 unkE0;
    s32 unkE4;
    u8 pad_E8[0x8C];
    s32 unk174;
};

extern s32 func_0011F5E0();

extern s32 func_0011F628();

void func_0012D808(struct Rac1Cpr8Input *arg0, struct Rac1Cpr8Output *arg1) {
    struct Rac1Cpr8InputView *sp0;
    s32 sp4;
    s32 sp8;
    s32 temp_2_106;
    s32 temp_2_73;
    s32 temp_4_27;
    s32 var_16_70;
    s32 var_17_68;
    s32 var_18_22;
    s32 var_20_40;
    s32 var_21_38;
    s32 var_30_43;
    s32 var_3_29;
    s32 var_5_64;

    sp0 = (struct Rac1Cpr8InputView *)arg0;
    var_18_22 = arg1->unk0 & 0x0FFFFFFF;
    sp8 = arg0->unkD8 & 0x0FFFFFFF;
    if (sp0->unk174 == 3) {
        goto block_3;
    }
    temp_4_27 = sp0->unkE0;
    var_3_29 = temp_4_27;
    if (temp_4_27 != 0) {
        goto block_8;
    }
    goto block_4;
block_3:
    var_3_29 = sp0->unkE0;
block_4:
    var_21_38 = arg1->unk10 * 0x180;
    var_20_40 = var_21_38 >> 4;
    if (var_3_29 == 0) {
        goto block_6;
    }
    var_30_43 = (var_3_29 >> 4) * 0x180;
    goto block_7;
block_6:
    var_30_43 = var_21_38;
block_7:
    sp4 = 1;
    goto block_9;
block_8:
    var_30_43 = (temp_4_27 >> 4) * 0xC0;
    var_21_38 = ((s32) arg1->unk10 >> 1) * 0x180;
    sp4 = 2;
    var_20_40 = var_21_38 >> 4;
block_9:
    var_5_64 = 0;
    while (var_5_64 < sp4) {
        var_17_68 = sp8;
        for (var_16_70 = 0; var_16_70 < arg1->unkC; var_16_70++) {
            temp_2_73 = func_0011F5E0();
            *(volatile s32 *)0x1000D480 = 0;
            *(volatile s32 *)0x1000D410 = var_18_22;
            *(volatile s32 *)0x1000D420 = var_20_40;
            *(volatile s32 *)0x1000D400 = 0x101;
            if (temp_2_73 == 0) {
                goto block_15;
            }
            func_0011F628();
        block_15:
            while (*(volatile u32 *)0x1000D400 & 0x100) {
            }
            temp_2_106 = func_0011F5E0();
            *(volatile s32 *)0x1000D080 = 0;
            *(volatile s32 *)0x1000D010 = var_17_68;
            *(volatile s32 *)0x1000D020 = var_20_40;
            *(volatile s32 *)0x1000D000 = 0x100;
            if (temp_2_106 == 0) {
                goto block_19;
            }
            func_0011F628();
        block_19:
            while (*(volatile u32 *)0x1000D000 & 0x100) {
            }
            while (*(volatile u32 *)0x1000D020 != 0) {
            }
            var_17_68 += var_30_43;
            var_18_22 += var_21_38;
        }
        var_5_64++;
        sp8 += sp0->unkE4 * 0xC0;
    }
    return;
}
