extern char D_00188660[];
extern int D_0014B540[];
extern void func_002B6770(int, long);
extern void func_00132498(void *, int, void (*)(int, long), unsigned long);

void func_002B6788(void) {
    char *b = D_00188660;
    int *d = D_0014B540;
    *(unsigned int *)(b + 0x17A0) = 0xFFFFFFFFu;
    func_00132498((void *)(d[0x14AC] + d[0x14A7]), 0, func_002B6770,
                  (unsigned long)(unsigned int)(b + 0x17A0));
}
