/* functional: high. Same reads, writes, calls and return as retail; only the scheduling of the base store (0x0) and the header stores differs, which does not change behaviour. Parameter types are unsigned int where retail uses pointers or ints: same registers. */
extern int func_00118C70(void *, unsigned int);

int func_0023D018(void *arg0, unsigned int arg1, unsigned int arg2, int arg3, void *arg4, int arg5) {
    ViBuf *vb = (ViBuf *)arg0;
    int hdr[8];

    vb->base = arg1;
    vb->tbl = arg4;
    vb->ntbl = arg5;
    hdr[1] = 1;
    vb->tags = (unsigned char *)((arg2 & 0x0FFFFFFF) | 0x20000000);
    vb->f18 = arg3 << 11;
    hdr[2] = 1;
    vb->count = arg3;
    vb->sema = func_00118C70(hdr, arg1);
    func_0023D090(vb);
    *(s64 *)vb->pad48 = 0;
    return 1;
}
