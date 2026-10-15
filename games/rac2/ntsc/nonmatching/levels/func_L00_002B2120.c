/* Best plain-C attempt from worker w08. Last check: 212 bytes vs 220; 0/27. */
extern unsigned char D_00189E20_1[];
extern int ref_L00_002B2120_00(void);
extern void ref_L00_002B2120_01(void);
extern void ref_L00_002B2120_02(int);
extern void ref_L00_002B2120_03(void);
extern void ref_L00_002B2120_04(void);
extern void ref_L00_002B2120_05(void);

void func_L00_002B2120(void)
{
    char *p = (char *)D_00189E20_1;
    int state = *(int *)(p + 0x229C);
    int kind;

    if (state != 21 && state != 22 && state != 31 && ref_L00_002B2120_00() == 0)
        return;
    p[0x22BC] = 1;
    p[0x22B7] = 1;
    if (ref_L00_002B2120_00() != 0) {
        p[0x22BE] = 1;
    } else {
        kind = *(int *)(p + 0x2294);
        if (kind == 49 || state == 3 || kind == 18 || kind == 130)
            p[0x22BE] = 1;
        else
            p[0x22BE] = 0;
    }
    ref_L00_002B2120_01();
    ref_L00_002B2120_02(*(int *)(p + 0x2290));
    ref_L00_002B2120_03();
    ref_L00_002B2120_04();
    ref_L00_002B2120_05();
}
