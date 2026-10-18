/* Bink RAD IOP entry, retail 0x01EB27C8 (24 bytes):
 *   lui $v1,%hi(handle) ; lui $v0,%hi(entry) ; sw $a0,%lo(handle)($v1) ;
 *   jr $ra ; addiu $v0,$v0,%lo(entry)
 * Stores the opening handle and hands back the IOP side entry address.
 */
extern int rac5_bink_open_handle;          /* 0x01FD4160 */
extern char rac5_bink_iop_entry[];         /* 0x01EB2920 */

int BinkOpenRAD_IOP(int handle) {
    rac5_bink_open_handle = handle;
    return (int)rac5_bink_iop_entry;
}
