/* func_L00_00401D50 — NOT MATCHED, best attempt — 212 bytes expected, 27 places.
 * last check.py line: "00_aranos_tutorial: 204 bytes, expected 212"
 * then "func_L00_00401D50: 0 of 27 places match".
 *
 * Body almost identical: exactly the pair
 *   beql $s0,$zero, <loop test>  ;  lw $s0,0x28($s0)   (delay)
 * is missing, which the retail emits BEFORE the load of node->obj, that is a
 * redundant `if (node == 0) goto next` test inside the body. Tried without
 * obtaining it: `while` loop, `for(;node;node=node->next)` loop, chain of
 * `&&`, separate `continue`. The rest (state 0xFE/0xFD, obj==0,
 * *(s16*)(obj+0x46)==0x13, floating-point call, duplication of
 * node=node->next on every exit) is reproduced instruction by instruction.
 *
 * WARNING: this body must live in the -g8.c sibling — the retail loads the
 * float as `lwc1 $f1,0x1CA0($gp)` in ordinary flow (=> -G8 file), while
 * keeping $gp free for itself. The two integers of the body are absolute
 * (folded lui + lw), obtained here by declaring them as ARRAYS (>8 bytes
 * hence outside small-data at -G8), where D_L00_001B0C90 (float, 4 bytes) is
 * in sda.
 */

typedef struct Node Node;
struct Node {
    unsigned char pad0[32];
    unsigned char state;
    unsigned char pad1[3];
    int *obj;
    Node *next;
};

extern int D_L00_001B39C8[4];
extern void *D_L00_001B3EA8[4];
extern float D_L00_001B0C90 __attribute__((sda));
extern float func_002EE2B0(char *a, char *b);
extern void func_L00_00381F80(Node *a, char *b);

void func_L00_00401D50(char *arg)
{
    Node *node;

    if ((D_L00_001B39C8[0] & 1) == 0)
        return;
    node = D_L00_001B3EA8[0];
    while (node != 0) {
        if (node->state == 0xFE)
            return;
        if (node->state == 0xFD)
            return;
        if (node->obj != 0 && *(short *)((char *)node->obj + 0x46) == 0x13 &&
            func_002EE2B0((char *)node + 16, arg + 16) < D_L00_001B0C90)
            func_L00_00381F80(node, arg);
        node = node->next;
    }
}
