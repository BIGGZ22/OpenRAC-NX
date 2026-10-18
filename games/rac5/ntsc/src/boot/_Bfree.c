/* newlib _Bfree, retail 0x01ECF860 (52 bytes + nop padding):
 *   if (v) { v->_next = ptr->_freelist[v->_k]; ptr->_freelist[v->_k] = v; }
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

struct _Bigint {
    struct _Bigint *_next;       /* 0x00 */
    int _k;                      /* 0x04 */
    int _maxwds;                 /* 0x08 */
    int _sign;                   /* 0x0C */
    int _wds;                    /* 0x10 */
    unsigned int _x[1];          /* 0x14 */
};

void _Bfree(struct _reent *ptr, struct _Bigint *v) {
    if (v) {
        v->_next = (struct _Bigint *)ptr->_freelist[v->_k];
        ptr->_freelist[v->_k] = v;
    }
}
