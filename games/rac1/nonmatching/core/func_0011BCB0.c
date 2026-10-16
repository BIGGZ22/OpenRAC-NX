/* CD/DVD init step: install the two RPC-side handlers, bind the RPC server
 * at 0x80000001 (retrying until it has an address), clear the 32-slot table
 * D_00157E80, then send command 0xFF and copy back the reply word and the
 * flag that says whether the reply was 2. Returns 0 on success, -1 when the
 * bind failed, 0xFFFEFFFF when the RPC failed. */
extern char D_00158100[];
extern char D_001580C0[];
extern char D_00158080[];
extern char D_001575C0[];
extern char D_00157600[];
extern char *D_00156940[];
extern char D_001580A8[];
extern int D_0012FD94;
extern int D_0012FD98;
extern void func_0011B868();
extern void func_0011BC80();

s32 func_0011BCB0(void) {
    s32 r;
    s32 t;
    s32 i;
    char *p;

    func_0011AE20(0);
    *(s32 *)D_00158100 = 0;
    *(s32 *)(D_00158100 + 4) = 0;
    r = func_0011D960();
    func_0011AA38(0x80000011, (int)func_0011B868, (int)D_001580C0);
    func_0011AA38(0x80000013, (int)func_0011BC80, (int)D_00158100);
    if (r != 0) {
        func_0011D9A8();
    }
    for (;;) {
        r = func_0011B2F8(D_00158080, 0x80000001, 0);
        if (r < 0) {
            return -1;
        }
        if (*(s32 *)(D_00158080 + 0x24) != 0) {
            break;
        }
        for (i = 0x100000; i != -1; i--) {
        }
    }
    func_0011B710();
    func_00118CB0(D_0012FDA0);
    for (p = D_00157E80; p < D_00157E80 + 0x200; p += 0x10) {
        *(s32 *)(p + 4) = 0;
    }
    func_00118C90(D_0012FDA0);
    D_00156940[0] = D_00157600;
    D_00156940[1] = D_00157600 + 0x440;
    r = func_0011B4C8(D_00158080, 0xFF, 0, D_00156940, 8, D_001575C0, 8, 0, 0);
    if (r < 0) {
        return (s32)0xFFFEFFFF;
    }
    *(s32 *)D_001580A8 = *(s32 *)((u32)D_001575C0 | 0x20000000);
    t = *(s32 *)((u32)(D_001575C0 + 4) | 0x20000000);
    D_0012FD94 = 1;
    D_0012FD98 = (t == 2);
    return 0;
}
