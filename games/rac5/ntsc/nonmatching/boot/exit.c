extern void __do_global_dtors(void);
extern void _cleanup(void);
extern void _exit(int status);

void exit(int status) {
    __do_global_dtors();
    _cleanup();
    _exit(status);
}

/* Adapted from public GNU GCC 2.95.3 libgcc2.c; full upstream source: https://github.com/gcc-mirror/gcc/blob/7f236610e99c7a11fb18f056b6446b3c7c40979c/gcc/libgcc2.c
 * FSF copyright, GPL-2.0-or-later terms and the original linking exception follow. */

/* More subroutines needed by GCC output code on some machines.  */
/* Compile this one with gcc.  */
/* Copyright (C) 1989, 92-98, 1999 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* As a special exception, if you link this library with other files,
   some of which are compiled with GCC, to produce an executable,
   this library does not by itself cause the resulting executable
   to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */
