/* write - SN libc write: only the tty fds 1 and 2 are accepted, as for read. */

extern int tty_initialised;
extern int sceTtyInit(void);
extern int sceTtyWrite(const void *buf, int count);

int write(int fd, const void *buf, int count) {
    if ((unsigned int)(fd - 1) < 2) {
        if (tty_initialised == 0) {
            if (sceTtyInit() == 0)
                return -1;
            tty_initialised = 1;
        }
        return sceTtyWrite(buf, count);
    }
    return -1;
}
