/* functional: high. Handles one reply record of a 0x440-byte slot table (D_00157600, slot from arg or 0): copies the 16-byte header, optionally memcpys data, runs per-command copy-outs (2, 11, 12, 23/25/26) and releases the request id (negative id: clears it in the D_0012FD10 table, else calls func_00118CA0). Corrected in place: copy-out case labels were 22/24/25, the jump table jtbl_001529E0 gives 23/25/26. */
extern s32 D_0012FD98_B868 __asm__("D_0012FD98");
extern s32 D_0012FD90_B868 __asm__("D_0012FD90");
extern volatile s32 D_0012FD10_B868[] __asm__("D_0012FD10");
extern u8 D_00157600_B868[] __asm__("D_00157600");
extern void *func_00115248_B868(void *, const void *, u32) __asm__("func_00115248");
extern s32 func_00118CA0_B868(s32) __asm__("func_00118CA0");

void func_0011B868(u8 *arg) {
    s32 hdr[4];
    s32 slot;
    u8 *rec;
    s32 *r;
    s32 i;
    s32 id;
    s32 count;
    s32 len;
    u8 *dst;
    u8 *src;

    slot = 0;
    if (D_0012FD98_B868 != 0)
        slot = *(s32 *)(arg + 0xC);
    D_0012FD90_B868 = slot;
    rec = (u8 *)(((u32)D_00157600_B868 + slot * 0x440) | 0x20000000);
    r = (s32 *)rec;
    hdr[0] = r[0];
    hdr[1] = r[1];
    hdr[2] = r[2];
    hdr[3] = r[3];
    if (hdr[0] >= 0)
        func_00115248_B868((void *)hdr[2], rec + 0x10, hdr[3]);

    switch (hdr[1]) {
    case 2:
        if (*(s32 *)(rec + 0x14) > 0) {
            dst = (u8 *)*(s32 *)(rec + 0x1C);
            src = rec + 0x24;
            i = 0;
            do {
                dst[i] = src[i];
                i++;
            } while (i < *(s32 *)(rec + 0x14));
        }
        if (*(s32 *)(rec + 0x18) > 0) {
            dst = (u8 *)*(s32 *)(rec + 0x20);
            src = rec + 0x64;
            i = 0;
            do {
                dst[i] = src[i];
                i++;
            } while (i < *(s32 *)(rec + 0x18));
        }
        break;
    case 11:
        dst = (u8 *)*(s32 *)(rec + 0x14);
        src = rec + 0x18;
        for (i = 0; i < 0x144; i++)
            dst[i] = src[i];
        break;
    case 12:
        dst = (u8 *)*(s32 *)(rec + 0x14);
        src = rec + 0x18;
        for (i = 0; i < 0x40; i++)
            dst[i] = src[i];
        break;
    case 23:
    case 25:
    case 26:
        dst = (u8 *)*(s32 *)(rec + 0x14);
        len = *(s32 *)(rec + 0x18);
        if ((u32)len > 0x400)
            len = 0x400;
        func_00115248_B868(dst, rec + 0x1C, len);
        break;
    default:
        break;
    }

    id = hdr[0];
    if (id >= 0) {
        func_00118CA0_B868(id);
        return;
    }
    id = -id;
    if (D_0012FD10_B868[0] == id) {
        D_0012FD10_B868[0] = -1;
        return;
    }
    for (i = 1; i < 0x20; i++) {
        if (D_0012FD10_B868[i] == id) {
            D_0012FD10_B868[i] = -1;
            return;
        }
    }
}
