/* 0 of 27 exact: about 10 of 26 instructions differ. Loop shape and calls match (func_L00_003E6F48, the back-edge reload of D_L00_001B2B50 via sda); we keep the base load before the saves and swap the counter/offset registers (retail: counter s0, offset s1, base loaded after s2=a0). */
extern char *D_L00_001B2B50;
extern char *D_L00_001B2B50_1 __attribute__((sda));
extern void func_L00_003E6F48(char *rec);

void func_L00_003E6710(char *object)
{
    int off = 0;
    int i = 0;
    char *base;

    base = D_L00_001B2B50;
    do {
        char *rec;
        off--;
        rec = base + i;
        if (*(char **)(rec + 0x30) == object)
            func_L00_003E6F48(rec);
        i += 0x50;
        base = D_L00_001B2B50_1;
    } while (off >= 0);
}
