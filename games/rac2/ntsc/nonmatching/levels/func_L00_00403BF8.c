/* 52 bytes: under a -G8 probe only the first two instructions differ (retail does li v1,15 before the lui of the base address into v1; the compiler emits lui v0 first); the loop and store match */
extern int D_L00_001B2C6C[] __attribute__((nosda));

void func_L00_00403BF8(char *arg)
{
    int *p = D_L00_001B2C6C;
    int i;
    for (i = 15; i >= 0; i--) {
        *p = 0;
        p--;
    }
}
