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
