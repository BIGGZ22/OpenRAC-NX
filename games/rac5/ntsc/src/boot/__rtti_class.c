struct class_type_info {
    char *name;
    void *vptr;
    const void *base_info;
    unsigned int count;
};

extern void *V_01F20970[];

void __rtti_class(struct class_type_info *self, char *name,
                  const void *base_info, unsigned int count) {
    if (self) {
        self->name = name;
        self->vptr = V_01F20970;
        self->base_info = base_info;
        self->count = count;
    }
}
