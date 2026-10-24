extern int D_L00_00173CF0[];
void update_slideshow(void) {
    int s0, t, i, r, one;
    int *src, *dst;
    float f;
    update_audio_stream_until_idle(0);
    f = *(float *)&gFadeLvl - 0.05f;
    *(float *)&gFadeLvl = f;
    if (f < 0.0f) {
        *(float *)&gFadeLvl = 0.0f;
    }
    s0 = *(unsigned char *)(D_0013A5E0 + 0x2CF2) == 0x41;
    one = 1;
    if (IST == 0) {
        if (ITMR >= D_L00_00173CF0[IIDX] - 0x24) {
            if (*(short *)(D_0014171B + 0x100BD) != 0) {
                update_audio_stream_until_idle(1);
            }
            if (!s0 && IIDX == 0x27) {
            quit:
                end_slideshow();
                return;
            }
            if (IIDX >= 0x2B) goto quit;
            ICNT = -1;
            IST = 1;
        }
    } else if (IST == one) {
        VU1_syncChain(1);
        t = ICNT;
        if (t < 0x20) {
            src = (int *)slideshow[3];
            dst = (int *)slideshow[2];
            src += t;
            dst += t;
            for (i = 0; i <= 0x37FFF; i += 0x20) {
                *dst = *src;
                dst += 0x20;
                src += 0x20;
            }
        } else if (t >= 0x24) {
            if (!s0) {
                if (IIDX == 8) {
                    IIDX = 0x11;
                    ITMR = 0x1590;
                }
            } else {
                if (IIDX == 0x11) {
                    IIDX = 0x1B;
                    ITMR = 0x2238;
                }
                if (IIDX == 0x25) {
                    IIDX = 0x27;
                    ITMR = 0x32B4;
                }
            }
            IST = 0;
            ICNT = -1;
            IIDX = IIDX + 1;
            r = IIDX % 20;
            if (D_0015EE80 != 0) {
                start_audio_stream_read(slideshow[3], D_00137C80.alt[r].a, D_00137C80.alt[r].b);
            } else {
                start_audio_stream_read(slideshow[3], D_00137C80.normal[r].a, D_00137C80.normal[r].b);
            }
        }
    }
    ICNT = ICNT + 1;
    ITMR = ITMR + 1;
    if (*(int *)(D_0013A5E0 + 0x2604) & 0x800) {
        end_slideshow();
    }
    sound_update();
}
