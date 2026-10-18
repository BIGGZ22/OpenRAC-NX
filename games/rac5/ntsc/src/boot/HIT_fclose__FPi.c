extern void higIoClose__Fi(int handle);

void HIT_fclose__FPi(int handle) {
    higIoClose__Fi(handle - 1);
}
