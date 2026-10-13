/* 0 of 27 exact: 164 bytes, retail 172. The 128-bit copies (lq/sq) and the calls match; the copy store lands in the branch delay slot (retail: lq, sq, b, ld in the delay slot), so the two copy paths are 2 instructions off. */
typedef int Qword_T_9490 __attribute__((mode(TI)));

void func_L00_003E9490(float t, Vec16_Fc936841d *dst, Vec16_Fc936841d *src, Vec16_Fc936841d *other)
{
    Vec16_Fc936841d local;

    if (t == 0.0f) {
        *(Qword_T_9490 *)dst = *(Qword_T_9490 *)src;
    } else if (t == 1.0f) {
        *(Qword_T_9490 *)dst = *(Qword_T_9490 *)other;
    } else {
        func_00282CF0_9((char *)&local, (char *)other, (char *)src);
        func_00282D30_3((char *)&local, (char *)&local, t);
        func_00282CC0_15((char *)dst, (char *)src, (char *)&local);
    }
}

