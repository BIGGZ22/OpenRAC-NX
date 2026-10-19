/* fp-bit.c, single precision (GCC 2.9 config/fp-bit.c), adapted without
   headers.  Source: https://github.com/gcc-mirror/gcc/blob/7f236610e99c7a11fb18f056b6446b3c7c40979c/gcc/config/fp-bit.c
   FLOAT build: fractype = USItype. */

typedef float SFtype __attribute__ ((mode (SF)));
typedef float DFtype __attribute__ ((mode (DF)));
typedef unsigned int USItype __attribute__ ((mode (SI)));
typedef unsigned int UDItype __attribute__ ((mode (DI)));
typedef unsigned int UHItype __attribute__ ((mode (HI)));
typedef int SItype __attribute__ ((mode (SI)));

typedef USItype fractype;
typedef UHItype halffractype;
typedef SFtype FLO_type;

#define MAX_SI_INT   ((SItype) ((unsigned) (~0)>>1))
#define MAX_USI_INT  ((USItype) ~0)

#define NGARDS 7L
#define GARDROUND 0x3f
#define GARDMASK  0x7f
#define GARDMSB   0x40
#define EXPBITS 8
#define EXPBIAS 127
#define FRACBITS 23
#define EXPMAX (0xff)
#define QUIET_NAN 0x100000L
#define FRAC_NBITS 32
#define FRACHIGH  0x80000000L
#define FRACHIGH2 0xc0000000L
#define F_D_BITOFF (52+8-(23+7))
#define IMPLICIT_1 (1LL<<(FRACBITS+NGARDS))
#define IMPLICIT_2 (1LL<<(FRACBITS+1+NGARDS))

typedef enum
{
  CLASS_SNAN,
  CLASS_QNAN,
  CLASS_ZERO,
  CLASS_NUMBER,
  CLASS_INFINITY
} fp_class_type;

typedef struct
{
  fp_class_type class;
  unsigned int sign;
  int normal_exp;

  union
    {
      fractype ll;
      halffractype l[2];
    } fraction;
} fp_number_type;

typedef union
{
  FLO_type value;
  fractype value_raw;
  halffractype words[2];
}
FLO_union_type;

__inline__ static int
isnan ( fp_number_type *  x)
{
  return x->class == CLASS_SNAN || x->class == CLASS_QNAN;
}

__inline__ static int
isinf ( fp_number_type *  x)
{
  return x->class == CLASS_INFINITY;
}

__inline__ static int
iszero ( fp_number_type *  x)
{
  return x->class == CLASS_ZERO;
}

void __unpack_f (FLO_union_type *, fp_number_type *);
extern FLO_type __pack_f ( fp_number_type * );
extern DFtype __make_dp (fp_class_type, unsigned int, int, UDItype frac);


DFtype
fptodp (SFtype arg_a)
{
  fp_number_type in;

  __unpack_f ((FLO_union_type *) & arg_a, &in);
  return __make_dp (in.class, in.sign, in.normal_exp,
		    ((UDItype) in.fraction.ll) << F_D_BITOFF);
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
