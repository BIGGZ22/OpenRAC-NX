/* Near miss from src/levels/shared/L01_00397110.c: the build refuses this body at its
   place — 244 bytes, expected 248. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L01_00397110.c. */
extern char D_00189EA0[];

extern void func_2E34A0(void *, char *, char *);

extern void func_2E3918(void *, void *, char *);

extern float func_2E33E0(float);

extern void func_323310(int, int);

void func_L01_0042E0A8(char *a0)
{
    float v[4];
    float w[4];
    char *s1 = *(char **)(a0 + 8);

    func_2E34A0(v, D_00189EA0, a0 + 0x40);
    v[3] = 0.0f;
    func_2E3918(w, v, a0 + 0x50);

    if (func_2E33E0(w[0]) <= 1.0f && func_2E33E0(w[1]) <= 1.0f &&
        func_2E33E0(w[2]) <= 1.0f)
        *(int *)(s1 + 8) = 1;

    if (*(int *)(s1 + 8) == 0)
        return;

    if (w[0] < 0.0f)
        func_323310(*(int *)(s1 + 4), *(int *)(s1 + 0x10));
    else
        func_323310(*(int *)(s1 + 0), *(int *)(s1 + 0xC));

    *(int *)(s1 + 8) = 0;
}
