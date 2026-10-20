#include "include_asm.h"

/* libgcc single-precision software floating point, EE build - vendored copy.
 *
 * Modified distribution prepared by the RAC2 decompilation campaign on
 * 5 October 2026: upstream fp-bit.c was preprocessed for the EE single-float
 * configuration and one symbol was renamed. The original copyright,
 * GPL version 2-or-later notice, additional permissions, linking exception,
 * warranty disclaimer and author credits are retained below. Only
 * trailing whitespace on added comment lines is normalized.
 * See COPYING in this directory for the GNU GPL version 2 text. The root
 * MIT licence does not relicense this source or its generated copy within
 * candidates/boot.c.
 *
 * Provenance:
 *   upstream   rac1-decomp src/libgcc/fp-bit.c, commit
 *              cb22f0b0d3a171d1fd4b6851b86fe214a22c9822, sha256
 *              3069e3a1385e9b2d316929a676a71e2af8a834666b7388a89f10c52ae0e17336
 *   transform  one pass of the campaign's reconstructed 2.9-ee cpp:
 *              cpp -P -DFLOAT -DFLOAT_BIT_ORDER_MISMATCH -DNO_DENORMALS
 *                  -DUS_SOFTWARE_GOFAST
 *   rename     __unpack_f becomes func_00123400 at 16 occurrences in the
 *              preprocessed body. This is a mechanical, reversible rename;
 *              fptodp and every other body keep their original names.
 *
 * The published body is derived from the immutable two-symbol private trial;
 * it is independently qualified in the generated complete boot unit. The
 * refused fptodp body remains present and receives no matching credit.
 * The body between the markers is unchanged from the published PR13 source
 * at f39ac431876c1e05e88b9a2e28dce855a1e4f9cb. Only notices were restored.
 * See README.md in this directory and docs/RAC1-FP-BIT-EVIDENCE.md.
 */

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

/* --- verbatim preprocessed fp-bit.c begins here --- */
 



























 






 






 


 
















































 

















 












typedef float SFtype __attribute__ ((mode (SF)));

 




typedef unsigned int UHItype __attribute__ ((mode (HI)));

typedef unsigned int USItype __attribute__ ((mode (SI)));

	typedef USItype fractype;

	typedef UHItype halffractype;

	typedef SFtype FLO_type;

 


 
 










 

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




  struct
    {
      fractype fraction: 23  __attribute__ ((packed));
      unsigned int exp: 8  __attribute__ ((packed));
      unsigned int sign:1 __attribute__ ((packed));
    }
  bits;



}
FLO_union_type;

extern void func_00123400  (FLO_union_type *, fp_number_type *);

static __inline__ int fp_pack_isnan(fp_number_type *number) {
    return number->class == CLASS_SNAN || number->class == CLASS_QNAN;
}
static __inline__ int fp_pack_isinf(fp_number_type *number) {
    return number->class == CLASS_INFINITY;
}
static __inline__ int fp_pack_iszero(fp_number_type *number) {
    return number->class == CLASS_ZERO;
}

/* Single-precision packing from the GNU fp-bit.c implementation above. */
FLO_type func_001232F0(fp_number_type *src) {
    FLO_union_type dst;
    fractype fraction = src->fraction.ll;
    int sign = src->sign;
    int exp = 0;

    if (fp_pack_isnan(src)) {
        exp = 255;
        fraction |= 0x100000;
    } else if (fp_pack_isinf(src)) {
        exp = 255;
        fraction = 0;
    } else if (fp_pack_iszero(src)) {
        exp = 0;
        fraction = 0;
    } else if (fraction == 0) {
        exp = 0;
    } else {
        if (src->normal_exp < -126) {
            int shift = -126 - src->normal_exp;
            exp = 0;
            if (shift > 25)
                fraction = 0;
            else
                fraction >>= shift;
            fraction >>= 7;
        } else if (src->normal_exp > 127) {
            exp = 255;
            fraction = 0;
        } else {
            exp = src->normal_exp + 127;
            if ((fraction & 127) == 64) {
                if (fraction & 128)
                    fraction += 64;
            } else {
                fraction += 63;
            }
            if (fraction >= ((fractype)1 << 31)) {
                fraction >>= 1;
                exp += 1;
            }
            fraction >>= 7;
        }
    }
    dst.bits.fraction = fraction;
    dst.bits.exp = exp;
    dst.bits.sign = sign;
    return dst.value;
}

void
func_00123400  (FLO_union_type * src, fp_number_type * dst)
{
   


  fractype fraction;
  int exp;
  int sign;


  

  fraction = src->bits.fraction;
  exp = src->bits.exp;
  sign = src->bits.sign;


  dst->sign = sign;
  if (exp == 0)
    {
       
       



      if (fraction == 0

	  || 1

	  )
	{
	   
	  dst->class = CLASS_ZERO;
	}
      else
	{
	   


	  dst->normal_exp = exp - 127  + 1;
	  fraction <<= 7L ;

	  dst->class = CLASS_NUMBER;

	  while (fraction < (1LL<<(23 + 7L )) )
	    {
	      fraction <<= 1;
	      dst->normal_exp--;
	    }

	  dst->fraction.ll = fraction;
	}
    }
  else if (exp == (0xff) )
    {
       
      if (fraction == 0)
	{
	   
	  dst->class = CLASS_INFINITY;
	}
      else
	{
	   
	  if (fraction & 0x100000L )
	    {
	      dst->class = CLASS_QNAN;
	    }
	  else
	    {
	      dst->class = CLASS_SNAN;
	    }
	   
	  dst->fraction.ll = fraction;
	}
    }
  else
    {
       
      dst->normal_exp = exp - 127 ;
      dst->class = CLASS_NUMBER;
      dst->fraction.ll = (fraction << 7L ) | (1LL<<(23 + 7L )) ;
    }
}

INCLUDE_ASM("asm/core", func_00123490);

extern FLO_type func_001232F0(fp_number_type *);

FLO_type func_001234C0(fp_class_type classification, unsigned int sign, int exponent, fractype fraction) {
    fp_number_type number;
    number.class = classification;
    number.sign = sign;
    number.normal_exp = exponent;
    number.fraction.ll = fraction;
    return func_001232F0(&number);
}

extern double func_00123268(fp_class_type, unsigned int, int, unsigned long);

double func_001234F0(FLO_type value) {
    fp_number_type number;
    FLO_type input = value;
    func_00123400((FLO_union_type *)&input, &number);
    return func_00123268(number.class, number.sign, number.normal_exp,
                         (unsigned long)number.fraction.ll << 30);
}

/* Qwen draft (branch qwen/autonomy, fa99c1ac), checked by tools/check.py */
/*
 * Decodes a variable-length unsigned integer from a byte stream.
 * Each byte contributes 7 bits to the result. The most significant
 * bit (bit 7) indicates continuation: 1 means more bytes follow,
 * 0 means this is the last byte.
 *
 * param_1: pointer to current byte in the input stream
 * param_2: pointer to store the decoded unsigned integer
 * Returns: updated pointer to the next byte after the last decoded byte
 */
unsigned char *func_00123530(unsigned char *param_1, unsigned int *param_2)
{
  unsigned char b;
  unsigned int result;
  unsigned int shift;

  b = *param_1++;
  result = b & 0x7f;
  shift = 0;

  while ((b & 0x80) != 0) {
    b = *param_1++;
    shift += 7;
    result |= (b & 0x7f) << shift;
  }

  *param_2 = result;
  return param_1;
}

unsigned char *func_00123578(unsigned char *input, int *output) {
    unsigned int shift = 0;
    int value = 0;
    unsigned int byte;
    do {
        byte = *input++;
        value |= (byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);
    if (shift < 32 && (byte & 0x40))
        value |= -1 << shift;
    *output = value;
    return input;
}

INCLUDE_ASM("asm/core", func_001236C8);

typedef struct {
    int **records;
    int count;
} RecordOutput;

void func_00123978(int *record, RecordOutput *output, unsigned int *minimum, unsigned int *maximum) {
    unsigned int low = *minimum;
    unsigned int high = *maximum;
    RecordOutput *destination = output;
    if (record[0] != 0) {
        do {
            if (record[1] != 0 && record[2] != 0) {
                unsigned int address;
                unsigned int end;
                destination->records[destination->count++] = record;
                address = record[2];
                end = address + record[3];
                if (address < low)
                    low = address;
                if (high < end)
                    high = end;
            }
            record = (int *)((char *)record + record[0] + 4);
        } while (record[0] != 0);
    }
    *minimum = low;
    *maximum = high;
}


INCLUDE_ASM("asm/core", func_00123B40);

/* libgcc.a frame.o: extract_cie_info, verbatim from the gcc/frame.c of
   gnu-ee-binutils-gcc-1.1 - the same public source the qualified compiler is
   built from. The CIE is reached back through f->CIE_delta, and the unaligned
   pointer read is read_pointer's packed union, which is why retail shows an
   lwl/lwr pair rather than a plain lw. */

typedef struct {
    unsigned int length;
    int CIE_delta;
    void *pc_begin;
    unsigned int pc_range;
} fde;

struct dwarf_cie {
    unsigned int length;
    int CIE_id;
    unsigned char version;
    char augmentation[1];
};

struct cie_info {
    char *augmentation;
    void *eh_ptr;
    int code_align;
    int data_align;
    unsigned int ra_regno;
};

union unaligned {
    void *p;
    unsigned int b4 __attribute__ ((mode (SI)));
} __attribute__ ((packed));

static __inline__ void *read_pointer(void *p) {
    union unaligned *up = p;
    return up->p;
}

static __inline__ struct dwarf_cie *get_cie(fde *f) {
    return (struct dwarf_cie *)((void *)&f->CIE_delta - f->CIE_delta);
}

extern int func_00115544(const char *, const char *);
extern unsigned int func_001157AC(const char *);


INCLUDE_ASM("asm/core", func_00123D30);

INCLUDE_ASM("asm/core", func_001240C8);

ASM_FUNC("asm/core", func_00124414);

ASM_FUNC("asm/core", func_001244B8);

ASM_FUNC("asm/core", func_00124540);
