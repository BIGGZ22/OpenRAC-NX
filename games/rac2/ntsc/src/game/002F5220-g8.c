
extern char D_001BAD40[];
extern int D_001A7478 __attribute__((sda));
extern short D_00152C28[];
extern int D_0013955C[];
extern void func_00288AA0(char *, int);
extern void func_00288BE8(char *);
extern void func_002F5CB8(void (*)(void), int);
extern void func_002F6458(void);

void func_002F64A8(void)
{
    char *buffer = D_001BAD40;
    D_001A7478 = 1;
    func_00288AA0(buffer, 21);
    func_00288BE8(buffer);
    func_002F5CB8(func_002F6458, 0);
    D_00152C28[0] = 1;
    D_0013955C[0] = 0;
}
