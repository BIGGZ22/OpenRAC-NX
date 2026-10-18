/* __tf10bad_typeid - GCC 2.9 RTTI accessor: builds __ti10bad_typeid lazily through __rtti_si once,
   after the base type_info accessor has run, then hands it back. */

typedef struct {
    void *name;      /* +0x0 */
    void *vtable;    /* +0x4 */
    void *base;      /* +0x8 */
} ti_t;

extern void __tf9exception(void);
extern void __rtti_si(ti_t *, const char *, ti_t *);
extern ti_t __ti10bad_typeid;
extern ti_t __ti9exception;
extern char str_10bad_typeid[];   /* 01F20A40 */

void *__tf10bad_typeid(void)
{
    if (__ti10bad_typeid.name == 0) {
        __tf9exception();
        __rtti_si(&__ti10bad_typeid, str_10bad_typeid, &__ti9exception);
    }
    return &__ti10bad_typeid;
}
