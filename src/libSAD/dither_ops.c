/* Scale & Dither library (libSAD)
 * High-precision bit depth converter with ReplayGain support
 *
 * Copyright (c) 2007-2008 Eugene Zagidullin (e.asphyx@gmail.com)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* WARNING: reading this can damage your brain */

#include "common.h"
#include "dither_ops.h"
#include "dither.h"

#include "debug.h"

#define SAD_GET_LE16(a) ( (uint16_t)(((uint8_t*)(a))[0])      | (uint16_t)(((uint8_t*)(a))[1]) << 8 )
#define SAD_GET_BE16(a) ( (uint16_t)(((uint8_t*)(a))[0]) << 8 | (uint16_t)(((uint8_t*)(a))[1]) )

#define SAD_GET_LE32(a) ( (uint32_t)(((uint8_t*)(a))[0])       | (uint32_t)(((uint8_t*)(a))[1]) << 8 | \
                          (uint32_t)(((uint8_t*)(a))[2]) << 16 | (uint32_t)(((uint8_t*)(a))[3]) << 24 )
#define SAD_GET_BE32(a) ( (uint32_t)(((uint8_t*)(a))[0]) << 24 | (uint32_t)(((uint8_t*)(a))[1]) << 16 | \
                          (uint32_t)(((uint8_t*)(a))[2]) << 8 | (uint32_t)(((uint8_t*)(a))[3]) )

#define SAD_PUT_LE16(a,b) { \
          ((uint8_t*)(a))[0] = (uint8_t)((uint32_t)(b) &  0x000000ff);        \
          ((uint8_t*)(a))[1] = (uint8_t)(((uint32_t)(b) & 0x0000ff00) >> 8); \
        }

#define SAD_PUT_BE16(a,b) { \
          ((uint8_t*)(a))[0] = (uint8_t)(((uint32_t)(b) & 0x0000ff00) >> 8); \
          ((uint8_t*)(a))[1] = (uint8_t)((uint32_t)(b) &  0x000000ff);        \
        }

#define SAD_PUT_LE32(a,b) { \
          ((uint8_t*)(a))[0] = (uint8_t)((uint32_t)(b) &  0x000000ff);        \
          ((uint8_t*)(a))[1] = (uint8_t)(((uint32_t)(b) & 0x0000ff00) >> 8); \
          ((uint8_t*)(a))[2] = (uint8_t)(((uint32_t)(b) & 0x00ff0000) >> 16); \
          ((uint8_t*)(a))[3] = (uint8_t)(((uint32_t)(b) & 0xff000000) >> 24); \
        }

#define SAD_PUT_BE32(a,b) { \
          ((uint8_t*)(a))[0] = (uint8_t)(((uint32_t)(b) & 0xff000000) >> 24); \
          ((uint8_t*)(a))[1] = (uint8_t)(((uint32_t)(b) & 0x00ff0000) >> 16); \
          ((uint8_t*)(a))[2] = (uint8_t)(((uint32_t)(b) & 0x0000ff00) >> 8); \
          ((uint8_t*)(a))[3] = (uint8_t)((uint32_t)(b) &  0x000000ff);       \
        }


/* buffer ops: */
/**************************************************************************************************************** 
 * 8-bit                                                                                                        *
 ****************************************************************************************************************/

/* signed */
static int32_t get_s8_i_sample (void *buf, int nch, int ch, int i) {
  return ((int8_t*)buf)[i*nch+ch];
}

static int32_t get_s8_s_sample (void *buf, int nch, int ch, int i) {
  return ((int8_t**)buf)[ch][i];
}

static void put_s8_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((int8_t*)buf)[i*nch+ch] = (int8_t)sample;
}

static void put_s8_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((int8_t**)buf)[ch][i] = (int8_t)sample;
}

/* unsigned */
static int32_t get_u8_i_sample (void *buf, int nch, int ch, int i) {
  return (int32_t)(((uint8_t*)buf)[i*nch+ch]) - 128;
}

static int32_t get_u8_s_sample (void *buf, int nch, int ch, int i) {
  return (int32_t)(((uint8_t**)buf)[ch][i]) - 128;
}

static void put_u8_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((uint8_t*)buf)[i*nch+ch] = (uint8_t)sample + 128;
}

static void put_u8_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((uint8_t**)buf)[ch][i] = (uint8_t)sample + 128;
}

static SAD_buffer_ops buf_s8_i_ops = {
  &get_s8_i_sample,
  &put_s8_i_sample
};

static SAD_buffer_ops buf_s8_s_ops = {
  &get_s8_s_sample,
  &put_s8_s_sample
};

static SAD_buffer_ops buf_u8_i_ops = {
  &get_u8_i_sample,
  &put_u8_i_sample
};

static SAD_buffer_ops buf_u8_s_ops = {
  &get_u8_s_sample,
  &put_u8_s_sample
};

/**************************************************************************************************************** 
 * 16-bit                                                                                                       *
 ****************************************************************************************************************/

/* signed */
static int32_t get_s16_i_sample (void *buf, int nch, int ch, int i) {
  return (int32_t)(((int16_t*)buf)[i*nch+ch]);
}

static int32_t get_s16_s_sample (void *buf, int nch, int ch, int i) {
  return (int32_t)(((int16_t**)buf)[ch][i]);
}

static void put_s16_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((int16_t*)buf)[i*nch+ch] = (int16_t)sample;
}

static void put_s16_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((int16_t**)buf)[ch][i] = (int16_t)sample;
}

/* unsigned */
static int32_t get_u16_i_sample (void *buf, int nch, int ch, int i) {
  return ((int32_t)(((uint16_t*)buf)[i*nch+ch])) - 32768;
}

static int32_t get_u16_s_sample (void *buf, int nch, int ch, int i) {
  return ((int32_t)(((uint16_t**)buf)[ch][i])) - 32768;
}

static void put_u16_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((uint16_t*)buf)[i*nch+ch] = (uint16_t)(sample + 32768);
}

static void put_u16_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((uint16_t**)buf)[ch][i] = (uint16_t)(sample + 32768);
}

/* LE: signed */
static int32_t get_s16_le_i_sample (void *buf, int nch, int ch, int i) {
  int16_t *tmp = (int16_t*)buf+i*nch+ch;
  return (int16_t)SAD_GET_LE16(tmp);
}

static int32_t get_s16_le_s_sample (void *buf, int nch, int ch, int i) {
  int16_t *tmp = ((int16_t**)buf)[ch]+i;
  return (int16_t)SAD_GET_LE16(tmp);
}

static void put_s16_le_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int16_t *tmp = (int16_t*)buf+i*nch+ch;
  SAD_PUT_LE16(tmp, sample);
}

static void put_s16_le_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int16_t *tmp = ((int16_t**)buf)[ch]+i;
  SAD_PUT_LE16(tmp, sample);
}

/* BE: signed */
static int32_t get_s16_be_i_sample (void *buf, int nch, int ch, int i) {
  int16_t *tmp = (int16_t*)buf+i*nch+ch;
  return (int16_t)SAD_GET_BE16(tmp);
}

static int32_t get_s16_be_s_sample (void *buf, int nch, int ch, int i) {
  int16_t *tmp = ((int16_t**)buf)[ch]+i;
  return (int16_t)SAD_GET_BE16(tmp);
}

static void put_s16_be_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int16_t *tmp = (int16_t*)buf+i*nch+ch;
  SAD_PUT_BE16(tmp, sample);
}

static void put_s16_be_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int16_t *tmp = ((int16_t**)buf)[ch]+i;
  SAD_PUT_BE16(tmp, sample);
}

/* LE: unsigned */
static int32_t get_u16_le_i_sample (void *buf, int nch, int ch, int i) {
  int16_t *tmp = (int16_t*)buf+i*nch+ch;
  return (int16_t)SAD_GET_LE16(tmp) - 32768;
}

static int32_t get_u16_le_s_sample (void *buf, int nch, int ch, int i) {
  int16_t *tmp = ((int16_t**)buf)[ch]+i;
  return (int16_t)SAD_GET_LE16(tmp) - 32768;
}

static void put_u16_le_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int16_t *tmp = (int16_t*)buf+i*nch+ch;
  SAD_PUT_LE16(tmp, sample + 32768);
}

static void put_u16_le_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int16_t *tmp = ((int16_t**)buf)[ch]+i;
  SAD_PUT_LE16(tmp, sample + 32768);
}

/* BE: unsigned */
static int32_t get_u16_be_i_sample (void *buf, int nch, int ch, int i) {
  int16_t *tmp = (int16_t*)buf+i*nch+ch;
  return (int16_t)SAD_GET_BE16(tmp) - 32768;
}

static int32_t get_u16_be_s_sample (void *buf, int nch, int ch, int i) {
  int16_t *tmp = ((int16_t**)buf)[ch]+i;
  return (int16_t)SAD_GET_BE16(tmp) - 32768;
}

static void put_u16_be_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int16_t *tmp = (int16_t*)buf+i*nch+ch;
  SAD_PUT_BE16(tmp, sample + 32768);
}

static void put_u16_be_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int16_t *tmp = ((int16_t**)buf)[ch]+i;
  SAD_PUT_BE16(tmp, sample + 32768);
}


static SAD_buffer_ops buf_s16_i_ops = {
  &get_s16_i_sample,
  &put_s16_i_sample
};

static SAD_buffer_ops buf_s16_s_ops = {
  &get_s16_s_sample,
  &put_s16_s_sample
};

static SAD_buffer_ops buf_s16_le_i_ops = {
  &get_s16_le_i_sample,
  &put_s16_le_i_sample
};

static SAD_buffer_ops buf_s16_le_s_ops = {
  &get_s16_le_s_sample,
  &put_s16_le_s_sample
};

static SAD_buffer_ops buf_s16_be_i_ops = {
  &get_s16_be_i_sample,
  &put_s16_be_i_sample
};

static SAD_buffer_ops buf_s16_be_s_ops = {
  &get_s16_be_s_sample,
  &put_s16_be_s_sample
};

/* unsigned */

static SAD_buffer_ops buf_u16_i_ops = {
  &get_u16_i_sample,
  &put_u16_i_sample
};

static SAD_buffer_ops buf_u16_s_ops = {
  &get_u16_s_sample,
  &put_u16_s_sample
};

static SAD_buffer_ops buf_u16_le_i_ops = {
  &get_u16_le_i_sample,
  &put_u16_le_i_sample
};

static SAD_buffer_ops buf_u16_le_s_ops = {
  &get_u16_le_s_sample,
  &put_u16_le_s_sample
};

static SAD_buffer_ops buf_u16_be_i_ops = {
  &get_u16_be_i_sample,
  &put_u16_be_i_sample
};

static SAD_buffer_ops buf_u16_be_s_ops = {
  &get_u16_be_s_sample,
  &put_u16_be_s_sample
};

/**************************************************************************************************************** 
 * 24-bit                                                                                                       *
 ****************************************************************************************************************/

/*expand 24-bit signed value to 32-bit*/
#define EXPAND_S24_TO_32(x) (((int32_t)(((x) & 0x00ffffff) << 8)) >> 8)
#define EXPAND_U24_TO_32(x) ((int32_t)(x) & 0x00ffffff)

/* signed */
static int32_t get_s24_i_sample (void *buf, int nch, int ch, int i) {
  return (int32_t)EXPAND_S24_TO_32(((int32_t*)buf)[i*nch+ch]);
}

static int32_t get_s24_s_sample (void *buf, int nch, int ch, int i) {
  return (int32_t)EXPAND_S24_TO_32(((int32_t**)buf)[ch][i]);
}

static void put_s24_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((int32_t*)buf)[i*nch+ch] = (int32_t)sample & 0x00ffffff;
}

static void put_s24_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((int32_t**)buf)[ch][i] = (int32_t)sample & 0x00ffffff;
}

/* LE signed */

static int32_t get_s24_le_i_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  return (int32_t)EXPAND_S24_TO_32(SAD_GET_LE32(tmp));
}

static int32_t get_s24_le_s_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  return (int32_t)EXPAND_S24_TO_32(SAD_GET_LE32(tmp));
}

static void put_s24_le_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  SAD_PUT_LE32(tmp, sample & 0x00ffffff);
}

static void put_s24_le_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  SAD_PUT_LE32(tmp, sample & 0x00ffffff);
}

/* BE signed */

static int32_t get_s24_be_i_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  return (int32_t)EXPAND_S24_TO_32(SAD_GET_BE32(tmp));
}

static int32_t get_s24_be_s_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  return (int32_t)EXPAND_S24_TO_32(SAD_GET_BE32(tmp));
}

static void put_s24_be_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  SAD_PUT_BE32(tmp, sample & 0x00ffffff);
}

static void put_s24_be_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  SAD_PUT_BE32(tmp, sample & 0x00ffffff);
}

/* unsigned */
static int32_t get_u24_i_sample (void *buf, int nch, int ch, int i) {
  return (int32_t)EXPAND_U24_TO_32(((uint32_t*)buf)[i*nch+ch]) - 8388608;
}

static int32_t get_u24_s_sample (void *buf, int nch, int ch, int i) {
  return (int32_t)EXPAND_U24_TO_32(((uint32_t**)buf)[ch][i]) - 8388608;
}

static void put_u24_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((uint32_t*)buf)[i*nch+ch] = ((uint32_t)sample + 8388608) & 0x00ffffff;
}

static void put_u24_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((uint32_t**)buf)[ch][i] = ((uint32_t)sample + 8388608) & 0x00ffffff;
}

/* LE unsigned */

static int32_t get_u24_le_i_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  /*fprintf(stderr, "%d\n", (int32_t)EXPAND_U24_TO_32(SAD_GET_LE32(tmp)) - 8388608);*/
  return (int32_t)EXPAND_U24_TO_32(SAD_GET_LE32(tmp)) - 8388608;
}

static int32_t get_u24_le_s_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  return (int32_t)EXPAND_U24_TO_32(SAD_GET_LE32(tmp)) - 8388608;
}

static void put_u24_le_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  SAD_PUT_LE32(tmp, (uint32_t)(sample + 8388608) & 0x00ffffff);
}

static void put_u24_le_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  SAD_PUT_LE32(tmp, (uint32_t)(sample + 8388608) & 0x00ffffff);
}

/* BE unsigned */

static int32_t get_u24_be_i_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  return (int32_t)EXPAND_U24_TO_32(SAD_GET_BE32(tmp)) - 8388608;
}

static int32_t get_u24_be_s_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  return (int32_t)EXPAND_U24_TO_32(SAD_GET_BE32(tmp)) - 8388608;
}

static void put_u24_be_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  SAD_PUT_BE32(tmp, (uint32_t)(sample + 8388608) & 0x00ffffff);
}

static void put_u24_be_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  SAD_PUT_BE32(tmp, (uint32_t)(sample + 8388608) & 0x00ffffff);
}

static SAD_buffer_ops buf_s24_i_ops = {
  &get_s24_i_sample,
  &put_s24_i_sample
};

static SAD_buffer_ops buf_s24_s_ops = {
  &get_s24_s_sample,
  &put_s24_s_sample
};

static SAD_buffer_ops buf_s24_le_i_ops = {
  &get_s24_le_i_sample,
  &put_s24_le_i_sample
};

static SAD_buffer_ops buf_s24_le_s_ops = {
  &get_s24_le_s_sample,
  &put_s24_le_s_sample
};

static SAD_buffer_ops buf_s24_be_i_ops = {
  &get_s24_be_i_sample,
  &put_s24_be_i_sample
};

static SAD_buffer_ops buf_s24_be_s_ops = {
  &get_s24_be_s_sample,
  &put_s24_be_s_sample
};

static SAD_buffer_ops buf_u24_i_ops = {
  &get_u24_i_sample,
  &put_u24_i_sample
};

static SAD_buffer_ops buf_u24_s_ops = {
  &get_u24_s_sample,
  &put_u24_s_sample
};

static SAD_buffer_ops buf_u24_le_i_ops = {
  &get_u24_le_i_sample,
  &put_u24_le_i_sample
};

static SAD_buffer_ops buf_u24_le_s_ops = {
  &get_u24_le_s_sample,
  &put_u24_le_s_sample
};

static SAD_buffer_ops buf_u24_be_i_ops = {
  &get_u24_be_i_sample,
  &put_u24_be_i_sample
};

static SAD_buffer_ops buf_u24_be_s_ops = {
  &get_u24_be_s_sample,
  &put_u24_be_s_sample
};

/**************************************************************************************************************** 
 * 32-bit                                                                                                       *
 ****************************************************************************************************************/

/* signed */
static int32_t get_s32_i_sample (void *buf, int nch, int ch, int i) {
  return ((int32_t*)buf)[i*nch+ch];
}

static int32_t get_s32_s_sample (void *buf, int nch, int ch, int i) {
  return ((int32_t**)buf)[ch][i];
}

static void put_s32_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((int32_t*)buf)[i*nch+ch] = (int32_t)sample;
}

static void put_s32_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((int32_t**)buf)[ch][i] = (int32_t)sample;
}

/* LE: signed */
static int32_t get_s32_le_i_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  return (int32_t)SAD_GET_LE32(tmp);
}

static int32_t get_s32_le_s_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  return (int32_t)SAD_GET_LE32(tmp);
}

static void put_s32_le_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  SAD_PUT_LE32(tmp, sample);
}

static void put_s32_le_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  SAD_PUT_LE32(tmp, sample);
}

/* BE: signed */
static int32_t get_s32_be_i_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  return (int32_t)SAD_GET_BE32(tmp);
}

static int32_t get_s32_be_s_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  return (int32_t)SAD_GET_BE32(tmp);
}

static void put_s32_be_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  SAD_PUT_BE32(tmp, sample);
}

static void put_s32_be_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  SAD_PUT_BE32(tmp, sample);
}

/* unsigned */
static int32_t get_u32_i_sample (void *buf, int nch, int ch, int i) {
  return ((int32_t*)buf)[i*nch+ch] - (int32_t)(1L<<31);
}

static int32_t get_u32_s_sample (void *buf, int nch, int ch, int i) {
  return ((int32_t**)buf)[ch][i] - (int32_t)(1L<<31);
}

static void put_u32_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((uint32_t*)buf)[i*nch+ch] = (uint32_t)(sample + (int32_t)(1L<<31));
}

static void put_u32_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  ((uint32_t**)buf)[ch][i] = (uint32_t)(sample + (int32_t)(1L<<31));
}

/* LE: unsigned */
static int32_t get_u32_le_i_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  return (int32_t)SAD_GET_LE32(tmp) - (int32_t)(1L<<31);
}

static int32_t get_u32_le_s_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  return (int32_t)SAD_GET_LE32(tmp) - (int32_t)(1L<<31);
}

static void put_u32_le_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  SAD_PUT_LE32(tmp, sample + (int32_t)(1L<<31));
}

static void put_u32_le_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  SAD_PUT_LE32(tmp, sample + (int32_t)(1L<<31));
}

/* BE: unsigned */
static int32_t get_u32_be_i_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  return (int32_t)SAD_GET_BE32(tmp) - (int32_t)(1L<<31);
}

static int32_t get_u32_be_s_sample (void *buf, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  return (int32_t)SAD_GET_BE32(tmp) - (int32_t)(1L<<31);
}

static void put_u32_be_i_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = (int32_t*)buf+i*nch+ch;
  SAD_PUT_BE32(tmp, sample + (int32_t)(1L<<31));
}

static void put_u32_be_s_sample (void *buf, int32_t sample, int nch, int ch, int i) {
  int32_t *tmp = ((int32_t**)buf)[ch]+i;
  SAD_PUT_BE32(tmp, sample + (int32_t)(1L<<31));
}

static SAD_buffer_ops buf_s32_i_ops = {
  &get_s32_i_sample,
  &put_s32_i_sample
};

static SAD_buffer_ops buf_s32_s_ops = {
  &get_s32_s_sample,
  &put_s32_s_sample
};

static SAD_buffer_ops buf_s32_le_i_ops = {
  &get_s32_le_i_sample,
  &put_s32_le_i_sample
};

static SAD_buffer_ops buf_s32_le_s_ops = {
  &get_s32_le_s_sample,
  &put_s32_le_s_sample
};

static SAD_buffer_ops buf_s32_be_i_ops = {
  &get_s32_be_i_sample,
  &put_s32_be_i_sample
};

static SAD_buffer_ops buf_s32_be_s_ops = {
  &get_s32_be_s_sample,
  &put_s32_be_s_sample
};

static SAD_buffer_ops buf_u32_i_ops = {
  &get_u32_i_sample,
  &put_u32_i_sample
};

static SAD_buffer_ops buf_u32_s_ops = {
  &get_u32_s_sample,
  &put_u32_s_sample
};

static SAD_buffer_ops buf_u32_le_i_ops = {
  &get_u32_le_i_sample,
  &put_u32_le_i_sample
};

static SAD_buffer_ops buf_u32_le_s_ops = {
  &get_u32_le_s_sample,
  &put_u32_le_s_sample
};

static SAD_buffer_ops buf_u32_be_i_ops = {
  &get_u32_be_i_sample,
  &put_u32_be_i_sample
};

static SAD_buffer_ops buf_u32_be_s_ops = {
  &get_u32_be_s_sample,
  &put_u32_be_s_sample
};

static SAD_buffer_ops *SAD_buffer_optable[SAD_SAMPLE_MAX][SAD_CHORDER_MAX] = {
  {&buf_s8_i_ops,     &buf_s8_s_ops},	  /* SAD_SAMPLE_S8     */
  {&buf_u8_i_ops,     &buf_u8_s_ops},	  /* SAD_SAMPLE_U8     */

  {&buf_s16_i_ops,    &buf_s16_s_ops},	  /* SAD_SAMPLE_S16    */
  {&buf_s16_le_i_ops, &buf_s16_le_s_ops}, /* SAD_SAMPLE_S16_LE */
  {&buf_s16_be_i_ops, &buf_s16_be_s_ops}, /* SAD_SAMPLE_S16_BE */
  {&buf_u16_i_ops,    &buf_u16_s_ops},	  /* SAD_SAMPLE_U16    */
  {&buf_u16_le_i_ops, &buf_u16_le_s_ops}, /* SAD_SAMPLE_U16_LE */
  {&buf_u16_be_i_ops, &buf_u16_be_s_ops}, /* SAD_SAMPLE_U16_BE */

  {&buf_s24_i_ops,    &buf_s24_s_ops},	  /* SAD_SAMPLE_S24    */
  {&buf_s24_le_i_ops, &buf_s24_le_s_ops}, /* SAD_SAMPLE_S24_LE */
  {&buf_s24_be_i_ops, &buf_s24_be_s_ops}, /* SAD_SAMPLE_S24_BE */
  {&buf_u24_i_ops,    &buf_u24_s_ops},	  /* SAD_SAMPLE_U24    */
  {&buf_u24_le_i_ops, &buf_u24_le_s_ops}, /* SAD_SAMPLE_U24_LE */
  {&buf_u24_be_i_ops, &buf_u24_be_s_ops}, /* SAD_SAMPLE_U24_BE */

  {&buf_s32_i_ops,    &buf_s32_s_ops}, 	  /* SAD_SAMPLE_S32    */
  {&buf_s32_le_i_ops, &buf_s32_le_s_ops}, /* SAD_SAMPLE_S32_LE */
  {&buf_s32_be_i_ops, &buf_s32_be_s_ops}, /* SAD_SAMPLE_S32_BE */
  {&buf_u32_i_ops,    &buf_u32_s_ops}, 	  /* SAD_SAMPLE_U32    */
  {&buf_u32_le_i_ops, &buf_u32_le_s_ops}, /* SAD_SAMPLE_U32_LE */
  {&buf_u32_be_i_ops, &buf_u32_be_s_ops}, /* SAD_SAMPLE_U32_BE */

  {&buf_s32_i_ops,    &buf_s32_s_ops}, 	  /* SAD_SAMPLE_FIXED32*/

  {NULL,              NULL}		  /* SAD_SAMPLE_FLOAT  */
};

SAD_buffer_ops* SAD_assign_buf_ops (SAD_buffer_format *format) {
#ifdef DEBUG
  printf("f: SAD_assign_buf_ops\n");
#endif
  if (format->sample_format < SAD_SAMPLE_MAX && format->channels_order < SAD_CHORDER_MAX) {
    return SAD_buffer_optable[format->sample_format][format->channels_order];
  } else {
    return NULL;
  }
}

