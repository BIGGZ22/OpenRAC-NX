typedef unsigned char u8;
extern int func_00351CD8(void *, int *);
extern int func_00351D00(void);
extern int func_00351D38(void);
extern int func_00351D68(void);
extern int func_00351D98(void *, void *);
extern void func_00351888(u8 *);
extern void func_0012F738(void *);
extern void func_0012FA70(void *, int, void *, int);
extern void func_003508A0(void *, int, int, int, int, int);

int func_00351720(char *object, int unused1, int unused2, int a, int b, int c, int d, int e)
{
    func_0012F738(object);
    func_0012FA70(object, 0, func_00351CD8, 0);
    func_0012FA70(object, 1, func_00351D00, 0);
    func_0012FA70(object, 2, func_00351D38, 0);
    func_0012FA70(object, 3, func_00351D68, 0);
    func_0012FA70(object, 5, func_00351D98, 0);
    func_00351888((u8 *)object);
    func_003508A0(object + 0x48, a, b, c, d, e);
    return 1;
}
