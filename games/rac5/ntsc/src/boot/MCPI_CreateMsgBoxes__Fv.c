extern void *hig_msgbox1;
extern void *hig_msgbox2;
extern const char hig_msgbox_name1[];
extern const char hig_msgbox_name2[];
extern void *MBOX_Create__FPCcUiPv(const char *, unsigned int, void *);

void MCPI_CreateMsgBoxes__Fv(void) {
    if (hig_msgbox1 != 0)
        return;
    if (hig_msgbox2 != 0)
        return;
    hig_msgbox1 = MBOX_Create__FPCcUiPv(hig_msgbox_name1, 5, 0);
    hig_msgbox2 = MBOX_Create__FPCcUiPv(hig_msgbox_name2, 5, 0);
}
