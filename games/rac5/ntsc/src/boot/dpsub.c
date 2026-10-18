/* PARKED (w2-agent-04): retail saves callee-saved GPRs with a 16-byte slot
   stride (s0@0x0, ra@0x10, ...) which the pinned chain emits as 8-byte slots
   (mcc/mips.c:4723, RAC0/RAC2 TImode anchor neutralised). Body is otherwise
   identical; the frame offsets are not reachable from C. Reopen if the
   compiler profile gains that anchor. */
/* dpsub 0x01E96458..0x01E964C0 */
typedef struct { int class; int sign; int normal_exp; int pad; long long fraction; int pad2; } fp_number_type;
extern void __unpack_d(double *a, fp_number_type *x);
extern fp_number_type *__add_d(fp_number_type *x, fp_number_type *y, fp_number_type *z);
extern double __pack_d(fp_number_type *z);
double dpsub(double a, double b) {
    fp_number_type x, y, z;
    fp_number_type *r;
    __unpack_d(&a, &x);
    __unpack_d(&b, &y);
    y.sign ^= 1;
    r = __add_d(&x, &y, &z);
    return __pack_d(r);
}

/* Adapted from public GNU GCC 2.95.3 fp-bit.c; full upstream source: https://github.com/gcc-mirror/gcc/blob/7f236610e99c7a11fb18f056b6446b3c7c40979c/gcc/config/fp-bit.c
 * FSF copyright, GPL-2.0-or-later terms and the original linking exception follow. */

/* This is a software floating point library which can be used instead of
   the floating point routines in libgcc1.c for targets without hardware
   floating point.
 Copyright (C) 1994, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.

This file is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file with other programs, and to distribute
those programs without any restriction coming from the use of this
file.  (The General Public License restrictions do apply in other
respects; for example, they cover modification of the file, and
distribution when not linked into another program.)

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* As a special exception, if you link this library with other files,
   some of which are compiled with GCC, to produce an executable,
   this library does not by itself cause the resulting executable
   to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */

/* This implements IEEE 754 format arithmetic, but does not provide a
   mechanism for setting the rounding mode, or for generating or handling
   exceptions.

   The original code by Steve Chamberlain, hacked by Mark Eichin and Jim
   Wilson, all of Cygnus Support.  */
