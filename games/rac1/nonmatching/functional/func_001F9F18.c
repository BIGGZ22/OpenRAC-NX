/* functional: high. FastVectorFromPackedChars: out[i] = (float)byte i of packed (low byte first), as unsigned. */
void func_001F9F18(float *out, unsigned int packed) {
    out[0] = (float)(int)(packed & 0xFF);
    out[1] = (float)(int)((packed >> 8) & 0xFF);
    out[2] = (float)(int)((packed >> 16) & 0xFF);
    out[3] = (float)(int)(packed >> 24);
}
