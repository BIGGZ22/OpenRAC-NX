struct si_type_info {
    char *name;
    void *vptr;
    const void *base;
};

extern void *V_01F20988[];

void __rtti_si(struct si_type_info *self, char *name, const void *base) {
    if (self) {
        self->name = name;
        self->vptr = V_01F20988;
        self->base = base;
    }
}
