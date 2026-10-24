void mode_freeze_init(int arg0, int arg1, int arg2) {
    char *s = D_00193400;

    *(int *)(s + 0x30) = arg2;
    if (D_0015F6E8 != 3) {
        func_0012E528(0x1D);
        music_Pause(0);
    }
    *(int *)(s + 0x18) = arg1;
    *(int *)(s + 0x14) = D_0015F6E8;
    D_0015F6E8 = 4;
    *(int *)s = arg0;
    *(int *)(s + 0x2C) = 0;
    switch (arg0) {
    case 0: {
        char *p = D_00193400;
        *(int *)(p + 8) = msg_string(0x4F6F);
        *(int *)(p + 0xC) = msg_string(0x529F);
        *(int *)(p + 0x10) = msg_string(0x52A0);
        *(int *)(p + 4) = 0;
        *(int *)(p + 0x1C) = 0;
        *(int *)(p + 0x20) = 0;
        *(int *)(p + 0x24) = 0;
        *(int *)(p + 0x28) = 0;
        break;
    }
    case 2: {
        int v = msg_string(0x52A0);
        char *p = D_00193400;
        *(int *)(p + 8) = v;
        *(int *)(p + 0xC) = 0;
        *(int *)(p + 4) = 0;
        break;
    }
    case 1:
    case 4: {
        char *p = D_00193400;
        *(int *)(p + 8) = msg_string(0x527F);
        *(int *)(p + 0xC) = msg_string(0x4EE0);
        *(int *)(p + 0x10) = msg_string(0x52A0);
        *(int *)(p + 4) = 0;
        break;
    }
    case 5: {
        char *p = D_00193400;
        push_help_history(0x4E2B);
        *(int *)(p + 4) = func_001F98C0(0x1E);
        *(int *)(p + 0x20) = 0;
        *(int *)(p + 0x24) = func_001F98C0(0x1E);
        break;
    }
    case 7:
    case 8: {
        char *p = D_00193400;
        *(int *)(p + 4) = func_001F98C0(0x1E);
        *(int *)(p + 0x20) = -1;
        *(int *)(p + 0x24) = func_001F98C0(0x1E);
        break;
    }
    case 3: {
        char *p = D_00193400;
        *(int *)(p + 4) = func_001F98C0(0x1E);
        *(int *)(p + 0x20) = -1;
        *(int *)(p + 0x24) = func_001F98C0(0x1E);
        D_0013D440 = 3;
        *(int *)(p + 0x2C) = func_001F98C0(0xA);
        break;
    }
    case 6: {
        int v = func_001F98C0(0x1E);
        char *p = D_00193400;
        *(int *)(p + 4) = v;
        *(int *)(p + 0x1C) = 0;
        break;
    }
    default:
        *(int *)(s + 4) = 0x78;
        *(int *)(s + 0xC) = 0;
        *(int *)(s + 0x10) = 0;
        *(int *)(s + 8) = 0;
        break;
    }
}
