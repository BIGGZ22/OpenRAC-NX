extern void (*__terminate_handler)(void);

void (*set_terminate__FPFv_v(void (*handler)(void)))(void) {
    void (*old)(void) = __terminate_handler;
    __terminate_handler = handler;
    return old;
}
