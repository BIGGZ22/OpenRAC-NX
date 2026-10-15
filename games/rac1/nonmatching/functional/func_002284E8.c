/* functional: high. The pause screen's strip of sprites: a GIF packet of x/32 strip vertex pairs from the screen size at D_00151880 (+0x150, +0x152) appended at the packet cursor D_00161000. Not a match (420 of 424 bytes: retail counts its loop down); read against the instructions, equiv.py differs only in that counter. */
extern short D_00151880[];

void func_002284E8(void) {
    int x = D_00151880[0xA8];
    int y = D_00151880[0xA9];
    int n = x / 32;
    long *q;
    long *r;
    long *s;
    int i;

    func_00234C98(0x42, 0x64);
    D_00161000[0] = 0x10000000 | (n + 5);
    D_00161000[1] = 0;
    D_00161000[2] = 0;
    D_00161000[3] = (n + 5) | 0x50000000;
    q = (long *)D_00161000;
    r = q + 2;
    D_00161000 = (int *)r;
    q[2] = 0x1000000000000001L;
    r[9] = 0x44;
    r[1] = 0xE;
    r[2] = 0x31001;
    r[3] = 0x47;
    r[4] = 0x2400000000008001L;
    r[5] = 0x10;
    r[6] = 0x146;
    r[7] = 0x7F808080;
    r[8] = (n | 0x8000) | 0x2400000000000000L;
    s = q + 12;
    for (i = 0; i < n; i++) {
        *s++ = ((long)(0x8000 - y * 8) << 16) | (long)(0x8000 - x * 8 + i * 0x200);
        *s++ = ((long)(y * 8 + 0x7FF0) << 16) | (long)(0x8200 - x * 8 + i * 0x200);
    }
    D_00161000 = (int *)((char *)D_00161000 + n * 16 + 0x50);
}
