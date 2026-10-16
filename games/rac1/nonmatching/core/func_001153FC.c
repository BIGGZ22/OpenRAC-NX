/* memset: stores the byte a1 into the n bytes at a0 and returns a0. The
   retail code stores 8 or 32 bytes at a time when the pointer is 16-byte
   aligned; the bytes written are the same. */
void *func_001153FC(void *p, int c, int n) {
    unsigned char *q = p;
    while (n-- > 0) {
        *q++ = (unsigned char)c;
    }
    return p;
}
