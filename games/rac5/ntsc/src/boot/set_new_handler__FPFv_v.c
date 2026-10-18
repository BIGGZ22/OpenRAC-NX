extern void (*__new_handler)(void);

void (*set_new_handler__FPFv_v(void (*handler)(void)))(void) {
    void (*old)(void) = __new_handler;
    __new_handler = handler;
    return old;
}
