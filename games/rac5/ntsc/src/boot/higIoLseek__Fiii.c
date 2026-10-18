extern long ps2fioLseek(int handle, long offset, int whence);

int higIoLseek__Fiii(int handle, int offset, int whence) {
    return ps2fioLseek(handle, offset, whence);
}
