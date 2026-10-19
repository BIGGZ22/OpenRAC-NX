/* __rtti_ptmd 0x01E9A548..0x01E9A568 */
typedef struct __ptmd_type_info {
    const char *name;
    void *vtable;
} __ptmd_type_info;

extern void *ptmd_type_info_vtable; /* 0x01F20B18 */

void __rtti_ptmd(__ptmd_type_info *p, const char *name) {
    if (p) {
        p->name = name;
        p->vtable = &ptmd_type_info_vtable;
    }
}
