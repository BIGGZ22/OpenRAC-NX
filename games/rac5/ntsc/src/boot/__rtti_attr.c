typedef struct __attr_type_info { const char *name; void *vtable; const char *base; const char *attr; } __attr_type_info;
extern void *attr_type_info_vtable;
void __rtti_attr(__attr_type_info *p, const char *name, const char *attr, const char *base) {
    if (p) {
        p->name = name;
        p->vtable = &attr_type_info_vtable;
        p->base = base;
        p->attr = attr;
    }
}
