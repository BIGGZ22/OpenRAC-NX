extern int asyncPs2fioRead(int handle, void *buffer, unsigned int size);

int higIoReadAsync__FiPvUi(int handle, void *buffer, unsigned int size) {
    return asyncPs2fioRead(handle, buffer, size);
}
