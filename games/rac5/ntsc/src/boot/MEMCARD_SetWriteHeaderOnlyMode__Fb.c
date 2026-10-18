extern unsigned char memcard_write_header_only;

void MEMCARD_SetWriteHeaderOnlyMode__Fb(int header_only) {
    memcard_write_header_only = header_only;
}
