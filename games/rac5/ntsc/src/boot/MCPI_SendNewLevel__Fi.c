typedef struct {
    int word0;
    unsigned char byte4;
    unsigned char byte5;
    unsigned char byte6;
    unsigned char byte7;
    int payload[3];
} tHigMsgPacket;

extern tHigMsgPacket hig_msg_newlevel;
extern int MBOX_Send__FiP12HigMsgPacket(int, tHigMsgPacket *);

void MCPI_SendNewLevel__Fi(int level) {
    hig_msg_newlevel.payload[0] = 0x40000000;
    hig_msg_newlevel.byte4 = 0;
    hig_msg_newlevel.byte5 = 0;
    hig_msg_newlevel.byte6 = 0;
    hig_msg_newlevel.byte7 = 0;
    MBOX_Send__FiP12HigMsgPacket(level, &hig_msg_newlevel);
}
