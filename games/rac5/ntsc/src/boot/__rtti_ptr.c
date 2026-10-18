/* __rtti_ptr 0x01E9A4C0..0x01E9A4E0
   type_info initialiser for pointer types: p[0]=name, p[1]=vtable, p[2]=target. */
typedef struct __ptr_type_info {
    const char *name;
    void *vtable;
    const char *type;
} __ptr_type_info;

extern void *pointer_type_info_vtable; /* 0x01F20B68 */

void __rtti_ptr(__ptr_type_info *p, const char *name, const char *type) {
    if (p) {
        p->type = type;
        p->name = name;
        p->vtable = &pointer_type_info_vtable;
    }
}
