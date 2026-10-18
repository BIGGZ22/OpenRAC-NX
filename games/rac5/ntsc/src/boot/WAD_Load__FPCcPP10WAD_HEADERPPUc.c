extern int WAD_LoadInternal(const char *name, void *header, void *data, int flag);

int WAD_Load__FPCcPP10WAD_HEADERPPUc(const char *name, void *header, void *data) {
    while (WAD_LoadInternal(name, header, data, 0) == 0) {
        ;
    }
    return 1;
}
