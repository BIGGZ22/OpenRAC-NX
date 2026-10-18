/* read - SN libc read: only the tty fd 0 is accepted, initialised on first use. */

extern int tty_initialised;
extern int sceTtyInit(void);
extern int sceTtyRead(void *buf, int count);

int read(int fd, void *buf, int count) {
    if (fd == 0) {
        if (tty_initialised == 0) {
            if (sceTtyInit() == 0)
                return -1;
            tty_initialised = 1;
        }
        return sceTtyRead(buf, count);
    }
    return -1;
}
