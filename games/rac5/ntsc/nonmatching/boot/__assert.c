/* newlib __assert, retail 0x01EC82E8 (48 bytes).
 * Argument registers are a0=file, a1=line, a2=assertion; the varargs are
 * passed in a2,a3,t0, and the format string sits at 0x01F26B90.
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
extern int fiprintf(FILE *fp, const char *fmt, ...);
extern void abort(void);
extern const char __assert_fmt[];

void __assert(const char *file, int line, const char *assertion) {
    fiprintf(_impure_ptr->_stderr, __assert_fmt, assertion, file, line);
    abort();
}
