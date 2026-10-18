extern char iop_module_path_format[]; /* 0x01F1EF90 */
extern int sprintf(char *buffer, const char *format, ...);
extern int sceSifLoadModule(const char *path, int argc, const char **argv);

int SYSTEM_LoadIOPModule__FPc(char *module) {
    char path[128];
    int result;
    sprintf(path, iop_module_path_format, module);
    result = sceSifLoadModule(path, 0, 0);
    if (result < 0) return 0;
    return 1;
}
