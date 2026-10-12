

typedef int s32;

typedef unsigned char u8;

/* Adapted mechanically from Lombyte src/sdk/library/dual_prime_vector.c.
 * Source last change f18ea57e965bf7e88630871cc3b230b05b07c587;
 * current upstream2c4452dd03f5f7ebb2868dbe073ccdb1afb27f61 has identical source.
 * Source SHA256 e6df19f23bb23e15cf030b7d3c5222fae6d1fc4ba4411cbff4a03f31fc014b51.
 * Only the include of already-identical EE u8/s32 types and function name
 * change. Algorithm, parameter types, layout, expressions and order unchanged.
 * RAC1 matching source/object identity does not prove this RAC2 C candidate.
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
typedef struct {
    u8 pad0[0x174];
    s32 pictStruct;
    s32 topFieldFirst;
} MpegDec;

void func_0012B3E0(MpegDec *d, s32 DMV[][2], s32 *dmvector, s32 mvx, s32 mvy) {
    if (d->pictStruct == 3) {
        if (d->topFieldFirst) {
            DMV[0][0] = ((mvx + (mvx > 0)) >> 1) + dmvector[0];
            DMV[0][1] = ((mvy + (mvy > 0)) >> 1) + dmvector[1] - 1;
            DMV[1][0] = ((3 * mvx + (mvx > 0)) >> 1) + dmvector[0];
            DMV[1][1] = ((3 * mvy + (mvy > 0)) >> 1) + dmvector[1] + 1;
        } else {
            DMV[0][0] = ((3 * mvx + (mvx > 0)) >> 1) + dmvector[0];
            DMV[0][1] = ((3 * mvy + (mvy > 0)) >> 1) + dmvector[1] - 1;
            DMV[1][0] = ((mvx + (mvx > 0)) >> 1) + dmvector[0];
            DMV[1][1] = ((mvy + (mvy > 0)) >> 1) + dmvector[1] + 1;
        }
    } else {
        DMV[0][0] = ((mvx + (mvx > 0)) >> 1) + dmvector[0];
        DMV[0][1] = ((mvy + (mvy > 0)) >> 1) + dmvector[1];
        if (d->pictStruct == 1) {
            DMV[0][1]--;
        } else {
            DMV[0][1]++;
        }
    }
}
