/* libgcc2.c (GNU CC 2.9, L_eh): pop the dynamic handler and throw.
   Retail: __sjpopnthrow 0x01E973D0..0x01E974E8. */
typedef struct eh_context {
    void *handler_label;
    void **dynamic_handler_chain;
    void *info;
    void *table_index;
} eh_context;

extern eh_context *(*get_eh_context)(void);
extern void __terminate(void) __attribute__((noreturn));
extern void __sjthrow(void) __attribute__((noreturn));

void __sjpopnthrow(void) {
    eh_context *eh = (*get_eh_context)();
    void ***dhc = &eh->dynamic_handler_chain;
    void (*func)(void *, int);
    void *arg;
    void ***cleanup;

    cleanup = (void ***)&(*dhc)[1];

    if (cleanup[0]) {
        double store[200];
        void **buf = (void **)store;
        buf[1] = 0;
        buf[0] = (*dhc);

        if (!__builtin_setjmp(&buf[2])) {
            *dhc = buf;
            while (cleanup[0]) {
                func = (void (*)(void *, int))cleanup[0][1];
                arg = (void *)cleanup[0][2];

                cleanup[0] = (void **)cleanup[0][0];

                (*func)(arg, 2);
            }
            *dhc = buf[0];
        } else {
            __terminate();
        }
    }

    *dhc = (void **)(*dhc)[0];

    __sjthrow();
}
