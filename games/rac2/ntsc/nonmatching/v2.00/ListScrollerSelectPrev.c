typedef struct { int count; int cur; int pad08; void *items[1]; } LS;
void ListScrollerSelectPrev(LS *s)
{
    int n;
    int i;

    do {
        n = s->cur - 1;
        s->cur = n;
        i = n;
        if (i <= 0) {
            i = s->count;
        }
        s->cur = i;
    } while (s->items[i] == 0);
}
