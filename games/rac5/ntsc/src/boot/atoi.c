/* newlib atoi, retail 0x01EC83D8 (40 bytes):
 *   return (int)strtol(str, 0, 10);
 */
extern long strtol(const char *str, char **endptr, int base);

int atoi(const char *str) {
    return (int)strtol(str, 0, 10);
}
