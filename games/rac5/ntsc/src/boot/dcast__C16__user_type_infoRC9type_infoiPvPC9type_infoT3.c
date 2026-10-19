extern int __eq__C9type_infoRC9type_info(const void *self, const void *arg);

void *dcast__C16__user_type_infoRC9type_infoiPvPC9type_infoT3(
    const void *self, const void *arg, int is_public, void *ptr,
    const void *target) {
    void *result = ptr;
    (void)is_public;
    (void)target;
    if (__eq__C9type_infoRC9type_info(self, arg) == 0)
        result = 0;
    return result;
}
