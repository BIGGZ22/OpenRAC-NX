/* 72 bytes vs retail 64 (18 instructions vs 16): the loop body matches retail instruction for instruction; the difference is the first load. Retail reads the count straight off $gp (lw a0,0xD8C($gp)); cc1 emits lui+lw of the symbol address. Same ordinary-flow $gp boundary as func_L00_003B8138 (docs/COMPILER-NOTES.md, -G8 .extern route). */
typedef struct OBJ3 {
    char pad0[0x98];
    int f98;
} OBJ3;

extern int D_L00_001AFD7C __attribute__((sda));
extern OBJ3 *D_L00_0023DBA0[];
extern int D_L00_0023DCA0[];

void func_L00_003B9290(void)
{
    int n = D_L00_001AFD7C;
    int i;

    for (i = 0; i < n; i++) {
        OBJ3 *p = D_L00_0023DBA0[i];
        D_L00_0023DCA0[i] = p->f98;
        p->f98 = 0;
    }
}
