/* Candidate only: the full-program build and compiler profile are not qualified. */
extern int ps2fioRead(int handle, void *buffer, unsigned int size);

int higIoRead__FiPvUi(int handle, void *buffer, unsigned int size) {
    return ps2fioRead(handle, buffer, size);
}
