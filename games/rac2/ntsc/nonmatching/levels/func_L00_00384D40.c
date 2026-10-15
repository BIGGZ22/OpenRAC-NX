/* Near miss from src/levels/shared/L00_0037E898-g8.c: 196 bytes; 192-byte candidate; 0/27. Object/child register allocation and instruction schedule differ. */
extern void *func_0029EFD0(int id);
extern void func_002AEC18(void *object, int value);
extern void func_002A7750(void *object, int mode, int value);
extern void func_L00_00383B90(void);
extern int D_L00_001AEE0C __attribute__((sda));
extern float D_L00_001AEE08 __attribute__((sda));
typedef int Func_L00_00384D40_TI128 __attribute__((mode(TI)));

void *func_L00_00384D40(void *arg0, Func_L00_00384D40_TI128 *arg1, Func_L00_00384D40_TI128 *arg2)
{
    char *child;
    char *object = (char *)func_0029EFD0(0x5D8);

    if (object == 0)
        return (void *)0xFF;
    func_002AEC18(object, D_L00_001AEE0C);
    child = *(volatile char **)(object + 0x68);
    *(unsigned char *)(object + 0x30) = 0xFF;
    *(void **)(object + 0xB8) = arg0;
    *(void (**)(void))(object + 0x64) = func_L00_00383B90;
    *(unsigned short *)(object + 0x32) = 0xFF;
    *(float *)(object + 0x2C) = *(float *)(*(char **)(object + 0x24) + 0x24) * D_L00_001AEE08;
    *(Func_L00_00384D40_TI128 *)(object + 0x10) = *arg1;
    *(Func_L00_00384D40_TI128 *)child = *arg2;
    func_002A7750(object, 3, -1);
    return object;
}
