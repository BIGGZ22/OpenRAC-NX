extern void *(*__g_eh_info_getter)(void); /* 0x01EF6B90 */

char *__get_eh_info(void) {
    return (char *)__g_eh_info_getter() + 8;
}
