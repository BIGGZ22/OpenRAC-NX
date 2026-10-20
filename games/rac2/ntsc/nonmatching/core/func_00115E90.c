/* from src/core/00115200.c: C that compiles to the same length but permutes two registers */
/*
 * Handler-list walk: for every node of the list at obj+0x148, call the node's
 * callbacks from the last down to the first with arg0; then the object's own
 * handler at obj+0x3C, which is called with the object; then func_00131CA8,
 * which does not return - that is why the frame is never popped and the
 * epilogue is absent from retail.
 *
 * The shape, the call order and the noreturn tail are all reproduced. What
 * differs is which of the two saved registers holds what: retail keeps the
 * loop counter in $s0 and the entry pointer in $s1, this cc1 the other way
 * round, and the same swap shows in the node walk ($v0 against $s0 for the
 * count). Tried: the object held in a local against re-reading D_00133E74 at
 * the two uses - the second reads closer (+0x34 differs instead of +0x2C) but
 * neither matches, so this is the allocator's own choice.
 */
typedef struct Node_00115E90 {
    struct Node_00115E90 *next;
    int count;
    void (*callbacks[1])(int arg0);
} Node_00115E90;

typedef struct Obj_00115E90 {
    char pad0[0x3C];
    void (*handler)(void *arg0);
    char pad1[0x148 - 0x40];
    Node_00115E90 *list;
} Obj_00115E90;

extern Obj_00115E90 *D_00133E74;

extern void func_00131CA8(int arg0) __attribute__((noreturn));

void func_00115E90(int arg0) {
    Obj_00115E90 *obj = D_00133E74;
    Node_00115E90 *node;

    for (node = obj->list; node != 0; node = node->next) {
        int i;

        for (i = node->count - 1; i >= 0; i--)
            node->callbacks[i](arg0);
    }
    if (obj->handler != 0)
        obj->handler(obj);
    func_00131CA8(arg0);
}
