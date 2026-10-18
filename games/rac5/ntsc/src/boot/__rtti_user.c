struct type_info {
    char *name;
    void *vptr;
};

extern void *V_01F209A0[];

void __rtti_user(struct type_info *self, char *name, const void *type) {
    (void)type;
    if (self) {
        self->name = name;
        self->vptr = V_01F209A0;
    }
}
