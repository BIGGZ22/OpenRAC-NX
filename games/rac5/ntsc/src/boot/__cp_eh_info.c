extern char **__get_eh_info(void);

char *__cp_eh_info(void) {
    return *__get_eh_info();
}
