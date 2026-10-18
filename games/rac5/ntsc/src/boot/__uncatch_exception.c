struct cp_eh_info {
    char pad0[0x14];
    int handler_active;
};

extern struct cp_eh_info **__get_eh_info(void);
extern void terminate__Fv(void) __attribute__((noreturn));

void __uncatch_exception(void) {
    struct cp_eh_info *info = *__get_eh_info();
    if (info == 0)
        terminate__Fv();
    info->handler_active = 0;
}
