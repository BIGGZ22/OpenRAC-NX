/* 60-byte near miss: instruction sequence matches, but both address %hi relocations differ. */
typedef unsigned int u32;
extern void *D_L00_001B1E00[16];
extern u32 D_L00_001B1E40[16];

void func_0029FAE0(void)
{
    u32 *b = D_L00_001B1E40;
    u32 *a = (u32 *)D_L00_001B1E00;
    int count = 15;
    do {
        *a++ = 0;
        *b++ = 0;
    } while (--count >= 0);
}
