extern void dtor_delete(void *p);

void _$_t5Queue2Z14tHUDPROMQ_NODEUi8(void *self, int flags) {
    if (flags & 1)
        dtor_delete(self);
}
