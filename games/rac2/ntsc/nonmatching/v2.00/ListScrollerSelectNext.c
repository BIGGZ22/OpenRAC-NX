typedef struct {
    int count;
    int cur;
    int pad08;
    void *items[1];
} ListScroller;

void ListScrollerSelectNext(ListScroller *s)
{
    int i;

    do {
        i = s->cur + 1;
        if (s->count < i) {
            i = 0;
        }
        s->cur = i;
    } while (s->items[i] == 0);
}
