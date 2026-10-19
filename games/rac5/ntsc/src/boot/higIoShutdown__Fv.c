extern int hig_io_handle;
extern int hig_io_queue;

void higIoShutdown__Fv(void) {
    hig_io_queue = 0;
    hig_io_handle = 0;
}
