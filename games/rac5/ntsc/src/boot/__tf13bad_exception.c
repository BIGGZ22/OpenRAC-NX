/* __tf13bad_exception — GCC 2.9 RTTI accessor: lazily builds its si_type_info through
   __rtti_si after the base-class accessor, then returns it.
   Exact under route Q (Sony sdk991111 -O2 -G0 + pinned GNU as): 80/80.
   The pinned GNU chain keeps an 8-byte callee-save stride (76/80), so this
   body carries the sdk991111 selector in config/cflags.txt. */
struct si_type_info {
    char *name;
    void *vptr;
    const void *base;
};

extern struct si_type_info __ti13bad_exception;
extern struct si_type_info __ti9exception;
extern char STR_name[];

extern void __tf9exception(void);
extern void __rtti_si(struct si_type_info *self, char *name, const void *base);

struct si_type_info *__tf13bad_exception(void) {
    if (__ti13bad_exception.name == 0) {
        __tf9exception();
        __rtti_si(&__ti13bad_exception, STR_name, &__ti9exception);
    }
    return &__ti13bad_exception;
}
