/* From src/levels/shared/L00_003D37E8.c: links at 2 of 27 places (instruction at +0x4 differs). */
/* shared native body of family beb4059b71338403 */
void func_L00_003DC350(void)
{
    int *p;
    int i;

    p = *(int **)0x1B32A0 + 60;
    for (i = 3; i >= 0; i--) { *p = 0; p -= 16; }

    p = *(int **)0x1B32A4 + 4799;
    for (i = 149; i >= 0; i--) { *p = 0; p -= 32; }

    p = *(int **)0x1B32A8;
    for (i = 9; i >= 0; i--) { p[1] = 0; p[0] = 0; p += 4; }
}
