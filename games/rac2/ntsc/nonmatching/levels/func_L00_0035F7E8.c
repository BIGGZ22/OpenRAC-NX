/* Near match: table search; last candidate was 4 bytes larger than retail. */
struct Query_35F7E8 { short key; short index; int value; };
extern int D_L00_002A3B00[];
void func_L00_0035F7E8(struct Query_35F7E8 *q)
{
    int i = 0;
    if (D_L00_002A3B00[0] != -1 && D_L00_002A3B00[0] != q->key) {
        do {
            i++;
        } while (D_L00_002A3B00[i * 2] != -1 && D_L00_002A3B00[i * 2] != q->key);
    }
    q->index = i;
    q->value = D_L00_002A3B00[i * 2 + 1];
}
