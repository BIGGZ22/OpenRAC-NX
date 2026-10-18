extern void (*__terminate_handler)(void) __attribute__((noreturn));

void terminate__Fv(void) {
    __terminate_handler();
}
