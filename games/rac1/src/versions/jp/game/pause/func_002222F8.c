int update_menu_choice_list(char *arg0) {
    char *g = menu_system;

    if (*(char **)(*(char **)(g + 4) + 0x40) != arg0) {
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
                if ((*(Row18 **)(arg0 + 0x34))[c + 1].w0 != 0) {
                    *(int *)(arg0 + 0x38) = c + 1;
                }
            }
        }
        if (old != *(int *)(arg0 + 0x38)) {
            Moby_PlaySound(1, 0x11, *(int *)(arg0 + 0x14));
        }
    }
    {
        Row18 *rec = &(*(Row18 **)(arg0 + 0x34))[*(int *)(arg0 + 0x38)];
        int n = 0;
        char *pad5;
        int *e;

        e = rec->e;
        if (e[0] != 0) {
            do {
                n++;
            } while (e[n] != 0 && n < 4);
        }
        pad5 = D_0013CA40;
        if (*(int *)(pad5 + 0x1C4) & 0x60) {
            unsigned char *buf = rec->buf;
            if (buf != 0) {
                buf[0] = (buf[0] + 1) % n;
                Moby_PlaySound(0, 0x11, *(int *)(arg0 + 0x14));
            }
        }
    }
    return 0;
}
