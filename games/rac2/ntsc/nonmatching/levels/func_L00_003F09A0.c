/* func_L00_003F09A0: 124 bytes, 1st differing instruction at +0x2c: retail schedules the counter increment before the $a3 constant and puts the element address in $v1 (ours in $v0); the loop, the call to func_L00_003F0B10 and the gp reload match otherwise. Missing: the statement form that makes the compiler pick $v1 for the element. */
extern char *D_L00_001B2BB8 __attribute__((sda));

extern void func_L00_003F0B10(char *e, float f12, int a1, int a2, int a3, float f13, float f14, int t0);

void func_L00_003F09A0(char *a0)
{
    int i = 0;

    do {
        char *e = D_L00_001B2BB8 + ((i++) << 7);

        if (*(char **)(e + 0x7C) == a0) {
            func_L00_003F0B10(e, *(float *)(e + 0x48), *(int *)(e + 0x68), *(int *)(e + 0x60), 46,
                              *(float *)(e + 0x34), 0.25f, *(int *)(e + 0x70));
        }
    } while (i < 320);
}
