extern void (*__terminate_handler)(void);

void __terminate(void) {
    __terminate_handler();
}
