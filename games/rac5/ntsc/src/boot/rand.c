/* newlib rand, retail 0x01EC9258 (48 bytes):
 *   _impure_ptr->_rand_next = _rand_next * 1103515245 + 12345; return & 0x7fffffff
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

extern struct _reent *_impure_ptr;

int rand(void) {
    return (_impure_ptr->_rand_next =
                _impure_ptr->_rand_next * 1103515245u + 12345u) & 0x7fffffffu;
}
