/* Candidate only: the full-program build and compiler profile are not qualified.
   Measured per-source option: -mno-check-zero-division (config/cflags.txt). */
void dlib_displist_direct_end__FP13DLIB_DISPLIST(int *list) {
    unsigned long long *tag = (unsigned long long *)list[3];
    int n;

    n = (((list[1] - list[3]) >> 4) - 1) / (int)(*tag >> 60);
    *tag = (*tag & (unsigned long long)-0x8000) | (n & 0x7FFF);
    list[3] = 0;
    *(unsigned short *)list[4] = (unsigned short)((list[1] - list[4]) >> 4);
    list[4] = 0;
    {
        int last = (list[1] - list[2]) >> 4;

        *(unsigned short *)list[2] = (unsigned short)(last - 1);
    }
    list[2] = list[1];
    list[1] += 0x10;
}
