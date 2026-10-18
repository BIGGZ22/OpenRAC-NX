/* PS2 SDK sceDeci2ExSend, retail 0x01EDA358 (72 bytes): build the extended send
 * packet and hand it to Deci2Call(-6).
 */
typedef struct Deci2SendPacket {
    unsigned int dest;                  /* 0x00 */
    void *buffer;                       /* 0x04 */
    unsigned int length;                /* 0x08 */
} Deci2SendPacket;

extern int Deci2Call(int func, void *arg);

int sceDeci2ExSend(unsigned int dest, void *buffer, unsigned short length) {
    Deci2SendPacket pkt;

    pkt.dest = dest;
    pkt.buffer = buffer;
    pkt.length = length;
    return Deci2Call(-6, &pkt);
}
