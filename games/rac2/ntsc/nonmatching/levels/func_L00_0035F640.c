/* Near match: exact size, but the branch layout at +0x20 differs from retail. */
extern int D_L00_001B19E8 __attribute__((sda));
extern int D_L00_001B19EC __attribute__((sda));
extern int D_L00_001B8F0C __attribute__((sda));
extern char D_00188660_3[];
extern int func_002E5AD0(char *, int, int, int, int);
int func_L00_0035F640(int index, int a1, int value, int a3, int limit)
{
    int r;
    char *p;
    if (index >= D_L00_001B19E8)
        return -1;
    if (index == -1)
        return D_L00_001B19EC;
    r = func_002E5AD0((char *)(D_L00_001B19EC + index * 32), a1, 0, value + 0x40, limit);
    if (r >= 0) {
        p = D_00188660_3 + r * 112;
        *(short *)(p + 126) = index;
        *(int *)(p + 140) = value;
    }
    return r;
}
