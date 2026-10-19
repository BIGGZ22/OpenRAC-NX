/* __rtti_ptmf 0x01E9A528..0x01E9A548 */
typedef struct __ptmf_type_info {
    const char *name;
    void *vtable;
} __ptmf_type_info;

extern void *ptmf_type_info_vtable; /* 0x01F20B28 */

void __rtti_ptmf(__ptmf_type_info *p, const char *name) {
    if (p) {
        p->name = name;
        p->vtable = &ptmf_type_info_vtable;
    }
}
