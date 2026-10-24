void request_taxi_stop_prompt(char *moby) {
    char *data = *(char **)(moby + 0x78);
    int v = *(short *)(data + 0xB4);
    if (*(unsigned char *)(moby + 0x20) == 5) {
        v = *(short *)(data + 0xB6);
    }
    switch (v) {
    case 0:
        try_set_help_message(4, 0x53ED);
        break;
    case 1:
        try_set_help_message(4, 0x53EE);
        break;
    case 2:
        try_set_help_message(4, 0x53EE);
        break;
    case 3:
        try_set_help_message(4, 0x53EE);
        break;
    case 4:
        try_set_help_message(4, 0x1399);
        break;
    default:
        try_set_help_message(4, 0x139B);
        break;
    }
}
