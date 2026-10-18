/* radfree - RAD memory block release; a header byte selects the custom
   destructor recorded eight bytes before the block. */

extern void free(void *);

void radfree(void *p)
{
    unsigned char *b;

    if (p == 0)
        return;
    b = (unsigned char *)p - 0x8000;
    if (b[0x7FFE] == 3)
        (*(void (**)(void *))(b + 0x7FF8))((char *)p - b[0x7FFF]);
    else
        free((char *)p - b[0x7FFF]);
}
