extern void *malloc(unsigned int size);
extern void terminate__Fv(void) __attribute__((noreturn));

void *__eh_alloc(unsigned int size) {
    void *p = malloc(size);
    if (p == 0)
        terminate__Fv();
    return p;
}
