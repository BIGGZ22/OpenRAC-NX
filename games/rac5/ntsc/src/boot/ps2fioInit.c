/* ps2fioInit - clear the handle table and register the built-in devices. */

extern void *memset(void *, int, unsigned int);
extern void cdvddrvRegisterDevice(void);
extern void hostdrvRegisterDevice(void);
extern void mcdrvRegisterDevices(void);
extern void hdddrvRegisterDevice(void);
extern char ps2fio_handles[];
extern char *ps2fio_drv_list;

int ps2fioInit(void)
{
    memset(ps2fio_handles, 0, 0x80);
    ps2fio_drv_list = 0;
    cdvddrvRegisterDevice();
    hostdrvRegisterDevice();
    mcdrvRegisterDevices();
    hdddrvRegisterDevice();
    return 0;
}
