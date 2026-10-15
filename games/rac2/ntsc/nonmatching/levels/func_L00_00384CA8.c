/* Near miss from src/levels/shared/L00_0037E898.c: 152 bytes; 148-byte candidate; 0/27. Callee-saved registers are assigned differently and one instruction is absent. */
extern void *func_0029EFD0(int id);
extern void func_002A7750(void *object, int mode, int value);

void *func_L00_00384CA8(void *arg0, void *arg1, void *arg2, unsigned char arg3)
{
    typedef int Func_L00_00384CA8_TI128 __attribute__((mode(TI)));
    char *object = (char *)func_0029EFD0(0x1335);
    void *result = (void *)0xFF;

    if (object != 0) {
        *(void **)(object + 0xB8) = arg0;
        *(unsigned char *)(object + 0x30) = 0xFF;
        *(unsigned char *)(object + 0xBC) = arg3;
        *(Func_L00_00384CA8_TI128 *)(object + 0x10) = *(Func_L00_00384CA8_TI128 *)arg1;
        *(Func_L00_00384CA8_TI128 *)*(void **)(object + 0x68) = *(Func_L00_00384CA8_TI128 *)arg2;
        func_002A7750(object, 2, -1);
        result = object;
    }
    return result;
}
