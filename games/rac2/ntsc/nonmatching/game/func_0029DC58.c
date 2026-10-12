/* from src/game/0029BCC8.c: 52-byte near miss; 26 bytes differ at the boot placement */
int func_0029DC58(void) {
    int result;
    if (*(unsigned char *)0x001A7A0A != 0)
        goto false_result;
    if (*(unsigned int *)0x001397E0 & 0x08000000)
        goto true_result;
false_result:
    result = 0;
    goto done;
true_result:
    result = 1;
done:
    return result;
}
