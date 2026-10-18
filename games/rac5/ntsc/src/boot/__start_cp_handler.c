struct cp_eh_info {
    char pad0[0x14];
    int handler_active;
    char pad1[8];
    long long handler_count;
};

extern struct cp_eh_info **__get_eh_info(void);

void *__start_cp_handler(void) {
    struct cp_eh_info *info = *__get_eh_info();
    info->handler_active = 1;
    info->handler_count++;
    return info;
}
