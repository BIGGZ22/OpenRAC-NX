/* Near miss from src/levels/shared/L00_0037E898.c: 256 bytes; 268-byte candidate; 0/27. Candidate models the counted interpolation loop and packed result. */
extern int func_002A77E0(int mode);
extern volatile int D_L00_001AED68 __attribute__((sda));
extern float D_L00_001AED70 __attribute__((sda));
extern int D_L00_001AED6C __attribute__((sda));
extern float D_L00_001AEE74 __attribute__((sda));
extern int D_L00_001AEE94 __attribute__((sda));
extern int D_L00_001AEE90 __attribute__((sda));
extern void func_002A7A00(void *target, float first, float second);
extern int func_002A7820(int first, int second);
extern int func_002A77E0(int value);
extern void func_002C3150(void *first, void *second, int third, int fourth, int fifth, int sixth);

extern void func_00282CC0(void *out, void *first, void *second);

void func_L00_003851A0(void *unused, void *source)
{
    char buffer[0x20];
    float scale = 0.01666666753590106964111328125f;
    int i = 0;
    char *second = buffer + 0x10;

    if (D_L00_001AED68 <= 0)
        return;
    while (i < D_L00_001AED68) {
        func_002A7A00(buffer, 0.0f, 0.0333000011742115f);
        i++;
        func_00282CC0(buffer, buffer, source);
        func_002A7A00(second, D_L00_001AED70 * scale, D_L00_001AEE74 * scale);
        {
            int first = func_002A7820(D_L00_001AEE94, D_L00_001AEE90);
            int value = func_002A77E0(0xFF);
            func_002C3150(buffer, second, D_L00_001AED6C, first, value, 0);
        }
    }
}
