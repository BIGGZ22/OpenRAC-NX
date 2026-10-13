extern char D_L00_001FEC40[];
extern void func_L00_0030D788(void);

/* Guarded slot update. */
int func_L00_0034C090(void)
{
    int flags = D_00138344;
    int r = 0;

    if (flags & 0x10) {
        char *b = D_L00_001FEC40;
        int x = *(int *)(*(int *)(b + 0x14) + 0xE0);
        int s;

        if (x != 0) {
            *(int *)(b + 0x18) = x;
            s = 0;
        } else if (*(int *)(b + 0x134) == 0) {
            s = -1;
        }
        return s;
    }
    if (flags & 0x900) {
        r = 1;
    } else {
        func_L00_0030D788();
    }
    return r;
}
