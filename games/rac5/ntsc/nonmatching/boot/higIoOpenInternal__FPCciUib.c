/* higIoOpenInternal — RAC2-era high-level I/O open wrapper, adapted from the
   RAC5 retail shape at 0x01E654F8: expand a UMD path into a 0x100-byte
   buffer, retry while the UMD state handler asks to, then record the fd in
   the 0x80-entry open table. */

#define HIG_IO_TABLE 0x01F6F8B0
#define HIG_IO_COUNT 0x01EDC80C

extern int UMD_ExpandFilename(char *dst, unsigned int size, const char *src, int flag);
extern int UMD_checkState(int);
extern int ps2fioOpen(const char *path, int mode, unsigned int flags);

extern unsigned int D_HIG_IO_COUNT;
extern int D_HIG_IO_TABLE[];

int higIoOpenInternal__FPCciUib(const char *path, int mode, unsigned int flags, int flag) {
    char buf[0x100];
    int fd;

    UMD_ExpandFilename(buf, 0x100, path, flag);
    UMD_checkState(1);
    do {
        fd = ps2fioOpen(buf, mode, flags & 0xFFFF);
        if (fd >= 0)
            break;
    } while (UMD_checkState(1) != 0);

    if (fd >= 0 && D_HIG_IO_COUNT < 0x80u) {
        D_HIG_IO_TABLE[D_HIG_IO_COUNT] = fd;
        D_HIG_IO_COUNT++;
    }
    return fd;
}
