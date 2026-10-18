int sceGimPictureCheckFileHeader__FPCvi(const void *data, int version) {
    if (data == 0) return 0;
    if ((unsigned int)(version - 1) < 0x1F) return 0;
    if (*(const int *)data != 0x2E47494D) return 0;
    if (*(const int *)((const char *)data + 4) != 0x312E3030) return 0;
    return (*(const unsigned int *)((const char *)data + 8) ^ 0x00505350u) == 0;
}
