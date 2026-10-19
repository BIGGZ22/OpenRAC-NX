/* PS2 SDK sceDeci2Open, retail 0x01EDA270 (80 bytes): build the open packet and
 * hand it to Deci2Call(1).
 */
typedef struct Deci2OpenPacket {
    unsigned int proto;                 /* 0x00 */
    void *handler;                      /* 0x04 */
    unsigned int option;                /* 0x08 */
    unsigned int channel;               /* 0x0C */
} Deci2OpenPacket;

extern unsigned int deci2_channel;      /* 0x01FE9BC8 */
extern int Deci2Call(int func, void *arg);

int sceDeci2Open(unsigned short proto, void *handler, int option) {
    Deci2OpenPacket pkt;

    pkt.proto = proto;
    pkt.handler = handler;
    pkt.option = option;
    pkt.channel = (unsigned int)&deci2_channel | 0x20000000;
    return Deci2Call(1, &pkt);
}
