/* __tf16__user_type_info — GCC 2.9 RTTI accessor: lazily builds its si_type_info through
   __rtti_si after the base-class accessor, then returns it.
   Exact under route Q (Sony sdk991111 -O2 -G0 + pinned GNU as): 80/80.
   The pinned GNU chain keeps an 8-byte callee-save stride (76/80), so this
   body carries the sdk991111 selector in config/cflags.txt. */
struct si_type_info {
    char *name;
    void *vptr;
    const void *base;
};

extern struct si_type_info __ti16__user_type_info;
extern struct si_type_info __ti9type_info;
extern char STR_name_01F20A28[];

extern void __tf9type_info(void);
extern void __rtti_si(struct si_type_info *self, char *name, const void *base);

struct si_type_info *__tf16__user_type_info(void) {
    if (__ti16__user_type_info.name == 0) {
        __tf9type_info();
        __rtti_si(&__ti16__user_type_info, STR_name_01F20A28, &__ti9type_info);
    }
    return &__ti16__user_type_info;
}
