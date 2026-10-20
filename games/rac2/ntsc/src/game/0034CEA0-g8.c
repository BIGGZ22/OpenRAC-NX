extern char D_L00_001BD2C0[];
extern int D_L00_001A8FB4;
extern char *D_L00_001A904C;
extern int D_L00_001AEA7C __attribute__((sda));
extern int D_L00_001AEA80 __attribute__((sda));
extern void func_0027AB40(void);
extern unsigned int func_0034CA68(void *);
extern void func_0034E2E0(void *, int, int, int);
extern void func_0034E218(void *);
extern void func_0034DB98(void *);
extern void func_00338F58(void *);
extern void func_0029CB80(void);
void func_0034E4F8(char *object)
{
    float saved;
    char *camera;
    char *state = D_L00_001BD2C0;
    saved = *(float *)(state + 0xB0);
    *(float *)(state + 0xB0) = 0.62f;
    func_0027AB40();
    if (D_L00_001A8FB4 == 0)
        func_0034E2E0(object, D_L00_001AEA7C, D_L00_001AEA80, func_0034CA68(object + 0x7A0) >> 24);
    func_0034E218(object);
    func_0034DB98(object + 0x7A0);
    camera = D_L00_001A904C + 0x38000;
    *(int *)(camera + 0x7354) = 0;
    *(int *)(camera + 0x7358) = 0;
    func_00338F58(object + 0x1FBC);
    func_0029CB80();
    *(float *)(state + 0xB0) = saved;
    func_0027AB40();
}

extern char *D_001B23CC[] __attribute__((sda));
extern int ref_L00_0034F690_00 __attribute__((sda));
typedef int (*CleanupCallback)(void);
extern int func_001339B0(void);
extern void func_00124B88(int);
extern void func_003505A8(void *);
extern void func_00351E40(void *);
extern void func_0011AA70(int);
extern void func_0011AA30(int);
extern void func_0011B658(int);
extern void func_0011A950(int, int);
extern CleanupCallback func_00126DC0(CleanupCallback);
extern int func_00351890(void *);
extern int func_0034F898(void *);
extern int func_003506A8(void *);

void func_0034F690(void)
{
    func_00124B88(0);
    func_003505A8(D_001B23CC[0]);
    func_00351E40(D_001B23CC[0] + 0xd9168);
    func_0011AA70(ref_L00_0034F690_00);
    func_0011AA30(ref_L00_0034F690_00);
    func_0011B658(2);
    func_0011A950(2, *(int *)(D_001B23CC[0] + 0xd8000 + 0x10f8));
    func_00126DC0(func_001339B0);
    func_00351890(D_001B23CC[0] + 0xd9048);
    func_0034F898(D_001B23CC[0] + 0xd9100);
    func_003506A8(D_001B23CC[0] + 0xd9040);
    func_00124B88(0);
    *(volatile unsigned int *)0x1000e000 &= ~2u;
}

extern char *ref_L00_0034EF58_01[] __attribute__((sda));
extern char ref_L00_0034EF58_00[];
extern void func_0026FEB8(void *, ...);
extern void func_00284088(int, int);
extern int func_0011AB10(void);
extern void func_0011AAB0(int, int);
extern int func_0034F408(void *, void *, int);
extern int func_0034F048(void *, void *, void *);

int func_0034EF58(void *first, void *second, char *pool, char *state, int mode, int volume)
{
    int result = 0;
    ref_L00_0034EF58_01[0] = pool;
    D_001B23CC[0] = state;
    func_0026FEB8(ref_L00_0034EF58_00, 0x38cac0);
    func_00284088(volume, volume);
    func_0011AAB0(func_0011AB10(), 1);
    if (func_0034F408(first, second, mode)) {
        result = func_0034F048(D_001B23CC[0] + 0xd9048, D_001B23CC[0], D_001B23CC[0] + 0xd9040);
    }
    func_0034F690();
    ref_L00_0034EF58_01[0] = 0;
    D_001B23CC[0] = 0;
    return result;
}
