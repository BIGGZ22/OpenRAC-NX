extern void *V_01F209E8[];
extern void __builtin_delete(void *ptr);

void _$_9type_info(void *self, int flags) {
    *(void **)((char *)self + 4) = V_01F209E8;
    if (flags & 1)
        __builtin_delete(self);
}
