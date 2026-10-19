extern void SND_StopSoundsImmediate__Fb(int immediate);

int SND_Shutdown__Fv(void) {
    SND_StopSoundsImmediate__Fb(0);
    return 1;
}
