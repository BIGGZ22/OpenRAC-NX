/* newlib __sclose, retail 0x01EC7640 (32 bytes):
 *   lh $a1,0xE($a0) ; jal _close_r ; lw $a0,0x54($a0)  (delay) ; epilogue
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

extern int _close_r(struct _reent *ptr, int fd);

int __sclose(FILE *fp) {
    return _close_r((struct _reent *)fp->_data, fp->_file);
}
