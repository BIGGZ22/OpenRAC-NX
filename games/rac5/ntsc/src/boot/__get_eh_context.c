extern void *(*__eh_context_fn)(void);

void *__get_eh_context(void) {
    return __eh_context_fn();
}
