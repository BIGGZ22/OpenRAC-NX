/* scePadSetActAlign 0x01EA42A8..0x01EA4380 (216 bytes of body; the interval
 * to 0x01EA4408 continues with the next routine at 0x01EA43A0).
 *
 * Send the six actuator-alignment bytes of one pad slot to the pad server and,
 * if the server answers 1, commit them to the slot.
 *
 *   0x01EA42A8  lui $v1,0x1FD / addiu $s0,$v1,0x3840   request block
 *   0x01EA42D0  sw  $v0(8), 0x3840($v1)                type 8
 *   0x01EA42D8  sw  $s2, 0x4($s0)                      port
 *   0x01EA42E0  sw  $s1, 0x8($s0)                      slot
 *   0x01EA42E8  addu/lbu/sb loop over 6 bytes into +0xC
 *   0x01EA432C  sceSifCallRpc(client,1,0,block,0x80,block,0x80,0,0)
 *   0x01EA4348  bne $v1,1 -> return result          (result = +0x14)
 *   0x01EA4358  func_1EA4058(port, slot, 2)
 *
 * The request block doubles as the receive buffer, so the answer lands in the
 * same 0x80 bytes the request was built in. */

struct PadActData {
    int type;                      /* +0x00: 8 = set-act-align */
    int port;                      /* +0x04 */
    int slot;                      /* +0x08 */
    unsigned char align[8];        /* +0x0C: six meaningful bytes */
    int result;                    /* +0x14 */
};

extern int pad_act_req;            /* 0x01FD3840 */
extern int pad_act_client[];       /* 0x01FD3600 */

extern int sceSifCallRpc(void *, int, int, void *, int, void *, int, void *, void *);
extern int func_1EA4058(int, int, int);

int scePadSetActAlign(int port, int slot, unsigned char *data) {
    struct PadActData *d = (struct PadActData *)&pad_act_req;
    int i;

    d->type = 8;
    d->port = port;
    d->slot = slot;
    for (i = 0; i < 6; i++) {
        d->align[i] = data[i];
    }
    if (sceSifCallRpc(pad_act_client, 1, 0, d, 0x80, d, 0x80, 0, 0) < 0) {
        return 0;
    }
    if (d->result == 1) {
        func_1EA4058(port, slot, 2);
    }
    return d->result;
}
