/* Near miss from src/levels/shared/L00_0037E898.c: 204 bytes; exact-size candidate; 0/27, first nonrelocation mismatch +0x78 in helper argument/global load ordering. */
extern void func_00115DA8(void *buffer, void *source, int value);
extern void func_L00_002E95C0(int first, int second, int third, void *buffer, int flags);
extern char D_L00_001AEEC0[];
extern char D_L00_001AEED0[];
extern int D_L00_001AEE98 __attribute__((sda));
extern int D_L00_001AEE9C __attribute__((sda));
extern volatile int D_L00_001AEEA0 __attribute__((sda));
extern volatile int D_L00_001AEEA4 __attribute__((sda));
extern int D_L00_001AEEA8 __attribute__((sda));
extern int D_L00_001AEEAC __attribute__((sda));
extern int D_L00_001AEEB0 __attribute__((sda));
extern volatile int D_L00_001AEEB4;
extern int D_L00_001AEEB8 __attribute__((sda));

void func_L00_003852A0(void *object)
{
    char buffer[0x100];
    char *state = *(char **)((char *)object + 0x68);

    func_00115DA8(buffer, D_L00_001AEEC0, *(int *)(state + 0x60));
    func_L00_002E95C0(D_L00_001AEE98, D_L00_001AEE9C, D_L00_001AEEB8, buffer, -1);
    func_00115DA8(buffer, D_L00_001AEED0, *(int *)(state + 0x70) + 1);
    {
        int first = D_L00_001AEEA0;
        int second = D_L00_001AEEA4;
        int third = D_L00_001AEEB4;
        func_L00_002E95C0(first, second, third, buffer, -1);
    }
    func_00115DA8(buffer, D_L00_001AEEC0, *(int *)(state + 0x64));
    func_L00_002E95C0(D_L00_001AEEA8, D_L00_001AEEAC, D_L00_001AEEB0, buffer, -1);
}
