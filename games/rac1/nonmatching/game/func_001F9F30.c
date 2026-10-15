/* FastVectorToPackedChars: the top byte of each of the four words (vftoi0 truncates to int),
 * packed into the low four bytes of the result, x lowest. Hand-checked against ppach/ppacb. */
int func_001F9F30(float *v) {
    int x = (int)v[0];
    int y = (int)v[1];
    int z = (int)v[2];
    int w = (int)v[3];
    return ((x >> 24) & 0xFF) | (((y >> 24) & 0xFF) << 8) |
           (((z >> 24) & 0xFF) << 16) | (((w >> 24) & 0xFF) << 24);
}
