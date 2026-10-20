/* Alternative source form for func_L00_002D23C0.
   Retained from lvl6_12_c.c, entry func_L00_002D23C0_v3; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef int s32;
typedef unsigned char u8;

extern char *D_0018C0B0;
extern void func_00310498(void *);
extern int func_002A4DB0(void *, int, int, void *);
extern char D_001B4400[];

void func_L00_002D23C0(int a0, int a1)
{
    char *o = D_0018C0B0;
    int idx = a0;
    unsigned char t;

    *(unsigned char *)(o + 0x41) = a1 + 1;
    *(unsigned char *)(o + 0x40) = a1;
    t = *(unsigned char *)(*(int *)(*(int *)(o + 0x24) + (idx << 2) + 0x48) + 0x10) - 1;
    if (t < (unsigned char)(a1 + 1))
        *(unsigned char *)(o + 0x41) = t;
    *(int *)(o + 0x44) = 0;
    *(unsigned char *)(o + 0x43) = idx;
    *(unsigned char *)(o + 0x42) = idx;
    func_00310498(o);
    *(int *)(o + 0x58) = func_002A4DB0(o, *(unsigned char *)(o + 0x42),
                                       *(unsigned char *)(o + 0x40), D_001B4400);
    *(int *)(o + 0x5C) = func_002A4DB0(o, *(unsigned char *)(o + 0x43),
                                       *(unsigned char *)(o + 0x41), D_001B4400 + 0x600);
}
