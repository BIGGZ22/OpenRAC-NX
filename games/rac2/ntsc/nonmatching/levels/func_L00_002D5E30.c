/* 172 bytes vs 132 (not matched): the loop is misread. Retail stores the running sum to *a0 on
   each pass, adds *a2 from the flag (M>>5 < sum) and walks halfwords from 0x262262; the early
   exits and the sum update order are not right here. Start from the disassembly, not this C. */
extern short D_L00_00262262[];

extern int D_001A7A0C;

void func_L00_002D5E30(int *a0, int *a1, int *a2)
{
    int sum = 0;
    int i = 0;
    short *t0 = D_L00_00262262;
    short *t1 = t0 - 1;
    int m = D_001A7A0C;
    int v = t1[0];

    if (v < 0) {
        *a1 = sum;
        return;
    }
    sum = v;
    if ((m >> 5) < sum) {
        *a1 = sum;
        return;
    }
    for (;;) {
        i++;
        *a2 += (m >> 5) < sum;
        *a0 = sum;
        if (i >= 77)
            break;
        v = t1[2];
        if (v < 0)
            break;
        sum += v;
        if ((m >> 5) < sum)
            break;
        t0 += 2;
        t1 += 2;
    }
    *a1 = sum;
}

