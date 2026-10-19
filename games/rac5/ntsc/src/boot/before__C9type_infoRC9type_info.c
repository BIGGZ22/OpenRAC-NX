/* before__C9type_infoRC9type_info 0x01E99F80..0x01E99FA8 */
typedef struct __type_info {
    const char *name;
    void *vtable;
} __type_info;

extern int strcmp(const char *a, const char *b);

int before__C9type_infoRC9type_info(const __type_info *a, const __type_info *b) {
    return strcmp(a->name, b->name) < 0;
}
