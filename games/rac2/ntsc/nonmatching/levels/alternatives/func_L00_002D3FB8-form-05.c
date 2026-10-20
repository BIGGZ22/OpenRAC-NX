/* Alternative source form for func_L00_002D3FB8.
   Retained from v2d3fb8.c, entry v5; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef struct {
    unsigned char pad0[142];
    unsigned short f8E;
    unsigned char rest[80];
} Entry;

extern int func_2D3FB0(int);
extern unsigned char counts[];
extern Entry table[];
extern int slots[];

int func_L00_002D3FB8(int a0, int a1)
{
    int idx = func_2D3FB0(a0);
    int *slot = &slots[idx];
    int v = *slot + a1;
    Entry *e;
    int ceiling;

    *slot = v;
    e = &table[counts[idx]];
    ceiling = e->f8E;
    if (ceiling != 0 && v - ceiling > 0) {
        *slot = ceiling;
        return v - ceiling;
    }
    return 0;
}
