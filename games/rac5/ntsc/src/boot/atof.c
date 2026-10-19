/* newlib atof, retail 0x01EC83B8 (28 bytes + nop padding):
 *   return strtod(str, 0);
 */
extern double strtod(const char *str, char **endptr);

double atof(const char *str) {
    return strtod(str, 0);
}
