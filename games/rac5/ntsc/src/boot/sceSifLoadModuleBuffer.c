extern int func_1ED8028(void *a, int b, int c, int *out);

int sceSifLoadModuleBuffer(void *a, int b, int c) {
    int out;
    return func_1ED8028(a, b, c, &out);
}
