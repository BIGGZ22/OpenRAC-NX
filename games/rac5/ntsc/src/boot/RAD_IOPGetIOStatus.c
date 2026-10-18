/* RAD IOP status pole, retail 0x01EAF5B8 (16 bytes):
 *   lui $v1,%hi(s) ; lw $v0,%lo(s)($v1) ; jr $ra ; nop
 * The load is not moved into the jr delay slot here, unlike __errno.
 */
extern volatile int rac5_iop_io_status;   /* 0x01FD3D28 */

int RAD_IOPGetIOStatus(void) {
    return rac5_iop_io_status;
}
