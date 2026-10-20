/* Alternative source form for func_L03_003C85F8.
   Retained from lvl6_12_b.c, entry func_L03_003C85F8_v2; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef float f32;

extern char D_00189E20_2[];
extern int func_L00_002D1F78(int, int *);
extern void func_002A7470(void *, int, int, int);

extern char *D_0018C0B0;
extern void func_00310498(void *);
extern int func_002A4DB0(void *, int, int, void *);
extern char D_001B4400[];

extern s32 func_L22_003A48F8(int);
extern s32 func_312898(s32 a0, s32 a1, void *a2);
extern void func_2EEFE0(char *a0, char *a1, void *a2);
extern void func_31B920(char *a0, s32 a1, s32 a2);

void func_L03_003C85F8(char *a0, s32 a1) {
    char *s1 = *(char **)(a0 + 0x68);
    char buf[0x10];
    s32 mode = a1;

    *(s32 *)(s1 + 0xA0) = func_312898(mode, 2, buf);
    func_2EEFE0(s1 + 0x90, a0 + 0x10, buf);
    func_31B920(a0, 2, -1);
}
