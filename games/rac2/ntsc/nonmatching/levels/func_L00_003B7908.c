/* 156 bytes vs retail 148 (39 instructions vs 37): the loop, the calls and the frame match retail instruction for instruction; the difference is that retail takes the addresses of two $gp-resident globals with one addiu each (addiu s1,$gp,0xBA8 and addiu s3,$gp,0xBB8), while cc1 emits lui+addiu for each. Same ordinary-flow $gp boundary as func_L00_003B8138 (docs/COMPILER-NOTES.md, -G8 .extern route). */
typedef struct OBJ6 {
    char pad0[0x68];
    char *f68;
} OBJ6;

typedef struct PAIR6 {
    int a;
    int b;
} PAIR6;

extern PAIR6 D_L00_001AFB98[] __attribute__((sda));
extern int D_L00_001AFBA8[] __attribute__((sda));
extern char D_L00_001AFBB0[];
extern void func_115DA8(void *dst, void *src, int v);
extern void func_2E95C0(int a, int b, int c, void *d, int e);

void func_L00_003B7908(OBJ6 *a0)
{
    char local[0x40];
    int *s0 = (int *)(a0->f68 + 0x58);
    PAIR6 *s1 = D_L00_001AFB98;
    int *s3 = D_L00_001AFBA8;
    int s2 = 1;

    do {
        func_115DA8(local, D_L00_001AFBB0, *s0++);
        s2--;
        func_2E95C0(s1->a, s1->b, *s3++, local, -1);
        s1++;
    } while (s2 >= 0);
}
