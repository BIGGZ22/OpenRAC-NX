extern int WAD_LoadInternal(const char *name, void *header, void *data, int flag);

int WAD_Load__FPCcP10WAD_HEADER(const char *name, void *header) {
    while (WAD_LoadInternal(name, &header, 0, 1) == 0) {
        ;
    }
    return 1;
}
