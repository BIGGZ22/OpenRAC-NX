/* Alternative source form for func_L22_003F85A0.
   Retained from v3f85a0.c, entry b1; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
typedef float f32;
typedef int V4 __attribute__((mode(TI)));

extern u8 *func_315688(int);
extern void func_3180D8(u8 *);
extern f32 g_f;

u8 *func_L22_003F85A0(int a0, u8 *a1)
{
    u8 *o = func_315688(3413);
    if (o != 0) {
        u8 *p;
        *(u8 *)(o + 49) = 1;
        *(u8 *)(o + 48) = 0xFF;
        *(unsigned short *)(o + 50) = 0xFF;
        *(u8 *)(o + 32) = 0;
        *(f32 *)(o + 44) = *(f32 *)(*(int *)(o + 36) + 36) * g_f;
        p = *(u8 **)(o + 104);
        *(int *)(p + 16) = a0;
        *(V4 *)p = *(V4 *)a1;
        *(V4 *)(o + 16) = *(V4 *)a1;
        *(int *)(p + 20) = 0;
        func_3180D8(o);
    }
    return o;
}
