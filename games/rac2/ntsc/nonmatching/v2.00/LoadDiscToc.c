extern char g_discTocBuffer[];
extern void func_00133A28(int a, int b, char *c);

void LoadDiscToc(void)
{
    func_00133A28(0x3E9, 0xB, g_discTocBuffer);
}
