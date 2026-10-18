extern void *(*__eh_context_fn)(void);

void *__get_dynamic_handler_chain(void) {
    return (char *)__eh_context_fn() + 4;
}
