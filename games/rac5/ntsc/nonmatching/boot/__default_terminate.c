extern void abort(void);

void __default_terminate(void) {
    abort();
}
