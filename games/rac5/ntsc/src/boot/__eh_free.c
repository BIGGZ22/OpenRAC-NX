extern void free(void *p);

void __eh_free(void *ptr) {
    free(ptr);
}
