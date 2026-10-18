/* __rtti_func 0x01E9A508..0x01E9A528 */
typedef struct __func_type_info {
    const char *name;
    void *vtable;
} __func_type_info;

extern void *func_type_info_vtable; /* 0x01F20B38 */

void __rtti_func(__func_type_info *p, const char *name) {
    if (p) {
        p->name = name;
        p->vtable = &func_type_info_vtable;
    }
}
