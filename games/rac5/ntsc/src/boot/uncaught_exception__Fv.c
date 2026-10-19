struct cp_eh_info {
    char pad0[0x14];
    int handler_active;
};

extern struct cp_eh_info **__get_eh_info(void);

int uncaught_exception__Fv(void) {
    struct cp_eh_info *info = *__get_eh_info();
    int result = 0;
    if (info)
        result = !info->handler_active;
    return result;
}
