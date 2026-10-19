/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). CRC-16 over bytes data..end (exclusive) using polynomial 0x1F45 and the initial value 0xEDB88320 (only its low 16 bits matter for the result; the 32-bit register is kept as the assembly keeps it). Reads each byte once in order, 8 shift steps per byte, returns crc & 0xFFFF. Empty range returns 0x8320. Same arithmetic as the assembly; unsigned shifts so no undefined behaviour. */
unsigned int func_L00_0024DFC4(unsigned char *data, unsigned char *end) {
    unsigned int crc = 0xEDB88320u;
    unsigned int byte;
    int bit;

    if (data < end) {
        byte = *data;
        do {
            data++;
            crc ^= byte << 8;
            bit = 7;
            do {
                if (crc & 0x8000) {
                    crc = (crc << 1) ^ 0x1F45;
                } else {
                    crc <<= 1;
                }
            } while (--bit >= 0);
            if (!(data < end)) {
                break;
            }
            byte = *data;
        } while (1);
    }
    return crc & 0xFFFF;
}
