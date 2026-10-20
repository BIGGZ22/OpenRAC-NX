/* from src/game/002B5278.c: 260 bytes against retail's 280, not matched.
 * The factor branches copy the 16-byte vector at obj+0x10 again. Retail materialises that
 * pointer (addiu $a1,$a0,0x10) and reloads the quad (lq) in each branch; this cc1 folds the
 * offset into lq/sq and reuses the first load. The 0.1f constant is also hoisted into the
 * prologue here. */
typedef int TI128 __attribute__((mode(TI)));
typedef union { TI128 q; float f[4]; } Vec4Q_002B5BC0;
typedef struct { char pad[0x10]; TI128 q; } Src16_002B5BC0;

extern char D_001BAF80[];
extern int func_002761F8(void *a, void *b, int mode, void *self, int flag);
extern int func_00278E78(void);
extern void func_00282F20(char *, char *, float);
extern void func_00282C88(void *);

void func_002B5BC0(char *obj, char *moby, float factor)
{
    Vec4Q_002B5BC0 v;
    Vec4Q_002B5BC0 t;
    char *src;
    char *dst;
    char *data;
    int hit;

    src = obj + 0x10;
    v.q = *(TI128 *)src;
    v.f[2] = v.f[2] + (*(float *)(moby + 0x8) + 0.1f);
    if (factor != 0.0f) {
        dst = (char *)&t;
        *(TI128 *)dst = ((Src16_002B5BC0 *)obj)->q;
        t.f[2] = t.f[2] - factor;
    } else {
        dst = (char *)&t;
        *(TI128 *)dst = ((Src16_002B5BC0 *)obj)->q;
        t.f[2] = t.f[2] - (*(float *)(moby + 0xC) + 0.1f);
    }
    hit = func_002761F8(&v, dst, 0, obj, 0);
    if (hit != 0) {
        data = D_001BAF80;
        *(float *)(moby + 0x90) = *(float *)(data + 0x28);
        *(void **)(moby + 0x88) = *(void **)(data + 0x18);
        *(void **)(moby + 0x8C) = *(void **)(data + 0x1C);
        *(int *)(moby + 0xD8) = func_00278E78();
        func_00282F20(moby + 0x50, data + 0x40, 1.0f);
    } else {
        *(float *)(moby + 0x90) = 0.0f;
        *(void **)(moby + 0x88) = 0;
        *(void **)(moby + 0x8C) = 0;
        *(int *)(moby + 0xD8) = -1;
        func_00282C88(moby + 0x50);
    }
}
