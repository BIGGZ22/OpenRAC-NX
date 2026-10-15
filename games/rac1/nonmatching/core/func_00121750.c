/* sceCdRead: queue one CD/DVD read request (lba, sectors, buffer, mode)
 * in the request block at D_001314C0 and start it through the RPC
 * func_0011B4C8. Returns 1 when the RPC was accepted, 0 otherwise.
 * Sector size by mode->datapattern: 1 -> 0x918, 2 -> 0x924, else 0x800. */
extern int D_001313F4 NOT_SDA;
extern char D_001314C0[];
extern char D_001324C0[];
extern int D_00132580 NOT_SDA;
extern char D_001530C8[];
extern char D_001530E0[];
extern volatile int D_00131414;
extern int D_001313F0;
extern void func_0011AD70(void *, int);
extern void func_00118C90(int);
extern void func_00120A78();

int func_00121750(int lba, int sectors, void *buf, unsigned char *mode) {
    char *req = D_001314C0;
    int size;
    int r;

    if ((D_001313F4 & 1) == 0) {
        if (func_00120E98() == 6) {
            return 0;
        }
    }
    if (func_00120D28(4) == 0) {
        return 0;
    }

    *(int *)(req + 0x0) = lba;
    *(int *)(req + 0x4) = sectors;
    *(void **)(req + 0x8) = buf;
    req[0xC] = mode[0];
    req[0xD] = mode[1];
    req[0xE] = mode[2];
    *(void **)(req + 0x10) = D_001324C0;
    *(void **)(req + 0x14) = &D_00132580;

    if (mode[2] == 1) {
        size = sectors * 0x918;
    } else if (mode[2] == 2) {
        size = sectors * 0x924;
    } else {
        size = sectors << 11;
    }

    D_00132580 = 0;
    if ((D_001313F4 & 2) == 0) {
        func_0011AD70(buf, size);
    }
    func_0011AD70(D_001324C0, 0x90);
    func_0011AD70(req, 0x18);
    func_0011AD70(&D_00132580, 4);
    if (D_001313D0 > 0) {
        func_0011A6C8(D_001530C8);
    }

    D_00131414 = 1;
    D_001313F0 = 1;
    r = func_0011B4C8(D_00132590, 1, 1, req, 0x18, 0, 0,
                      (void *)((char *)func_00120A78 + 0x10), D_001324C0);
    if (r >= 0) {
        if (D_001313D0 > 0) {
            func_0011A6C8(D_001530E0);
        }
        return 1;
    }
    D_00131414 = 0;
    D_001313F0 = 0;
    func_00118C90(D_001313E8);
    return 0;
}
