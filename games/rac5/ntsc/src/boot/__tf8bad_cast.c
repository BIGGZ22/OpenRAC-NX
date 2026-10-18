/* __tf8bad_cast - GCC 2.9 RTTI accessor: builds __ti8bad_cast lazily through __rtti_si once,
   after the base type_info accessor has run, then hands it back. */

typedef struct {
    void *name;      /* +0x0 */
    void *vtable;    /* +0x4 */
    void *base;      /* +0x8 */
} ti_t;

extern void __tf9exception(void);
extern void __rtti_si(ti_t *, const char *, ti_t *);
extern ti_t __ti8bad_cast;
extern ti_t __ti9exception;
extern char str_8bad_cast[];   /* 01F20A50 */

void *__tf8bad_cast(void)
{
    if (__ti8bad_cast.name == 0) {
        __tf9exception();
        __rtti_si(&__ti8bad_cast, str_8bad_cast, &__ti9exception);
    }
    return &__ti8bad_cast;
}
