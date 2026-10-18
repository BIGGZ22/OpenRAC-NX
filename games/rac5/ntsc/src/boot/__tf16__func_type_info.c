/* __tf16__func_type_info - GCC 2.9 RTTI accessor: builds __ti16__func_type_info lazily through __rtti_si once,
   after the base type_info accessor has run, then hands it back. */

typedef struct {
    void *name;      /* +0x0 */
    void *vtable;    /* +0x4 */
    void *base;      /* +0x8 */
} ti_t;

extern void __tf9type_info(void);
extern void __rtti_si(ti_t *, const char *, ti_t *);
extern ti_t __ti16__func_type_info;
extern ti_t __ti9type_info;
extern char str_16__func_type_info[];   /* 01F20BC0 */

void *__tf16__func_type_info(void)
{
    if (__ti16__func_type_info.name == 0) {
        __tf9type_info();
        __rtti_si(&__ti16__func_type_info, str_16__func_type_info, &__ti9type_info);
    }
    return &__ti16__func_type_info;
}
