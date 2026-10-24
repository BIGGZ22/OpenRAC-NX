int update_timed_menu_sequence(char *arg0) {
    char *pad = D_0013CA40;
    char *g = menu_system;
    int fast = (*(long *)(pad + 0x1A0) & 0xF) == 0xF;
    int x;

    if (*(int *)(g + 0xDC) == 0 || (fast && (*(int *)(pad + 0x1A0) & 0x10))) {
        if (*(int *)(pad + 0x1C4) & 0xD00) {
            if (*(int *)(g + 0x124) == 0) {
                return 1;
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
    }
    x = *(int *)(arg0 + 0x40);
    *(int *)(arg0 + 0x40) = x - 1;
    if (fast) {
        *(int *)(arg0 + 0x40) = x - 2;
    }
    if (*(int *)(arg0 + 0x40) < 0) {
        *(int *)(arg0 + 0x40) = 0;
    }
    switch (*(int *)(arg0 + 0x50)) {
    case 0:
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        *(int *)(arg0 + 0x40) = func_001F98C0(0xB4);
        *(int *)(arg0 + 0x3C) = 0;
        D_001D2E74 = 0x50AA;
        *(char **)(arg0 + 0x34) = D_001D5418;
        break;
    case 1:
    case 5:
    case 8:
    case 12:
    case 16:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        *(int *)(arg0 + 0x54) = 0;
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        break;
    case 2:
    case 6:
    case 9:
    case 13:
    case 17:
        x = *(int *)(arg0 + 0x3C);
        *(int *)(arg0 + 0x3C) = x + 10;
        if (fast) {
            *(int *)(arg0 + 0x3C) = x + 20;
        }
        if (*(int *)(arg0 + 0x54) == 0) {
            return 0;
        }
        *(int *)(arg0 + 0x40) = func_001F98C0(0xB4);
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        break;
    case 4:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        *(int *)(arg0 + 0x50) = 5;
        *(int *)(arg0 + 0x40) = func_001F98C0(0xB4);
        *(int *)(arg0 + 0x3C) = 0;
        D_001D2E74 = 0x50D7;
        *(char **)(arg0 + 0x34) = D_001D54C8;
        break;
    case 3:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        D_001D2E74 = 0x50D5;
        *(int *)(arg0 + 0x34) = 0x50D6;
        *(int *)(arg0 + 0x40) = func_001F98C0(0xF0);
        *(int *)(arg0 + 0x3C) = 0;
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        break;
    case 7:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        D_001D2E74 = 0x5107;
        *(char **)(arg0 + 0x34) = D_001D5588;
        *(int *)(arg0 + 0x40) = func_001F98C0(0xF0);
        *(int *)(arg0 + 0x3C) = 0;
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        break;
    case 10:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        D_001D2E74 = 0x518C;
        *(int *)(arg0 + 0x34) = 0x518D;
        *(int *)(arg0 + 0x40) = func_001F98C0(0xF0);
        *(int *)(arg0 + 0x3C) = 0;
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        break;
    case 11:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        *(int *)(arg0 + 0x40) = func_001F98C0(0xB4);
        *(int *)(arg0 + 0x3C) = 0;
        D_001D2E74 = 0x518E;
        *(char **)(arg0 + 0x34) = D_001D5618;
        *(int *)(arg0 + 0x50) = 12;
        break;
    case 14:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        D_001D2E74 = 0;
        *(int *)(arg0 + 0x34) = 0x5199;
        *(int *)(arg0 + 0x40) = func_001F98C0(0xF0);
        *(int *)(arg0 + 0x3C) = 0;
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        break;
    case 15:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        *(int *)(arg0 + 0x40) = func_001F98C0(0xB4);
        *(int *)(arg0 + 0x3C) = 0;
        D_001D2E74 = 0x519A;
        *(char **)(arg0 + 0x34) = D_001D5630;
        *(int *)(arg0 + 0x50) = 16;
        break;
    case 18:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        D_001D2E74 = 0;
        *(int *)(arg0 + 0x34) = 0x51CB;
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        *(int *)(arg0 + 0x40) = func_001F98C0(300);
        break;
    case 19:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        {
            char *g3 = menu_system;
            if (*(int *)(g3 + 0xDC) != 0) {
                return 1;
            }
        }
        break;
    }
    return 0;
}
