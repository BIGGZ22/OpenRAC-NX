/* snd_GetLastLoadError 0x01E95C58..0x01E95CB0 */
extern int gLocalLoadError; /* 0x01EF6A80 */
extern void snd_SendIOPCommandAndWait(int command, int arg1, int arg2);

void snd_GetLastLoadError(void) {
    if (gLocalLoadError == 0) {
        snd_SendIOPCommandAndWait(0x52, 0, 0);
    } else {
        gLocalLoadError = 0;
    }
}
