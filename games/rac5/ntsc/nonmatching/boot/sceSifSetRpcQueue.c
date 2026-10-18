/* RAC5 (2026-10-17, w11/s3) sceSifSetRpcQueue 0x01ED47F0..0x01ED4888 (152 o) — profil sdk (Sony 2.9)
 * Profil mesure : sauvegardes 16 o (s0@0x0 s1@0x10 ra@0x20) et appel frere
 * `j EIntr` avec restauration de trame — le cc1 epingle n'emet ni l'un ni l'autre.
 * Forme : le global est lu dans le test, la boucle reprend la valeur chargee. */
typedef struct sceSifRpcQueue {
    unsigned int pad0[5];               /* 0x00 .. 0x13 */
    struct sceSifRpcQueue *link;        /* 0x14 */
} sceSifRpcQueue;

typedef struct sceSifRpcQueueRoot {
    unsigned int pad[10];               /* 0x00 .. 0x27 */
    sceSifRpcQueue *head;               /* 0x28 */
} sceSifRpcQueueRoot;

extern sceSifRpcQueueRoot D_01FE58C0;   /* 0x01FE58C0 */
extern int DIntr(void);
extern void EIntr(void);

void sceSifSetRpcQueue(sceSifRpcQueue *queue, int rpc) {
    sceSifRpcQueue *p;
    DIntr();
    queue->pad0[0] = rpc;
    queue->pad0[1] = 0;
    queue->pad0[2] = 0;
    queue->pad0[3] = 0;
    queue->pad0[4] = 0;
    queue->link = 0;
    if (D_01FE58C0.head == 0) {
        D_01FE58C0.head = queue;
    } else {
        p = D_01FE58C0.head;
        while (p->link != 0)
            p = p->link;
        p->link = queue;
    }
    EIntr();
}
