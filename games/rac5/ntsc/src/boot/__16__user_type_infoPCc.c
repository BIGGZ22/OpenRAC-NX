struct type_info {
    char *name;
    void *vptr;
};

extern void *V_01F209A0[];

void *__16__user_type_infoPCc(struct type_info *self, char *name) {
    self->name = name;
    self->vptr = V_01F209A0;
    return self;
}
