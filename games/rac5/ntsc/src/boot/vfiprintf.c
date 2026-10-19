/* newlib vfiprintf, retail 0x01ECC168 (44 bytes + padding):
 *   a0 = fp->_data, a1 = fp, a2 = fmt, a3 = args ; jal _vfiprintf_r
 */
struct _reent;
typedef struct _FILE {
    unsigned char _pad0[0x0E];
    short _file;                 /* 0x0E */
    unsigned char _pad1[0x44];   /* 0x10 .. 0x53 */
    struct _reent *_data;        /* 0x54 */
} FILE;
struct _reent {
    int _errno;                  /* 0x00 */
    void *_stdin;                /* 0x04 */
    void *_stdout;               /* 0x08 */
    FILE *_stderr;               /* 0x0C */
    int _pad0[15];               /* 0x10 .. 0x4B */
    void **_freelist;            /* 0x4C */
    int _pad1[2];                /* 0x50, 0x54 */
    unsigned int _rand_next;     /* 0x58 */
};

extern int _vfiprintf_r(void *ptr, FILE *fp, const char *fmt, void *args);

int vfiprintf(FILE *fp, const char *fmt, void *args) {
    return _vfiprintf_r(fp->_data, fp, fmt, args);
}
