/* functional: medium. Command-packet interrupt handler: runs func_0011D9A8 first (EI, as retail), returns 0 if the first byte of the buffer D_00154F58.a0 is 0; otherwise copies the packet ((size + 15) >> 4 quadwords) to a local buffer, clears the length byte, calls func_00118E50 (an acknowledge syscall, a0 = 0 as left by retail's copy loop), then dispatches on the word at +8: a negative value indexes table a3 (count a4) with the sign bit cleared, otherwise table a5 (count a6); a non-null entry f is called as f(packet, arg) where arg is the entry's second word. Ends with EI and returns 0. Return type is int (retail returns 0 in v0); other files declare it void. The syscall meaning is inferred. */
typedef struct {
    unsigned char *a0;
    void *a1;
    int a2;
    unsigned char *a3;
    int a4;
    unsigned char *a5;
    int a6;
    void *a7;
} CmdData_11AC48;

typedef struct {
    void (*f)(void *, void *);
    void *arg;
} CmdEnt_11AC48;

extern CmdData_11AC48 D_00154F58;
extern int func_0011D9A8(void);
extern void func_00118E50(int a0);

int func_0011AC48(void) {
    unsigned char buf[0x120] __attribute__((aligned(16)));
    unsigned char *src;
    unsigned int size, n, i;
    int cmd;

    func_0011D9A8();
    src = D_00154F58.a0;
    size = src[0];
    if (size == 0) {
        return 0;
    }
    n = (size + 15) >> 4;
    src[0] = 0;
    for (i = 0; i < n * 16; i++) {
        buf[i] = src[i];
    }
    func_00118E50(0);
    cmd = *(int *)(buf + 8);
    if (cmd < 0) {
        int idx = cmd & 0x7FFFFFFF;
        if (idx < D_00154F58.a4) {
            CmdEnt_11AC48 *e = (CmdEnt_11AC48 *)D_00154F58.a3 + idx;
            if (e->f != 0) {
                e->f(buf, e->arg);
            }
        }
    } else {
        if (cmd < D_00154F58.a6) {
            CmdEnt_11AC48 *e = (CmdEnt_11AC48 *)D_00154F58.a5 + cmd;
            if (e->f != 0) {
                e->f(buf, e->arg);
            }
        }
    }
    func_0011D9A8();
    return 0;
}
