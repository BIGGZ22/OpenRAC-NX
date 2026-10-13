/* 152 of 160 bytes: the found-slot loop and the call sequence match in shape; retail keeps arg in s1 and the slot value in s0 across the calls, ours assigns the registers the other way round (and the else path ends with the value stored back) */
extern int func_L00_00403C30(char *arg);
extern int func_002A77E0(int n);
extern void func_003320F0(int v);
extern void func_003323F0(int v);
extern int *D_L00_001B2C30[] __attribute__((nosda));

void func_L00_00403CB8(char *arg)
{
    if (func_L00_00403C30(arg) == 0) {
        int i;
        int **p = D_L00_001B2C30;
        for (i = 0; i < 16; i++) {
            if (*p == 0) {
                *p = (int *)arg;
                return;
            }
            p++;
        }
    } else {
        int **slot = &D_L00_001B2C30[func_002A77E0(16)];
        int *v = *slot;
        func_003320F0((int)v);
        func_003323F0((int)v);
        *slot = v;
    }
}
