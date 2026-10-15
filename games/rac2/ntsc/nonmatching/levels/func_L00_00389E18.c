/* Near miss from src/levels/shared/L00_0037E898-g8.c: 148 bytes; exact-size candidate; 0/27, first nonrelocation mismatch +0xC (s1/ra save order). */
extern void func_00282CC0(void *out, void *target, void *source);
extern void func_002FE880(void *target, void *source, float value);
extern int func_002761F8(void *out, void *target, int size, void *owner, int mode);
typedef int Func_L00_00387370_TI128 __attribute__((mode(TI)));
extern float D_L00_001AEF90 __attribute__((sda));

void func_L00_00389E18(void *owner)
{
    char *target = (char *)owner + 0x10;
    char *source = *(char **)((char *)owner + 0x68);
    Func_L00_00387370_TI128 local = *(volatile Func_L00_00387370_TI128 *)target;

    func_00282CC0(target, target, source);
    func_002FE880(target, source, D_L00_001AEF90 * 2.7777778086601757e-05f);
    if (func_002761F8(&local, target, 0x10, owner, 0) != 0)
        *((unsigned char *)owner + 0x20) = 8;
}
