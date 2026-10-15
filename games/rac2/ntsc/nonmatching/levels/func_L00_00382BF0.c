/* Near miss from src/levels/shared/L00_0037E898.c: 248 bytes; exact-size candidate; 0/3, first nonrelocation mismatch +0x3C (float spill/argument order); GP global has mixed access forms. */
extern char D_L00_001B8DC0[];
extern int D_001A7350;
extern int D_001A7344;
extern float D_L00_001B2738 __attribute__((sda));
extern void func_00282CF0(void *out, void *first, void *second);
extern float func_00282DB0(void *out, void *source);
extern void func_L00_00323F70(void *object, int *first, int *second);
extern int func_L00_002EA668(int first, int second, int third, int fourth, int fifth, int sixth, float *value);

float func_L00_00382BF0(void *object)
{
    char scratch[0x10];
    int first;
    int second;
    float value;
    char *state = *(char **)((char *)object + 0x68);

    func_00282CF0(scratch, state, D_L00_001B8DC0);
    value = func_00282DB0(scratch, D_L00_001B8DC0 + 0x230);
    func_L00_00323F70(state, &first, &second);
    if (first >= 0 && first < D_001A7350 - 0x10 && second >= 0 && second < D_001A7344 - 8 && value > 0.0f) {
        if (func_L00_002EA668(-1, first, second, 0x10, 8, 0, &D_L00_001B2738) == -1)
            D_L00_001B2738 = -1.0f;
    }
    return D_L00_001B2738;
}
