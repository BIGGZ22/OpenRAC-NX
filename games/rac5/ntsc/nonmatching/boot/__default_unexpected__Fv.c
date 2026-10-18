extern void terminate__Fv(void) __attribute__((noreturn));

void __default_unexpected__Fv(void) {
    terminate__Fv();
}
