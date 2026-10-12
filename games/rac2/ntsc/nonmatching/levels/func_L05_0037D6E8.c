/* func_L05_0037D6E8 (128 o) - best attempt, not a match.
   near miss: 124/128 with the struct-pointer form, 128/128-size with the form below. Retail bases the walk on &elem.f14 (lw -0x8(v1) / sw 0(v1)) and keeps %hi of D_L05_002A1800 in s0 for the later element accesses; this source materialises the full address early so cc1 emits daddu s0,v0 instead of lui s0.
   Verified with: python3 tools/check.py func_L05_0037D6E8 --diff
*/

void func_L05_0037D6E8(void)
{
    int *p = &D_L05_002A1800[0].f14;
    int i;

    for (i = 14; i >= 0; i--) {
        *p = *(p - 2);
        p += 10;
    }
    D_L05_002A1800[13].f14 = D_L05_002A1800[13].f0C;
    D_L05_002A1800[7].f14 = D_L05_002A1800[7].f0C;
    i = func_002ED8A0(6);
    D_L05_002A1800[i].f14 = D_L05_002A1800[i].f10;
}
