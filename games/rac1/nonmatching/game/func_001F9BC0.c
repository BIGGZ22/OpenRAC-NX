/* Clears the 16 bytes (one quadword, `sq $0`) at p. */
void func_001F9BC0(void *p) {
    unsigned int *q = (unsigned int *)p;
    q[0] = 0;
    q[1] = 0;
    q[2] = 0;
    q[3] = 0;
}
