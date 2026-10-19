extern void *V_01F20808[];
extern void __builtin_delete(void *ptr);

void _$_10bad_typeid(void *self, int flags) {
    *(void **)self = V_01F20808;
    if (flags & 1)
        __builtin_delete(self);
}
