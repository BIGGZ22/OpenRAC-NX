/* __rtti_array 0x01E9A568..0x01E9A588 */
typedef struct __array_type_info {
    const char *name;
    void *vtable;
} __array_type_info;

extern void *array_type_info_vtable; /* 0x01F20B08 */

void __rtti_array(__array_type_info *p, const char *name) {
    if (p) {
        p->name = name;
        p->vtable = &array_type_info_vtable;
    }
}
