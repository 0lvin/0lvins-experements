format_list = [
    (4, 4),
    (5, 4),
    (5, 5),
    (6, 5),
    (6, 6),
    (8, 5),
    (8, 6),
    (8, 8),
    (10, 5),
    (10, 6),
    (10, 8),
    (10, 10),
    (12, 10),
    (12, 12),
]

print(
    """
/**************************************************************************
 *
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
 * Copyright (c) 2008 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "u_math.h"
#include "u_format.h"
#include "u_format_astc.h"
#include "util/format_srgb.h"
#include <stdio.h>

// 128 bit
#define BLOCK_BYTES 16

static void
decompress_rgb_float_block(unsigned src_width, unsigned src_height,
                           const uint8_t *block,
                           float *dst_row, unsigned dst_rowstride,
                           int x_block_size, int y_block_size, bool is_srgb)
{
   unsigned x, y;

   if (1) {
      for(y = 0; y < src_height; y += 1) {
         float *result = dst_row;
         for(x = 0; x < src_width; x += 1) {
            result[0] = 1.0f;
            result[1] = 0.0f;
            result[2] = 1.0f;
            result[3] = 1.0f;
            result += 4;
         }
         dst_row += dst_rowstride / sizeof dst_row[0];
      }
      return;
   }
}

static void
fetch_rgb_float_from_block(const uint8_t *block,
                           float *result,
                           int texel,
                           int x_block_size, int y_block_size, bool is_srgb)
{
   printf("Called: %s %dx%d - %d\\n", __func__, x_block_size, y_block_size, is_srgb);
}

static void
decompress_rgb_float(int width, int height,
                      const uint8_t *src, int src_rowstride,
                      float *dst, int dst_rowstride,
                      int x_block_size, int y_block_size, bool is_srgb)
{
   int src_row_diff;
   int y, x;

   printf("Called: %s %dx%d - %d\\n", __func__, x_block_size, y_block_size, is_srgb);

   if (src_rowstride >= width * 4)
      src_row_diff = src_rowstride - ((width + 3) & ~3) * 4;
   else
      src_row_diff = 0;

   for (y = 0; y < height; y += y_block_size) {
      for (x = 0; x < width; x += x_block_size) {
         decompress_rgb_float_block(MIN2(width - x, x_block_size),
                                    MIN2(height - y, y_block_size),
                                    src,
                                    (dst + x * 4 +
                                     (y * dst_rowstride / sizeof dst[0])),
                                    dst_rowstride, x_block_size, y_block_size,
                                    is_srgb);
         src += BLOCK_BYTES;
      }
      src += src_row_diff;
   }
}

static void
compress_rgb_float_block(int src_width, int src_height,
                         const float *src, int src_rowstride,
                         uint8_t *dst,
                         int x_block_size, int y_block_size, bool is_srgb)
{
   // Not implemented
}

static void
compress_rgb_float(int width, int height,
                   const float *src, int src_rowstride,
                   uint8_t *dst, int dst_rowstride,
                   int x_block_size, int y_block_size, bool is_srgb)
{
   int dst_row_diff;
   int y, x;

   printf("Called: %s %dx%d - %d\\n", __func__, x_block_size, y_block_size, is_srgb);

   if (src_rowstride >= width * 4)
      dst_row_diff = dst_rowstride - ((width + 3) & ~3) * 4;
   else
      dst_row_diff = 0;

   for (y = 0; y < height; y += y_block_size) {
      for (x = 0; x < width; x += x_block_size) {
         compress_rgb_float_block(MIN2(width - x, x_block_size),
                                  MIN2(height - y, y_block_size),
                                  src + x * 3 +
                                  y * src_rowstride / sizeof (float),
                                  src_rowstride,
                                  dst,
                                  x_block_size, y_block_size, is_srgb);
         dst += BLOCK_BYTES;
      }
      dst += dst_row_diff;
   }
}

"""
)

for format_x, format_y in format_list:
    print(
        """
void
util_format_astc_{format_x}x{format_y}_unpack_rgba_8unorm(uint8_t *dst_row, unsigned dst_stride,
                                               const uint8_t *src_row, unsigned src_stride,
                                               unsigned width, unsigned height)
{{
   float *temp_block;
   temp_block = malloc(width * height * 4 * sizeof(float));
   decompress_rgb_float(width, height,
                        src_row, src_stride,
                        temp_block, width * 4 * sizeof(float),
                        {format_x}, {format_y}, false);
   util_format_read_4ub(PIPE_FORMAT_R32G32B32A32_FLOAT,
                        dst_row, dst_stride,
                        temp_block, width * 4 * sizeof(float),
                        0, 0, width, height);
   free((void *) temp_block);
}}

void
util_format_astc_{format_x}x{format_y}_pack_rgba_8unorm(uint8_t *dst_row, unsigned dst_stride,
                                             const uint8_t *src_row, unsigned src_stride,
                                             unsigned width, unsigned height)
{{
   float *temp_block;
   temp_block = malloc(width * height * 4 * sizeof(float));
   util_format_read_4f(PIPE_FORMAT_R8G8B8A8_SRGB,
                        temp_block, width * 4 * sizeof(float),
                        src_row, src_stride,
                        0, 0, width, height);
   compress_rgb_float(width, height,
                       temp_block, width * 4 * sizeof(float),
                       dst_row, dst_stride,
                       {format_x}, {format_y}, false);
   free((void *) temp_block);
}}

void
util_format_astc_{format_x}x{format_y}_unpack_rgba_float(float *dst_row, unsigned dst_stride,
                                              const uint8_t *src_row, unsigned src_stride,
                                              unsigned width, unsigned height)
{{
   decompress_rgb_float(width, height,
                        src_row, src_stride,
                        dst_row, dst_stride,
                        {format_x}, {format_y}, false);
}}

void
util_format_astc_{format_x}x{format_y}_pack_rgba_float(uint8_t *dst_row, unsigned dst_stride,
                                            const float *src_row, unsigned src_stride,
                                            unsigned width, unsigned height)
{{
   compress_rgb_float(width, height,
                      src_row, src_stride,
                      dst_row, dst_stride,
                      {format_x}, {format_y}, false);
}}

void
util_format_astc_{format_x}x{format_y}_fetch_rgba_float(float *dst, const uint8_t *src,
                                             unsigned width, unsigned height)
{{
   fetch_rgb_float_from_block(src + ((width * sizeof(uint8_t)) * (height / {format_x}) + (width / {format_y})) * BLOCK_BYTES,
                              dst, (width % {format_y}) + (height % {format_x}) * 4, {format_x}, {format_y}, false);
}}

void
util_format_astc_{format_x}x{format_y}_srgb_unpack_rgba_8unorm(uint8_t *dst_row, unsigned dst_stride,
                                               const uint8_t *src_row, unsigned src_stride,
                                               unsigned width, unsigned height)
{{
   float *temp_block;
   temp_block = malloc(width * height * 4 * sizeof(float));
   decompress_rgb_float(width, height,
                        src_row, src_stride,
                        temp_block, width * 4 * sizeof(float),
                        {format_x}, {format_y}, true);
   util_format_read_4ub(PIPE_FORMAT_R32G32B32A32_FLOAT,
                        dst_row, dst_stride,
                        temp_block, width * 4 * sizeof(float),
                        0, 0, width, height);
   free((void *) temp_block);
}}

void
util_format_astc_{format_x}x{format_y}_srgb_pack_rgba_8unorm(uint8_t *dst_row, unsigned dst_stride,
                                             const uint8_t *src_row, unsigned src_stride,
                                             unsigned width, unsigned height)
{{
   float *temp_block;
   temp_block = malloc(width * height * 4 * sizeof(float));
   util_format_read_4f(PIPE_FORMAT_R8G8B8A8_SRGB,
                        temp_block, width * 4 * sizeof(float),
                        src_row, src_stride,
                        0, 0, width, height);
   compress_rgb_float(width, height,
                       temp_block, width * 4 * sizeof(float),
                       dst_row, dst_stride,
                       {format_x}, {format_y}, true);
   free((void *) temp_block);
}}

void
util_format_astc_{format_x}x{format_y}_srgb_unpack_rgba_float(float *dst_row, unsigned dst_stride,
                                              const uint8_t *src_row, unsigned src_stride,
                                              unsigned width, unsigned height)
{{
   decompress_rgb_float(width, height,
                        src_row, src_stride,
                        dst_row, dst_stride,
                        {format_x}, {format_y}, true);
}}

void
util_format_astc_{format_x}x{format_y}_srgb_pack_rgba_float(uint8_t *dst_row, unsigned dst_stride,
                                            const float *src_row, unsigned src_stride,
                                            unsigned width, unsigned height)
{{
   compress_rgb_float(width, height,
                      src_row, src_stride,
                      dst_row, dst_stride,
                      {format_x}, {format_y}, true);
}}

void
util_format_astc_{format_x}x{format_y}_srgb_fetch_rgba_float(float *dst, const uint8_t *src,
                                             unsigned width, unsigned height)
{{
   fetch_rgb_float_from_block(src + ((width * sizeof(uint8_t)) * (height / {format_x}) + (width / {format_y})) * BLOCK_BYTES,
                              dst, (width % {format_y}) + (height % {format_x}) * 4, {format_x}, {format_y}, true);
}}

""".format(
            format_x=format_x, format_y=format_y
        )
    )
