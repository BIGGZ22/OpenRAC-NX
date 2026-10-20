/* sceSifCallRpc (libkernl.a:sifrpc.o), 492 bytes.

 * Complete transcription, adapted address-by-address from rac1-decomp's
 * matched func_0011B4C8 (which OpenRAC's port.py took from Lombyte, MIT:
 * src/sdk/rpc/sce_sif_call_rpc.c). Object deltas: +0x2158 for the sifrpc
 * helpers (0x11AFE8 -> func_0011D140, 0x11AD70 -> func_0011CEC8,
 * 0x11ABC8 -> func_0011CD20, 0x11B090 -> func_0011D1E8), +0x1FB0 for the
 * semaphore trio (0x118C70/0x118C80/0x118CB0 -> func_0011AC20/30/60); the
 * packet table is 0x13E900 (rac1 0x156900). Nine arguments: a0-a3, t0-t3,
 * then one word at 0(sp), as the sceCdRead call site shows.

 * Measured on 2026-10-15 in src/core/0011D140.c: 490 of the 492 bytes are
 * exact (both mode paths, the beql with rsize = 1 in its delay, the
 * semaphore trio, the completion_mode branch, the -1/-2/-3 tails). The one
 * difference is a store pair: retail stores client+0x0 (packet) before
 * client+0x4 (request_id); this cc1 emits 0x4 before 0x0 whatever the
 * source says. Tried, all with the local `request_id` loaded once at the
 * same spot as retail: statement order [0x0, 0x4]; [0x4, 0x0]; a locals-
 * for-both-values block before either store (func_0011D2F0's lever); a
 * volatile store -- identical output in all four. The direct form
 * `client->request_id = packet->request_id;` is strictly worse (moves the
 * load to after the packet store and trades two more registers). */

typedef signed int s32_1B4C8;

extern char r1_D_00156900[];
extern void *func_0011D140(void *);
extern void func_0011D1E8(void *);
extern s32_1B4C8 func_0011CEC8();
extern s32_1B4C8 func_0011CD20();
extern s32_1B4C8 r1_func_00118C70();
extern void r1_func_00118C80(int);
extern int r1_func_00118CB0(int);

typedef struct {
    s32_1B4C8 packet;          /* 0x00 */
    s32_1B4C8 request_id;      /* 0x04 */
    s32_1B4C8 semaphore_id;    /* 0x08 */
    char pad_0C[0x8];
    s32_1B4C8 server_buffer;   /* 0x14 */
    char pad_18[0x4];
    s32_1B4C8 end_callback;    /* 0x1C */
    s32_1B4C8 end_argument;    /* 0x20 */
    s32_1B4C8 server;          /* 0x24 */
} SifRpcClient_1D620;

typedef struct {
    char pad_00[0x14];
    s32_1B4C8 packet_address;  /* 0x14 */
    s32_1B4C8 request_id;      /* 0x18 */
    s32_1B4C8 client;          /* 0x1C */
    s32_1B4C8 rpc_number;      /* 0x20 */
    s32_1B4C8 send_size;       /* 0x24 */
    s32_1B4C8 receive_address; /* 0x28 */
    s32_1B4C8 receive_size;    /* 0x2C */
    s32_1B4C8 completion_mode; /* 0x30 */
    s32_1B4C8 server;          /* 0x34 */
} SifRpcPacket_1D620;

s32_1B4C8 func_0011D620(SifRpcClient_1D620 *client, s32_1B4C8 rpc_number, s32_1B4C8 mode,
                        s32_1B4C8 send_buf, s32_1B4C8 send_size, s32_1B4C8 recv_buf,
                        s32_1B4C8 recv_size, s32_1B4C8 end_func, s32_1B4C8 end_data) {
    s32_1B4C8 semaphore_parameters[6];
    SifRpcPacket_1D620 *packet;
    s32_1B4C8 semaphore_id;
    s32_1B4C8 skip_cache_writeback;
    s32_1B4C8 request_id;
    s32_1B4C8 allocation_failure_result;

    packet = (SifRpcPacket_1D620 *)func_0011D140(r1_D_00156900);
    allocation_failure_result = -1;
    if (packet == 0) {
        return allocation_failure_result;
    }
    request_id = packet->request_id;
    skip_cache_writeback = mode & 2;
    client->end_argument = end_data;
    client->packet = (s32_1B4C8)packet;
    client->request_id = request_id;
    client->end_callback = end_func;
    packet->rpc_number = rpc_number;
    packet->send_size = send_size;
    packet->receive_address = recv_buf;
    packet->receive_size = recv_size;
    packet->packet_address = (s32_1B4C8)packet;
    packet->server = client->server;
    packet->client = (s32_1B4C8)client;
    if (!skip_cache_writeback) {
        if (send_buf == recv_buf) {
            func_0011CEC8(send_buf, (send_size >= recv_size) ? (send_size) : (recv_size));
        } else {
            if (send_size > 0) {
                func_0011CEC8(send_buf, send_size);
            }
            if (recv_size > 0) {
                func_0011CEC8(recv_buf, recv_size);
            }
        }
    }
    if (mode & 1) {
        if (end_func == 0) {
            packet->completion_mode = 0;
        } else {
            packet->completion_mode = 1;
        }
        client->semaphore_id = -1;
        if (func_0011CD20(0x8000000A, packet, 0x40, send_buf, client->server_buffer, send_size) != 0) {
            return 0;
        } else {
            func_0011D1E8(packet);
            return -2;
        }
    } else {
        semaphore_parameters[1] = 1;
        semaphore_parameters[2] = 0;
        semaphore_id = r1_func_00118C70(semaphore_parameters);
        client->semaphore_id = semaphore_id;
        if (semaphore_id < 0) {
            func_0011D1E8(packet);
            return -3;
        }
        packet->completion_mode = 1;
        if (func_0011CD20(0x8000000A, packet, 0x40, send_buf, client->server_buffer, send_size) == 0) {
            r1_func_00118C80(client->semaphore_id);
            func_0011D1E8(packet);
            return -2;
        }
        r1_func_00118CB0(client->semaphore_id);
        r1_func_00118C80(client->semaphore_id);
        return 0;
    }
}
