/* Near miss: the global lui and stack adjustment are ordered oppositely in retail. */
int func_002D33C0(void)
{
    extern char *D_001A8CFC;
    extern char D_002617E0[];
    extern void func_0033E368(void *, void *);
    if (D_001A8CFC != 0)
        func_0033E368(D_001A8CFC + 0x3EDB8, (void *)D_002617E0);
    return 0;
}
