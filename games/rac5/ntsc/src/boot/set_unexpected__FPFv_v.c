extern void (*__unexpected_handler)(void);

void (*set_unexpected__FPFv_v(void (*handler)(void)))(void) {
    void (*old)(void) = __unexpected_handler;
    __unexpected_handler = handler;
    return old;
}
