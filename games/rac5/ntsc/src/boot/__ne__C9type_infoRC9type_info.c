extern int __eq__C9type_infoRC9type_info(const void *self, const void *arg);

int __ne__C9type_infoRC9type_info(const void *self, const void *arg) {
    return __eq__C9type_infoRC9type_info(self, arg) ^ 1;
}
