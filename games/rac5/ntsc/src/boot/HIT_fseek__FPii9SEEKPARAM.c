typedef struct {
    int values[3];
} SEEKPARAM;

extern SEEKPARAM g_seekParams;
extern int higIoLseek__Fiii(int handle, int offset, int whence);

void HIT_fseek__FPii9SEEKPARAM(int handle, int offset, int mode) {
    SEEKPARAM local = g_seekParams;
    higIoLseek__Fiii(handle - 1, offset, local.values[mode]);
}
