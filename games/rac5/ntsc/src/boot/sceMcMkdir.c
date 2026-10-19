/* libmc sceMcMkdir, retail 0x01EC3FC0 (52 bytes + nop padding):
 * opens with mode 0x40; a zero handle stores error 0xB in the mc error pole.
 */
extern int sceMcOpen(int port, int slot, const char *name, int mode);
extern int rac5_mc_error;               /* 0x01EFE2C8 */

int sceMcMkdir(int port, int slot, const char *name) {
    int fd = sceMcOpen(port, slot, name, 0x40);
    if (fd == 0)
        rac5_mc_error = 0xB;
    return fd;
}
