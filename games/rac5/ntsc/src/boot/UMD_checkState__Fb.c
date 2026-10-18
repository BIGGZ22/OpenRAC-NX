/* Mangling __Fb: the parameter is a 1-byte bool in this binary (see the
   `sb $a0` stores of the other __Fb setters). It is unused here. */
int UMD_checkState__Fb(unsigned char state) {
    (void)state;
    return 0;
}
