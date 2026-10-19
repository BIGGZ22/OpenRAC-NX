extern void snd_SetSoundParams_CB(int a, int b, int c, int d, int e, int f,
                                  int g, int h);

void snd_SetSoundParams_A(int a, int b, int c, int d, int e, int f) {
    snd_SetSoundParams_CB(a, b, c, d, e, f, 0, 0);
}
