typedef struct {
    char pad0[0x488];
    int enabled[1];
} Popup;

void SetPopupItemEnabled(Popup *popup, int index, int enabled)
{
    popup->enabled[index] = enabled;
}
