/* __9type_infoPCc 0x01E99F28..0x01E99F40 */
typedef struct __type_info {
    const char *name;
    void *vtable;
} __type_info;

extern void *V_01F209E8; /* 0x01F209E8 */

__type_info *__9type_infoPCc(__type_info *self, const char *name) {
    self->name = name;
    self->vtable = &V_01F209E8;
    return self;
}
