extern void (*__unexpected_handler)(void) __attribute__((noreturn));

void unexpected__Fv(void) {
    __unexpected_handler();
}
