/* 56 bytes: under a -G8 probe only the first two instructions differ (retail does daddu a0,zero,zero before the lui of the base into v1; the compiler emits lui v0 first and copies it); the loop matches */
extern int D_L00_001B2C30[] __attribute__((nosda));

int func_L00_00403C30(char *arg)
{
    int i;
    for (i = 0; i < 16; i++) {
        if (D_L00_001B2C30[i] == 0) {
            return 0;
        }
    }
    return 1;
}
