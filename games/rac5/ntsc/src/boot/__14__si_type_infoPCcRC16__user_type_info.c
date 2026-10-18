struct si_type_info {
    char *name;
    void *vptr;
    const void *base;
};

extern void *V_01F20988[];

void *__14__si_type_infoPCcRC16__user_type_info(struct si_type_info *self,
                                                char *name,
                                                const void *base) {
    self->name = name;
    self->vptr = V_01F20988;
    self->base = base;
    return self;
}
