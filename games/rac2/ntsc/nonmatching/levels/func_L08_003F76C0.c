/* func_L08_003F76C0 (252 bytes, 1 place) - five $gp accesses, needs a -g8 unit; not
 * decoded in detail this wave.
 *
 * Classified by pattern: no INVALID word and no max.s/min.s, four calls, five $gp
 * reads/writes.  The row therefore belongs in the L08_003D8348-g8.c sibling with sized
 * sda declarations - see func_L10_00425888 and func_L00_004282F0 for the measured rules:
 * sda is the only route to GPREL16, plain externs stay absolute even under -G8, and an
 * incomplete `extern u8 D[]` costs a lui.  Nothing else in the row looks like a wall.
 */
