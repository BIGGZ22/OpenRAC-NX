/* KillPart(rec): frees particle record rec (2048 records of 0x40 bytes at the pool pointer).
 * Live count--; clears the record's bit in the allocation bitmap (and in the second bitmap when
 * byte 1 has 0x40); when it was the highest live index, moves that index down to the next set
 * bit; sets byte 1 to 0x80; lowers the lowest-free hint to the record's index if smaller. */
extern unsigned char *D_0016022C SDATA(D_0016022C);
extern int D_00160230 SDATA(D_00160230);
extern int D_00160234 SDATA(D_00160234);
extern int D_00160238 SDATA(D_00160238);
extern unsigned char D_L00_001B1C00[];

void func_L00_002688A8(unsigned char *rec) {
    unsigned int idx;
    int top;

    idx = (unsigned int)((char *)rec - (char *)D_0016022C) >> 6;
    D_00160238--;
    if (rec[1] & 0x40) {
        D_L00_001B1C00[0x100 + (idx >> 3)] &= (unsigned char)~(1 << (idx & 7));
    }
    D_L00_001B1C00[idx >> 3] &= (unsigned char)~(1 << (idx & 7));
    if ((int)idx == D_00160234) {
        top = D_00160234 - 1;
        while (top >= 0 && !(D_L00_001B1C00[top >> 3] & (1 << (top & 7)))) {
            top--;
        }
        D_00160234 = top;
    }
    rec[1] = 0x80;
    if ((int)idx < D_00160230) {
        D_00160230 = (int)idx;
    }
}
