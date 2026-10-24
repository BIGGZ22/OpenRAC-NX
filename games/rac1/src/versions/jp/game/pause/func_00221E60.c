int update_menu_option_list(char *arg0) {
    char *g = menu_system;
    int sel = *(char **)(*(char **)(g + 4) + 0x40) == arg0;

    if (*(int *)(arg0 + 0x3C) != 0) {
        int t = *(int *)(arg0 + 0x3C) - 1;
        *(int *)(arg0 + 0x3C) = t;
        gFade = (float)(t < 5 ? t : 4) * 0.25f;
        return 0;
    }
    if (!sel) {
        return 0;
    }
    {
        char *pad = D_0013CA40;
        if (*(int *)(pad + 0x1C4) & 0xD00) {
            if (*(int *)(g + 0x124) == 0) {
                return 1;
            }
        }
    }
    {
        char *pad2 = D_0013CA40;
        if (*(int *)(pad2 + 0x1C4) & 0x10) {
            char *g2 = menu_system;
            int t = *(int *)(*(char **)(g2 + 4) + 0x38);
            if (t != 0) {
                *(int *)(g2 + 8) = t;
                return 0;
            }
            if (*(int *)(g2 + 0x124) == 0) {
                return -1;
            }
        }
    }
    {
        char *pad3 = D_0013CA40;
        int old = *(int *)(arg0 + 0x38);
        if ((*(int *)(pad3 + 0x1C4) & 0x1000) && old != 0) {
            *(int *)(arg0 + 0x38) = old - 1;
        }
        {
            char *pad4 = D_0013CA40;
            if (*(int *)(pad4 + 0x1C4) & 0x4000) {
                int c = *(int *)(arg0 + 0x38);
                if ((*(Row14 **)(arg0 + 0x34))[c + 1].w0 != 0) {
                    *(int *)(arg0 + 0x38) = c + 1;
                }
            }
        }
        {
            char *pad5 = D_0013CA40;
            if (*(int *)(pad5 + 0x1C4) & 0x60) {
                Moby_PlaySound(0, 0x11, *(int *)(arg0 + 0x14));
                if ((*(Row14 **)(arg0 + 0x34))[*(int *)(arg0 + 0x38)].flags & 1) {
                    if (*(unsigned char *)&D_0016044C_i) {
                        char *g3 = menu_system;
                        mode_freeze_init(6, *(void **)(g3 + 4), 0);
                    } else {
                        FadeToBlack(4);
                        *(int *)(arg0 + 0x3C) = func_001F98C0(0x10);
                        *(unsigned char *)&D_0016044C_i = !*(unsigned char *)&D_0016044C_i;
                    }
                } else {
                    unsigned char *buf = (*(Row14 **)(arg0 + 0x34))[*(int *)(arg0 + 0x38)].buf;
                    if (buf != 0) {
                        buf[0] = !buf[0];
                    }
                }
            }
        }
        if (*(int *)(arg0 + 0x38) != old) {
            Moby_PlaySound(1, 0x11, *(int *)(arg0 + 0x14));
        }
    }
    return 0;
}
