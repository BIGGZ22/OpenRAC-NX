typedef struct {
    char pad00[0xC];
    int *colors;
} GuiList;

void GuiListSetColorPair0(GuiList *list, int first, int second)
{
    list->colors[0] = first;
    list->colors[1] = second;
}
