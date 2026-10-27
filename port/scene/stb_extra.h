/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * What viewer/third_party.c compiles of stb that its headers do not declare. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* stb_image_write's zlib compressor, the one its PNGs use: a zlib stream (RFC 1950) of `data`,
 * `quality` 5 and up (more is slower and smaller), its length in `out_len`; allocated with
 * malloc, null when it could not. */
unsigned char* stbi_zlib_compress(unsigned char* data, int data_len, int* out_len, int quality);

#ifdef __cplusplus
}
#endif
