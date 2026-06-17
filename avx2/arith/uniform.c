/* 
 * Copyright 2026 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "uniform.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef RRLWR_SIGN_BREAKDOWN
#include "sign_test.h"
#include "test/cpucycles.h"
#endif

#define RRLWR_MAX_SAMPLING_BITLEN (24)  // Support sampling bit lengths up to 24 bits, only required to define buffer size
#define RRLWR_MAX_SEED_LEN        (129) // Support seed lengths up to 129 bytes, only required to define buffer size
#define RRLWR_MAX_POLY_OUTLEN     ((RRLWR_MAX_SAMPLING_BITLEN * RRLWR_N) >> 3)
#define RRLWR_MAX_QUARTER_OUTLEN  ((RRLWR_MAX_SAMPLING_BITLEN * RRLWR_N) >> 5)

static void set_seed_coeff_lane(uint8_t *out,
                                const unsigned char *seed,
                                int32_t seed_len,
                                unsigned char coeff,
                                unsigned char lane)
{
  memcpy(out, seed, (size_t)seed_len);
  out[seed_len] = coeff;
  out[seed_len + 1] = lane;
}

static void poly_uniform_public_x4(poly *r,
                                   int32_t bitlen,
                                   const unsigned char *seed,
                                   int32_t seed_len,
                                   unsigned char coeff)
{
  const size_t outlen = (size_t)bitlen * (RRLWR_N >> 5);
  uint8_t buf[4 * RRLWR_MAX_QUARTER_OUTLEN];
  uint8_t in0[RRLWR_MAX_SEED_LEN + 2];
  uint8_t in1[RRLWR_MAX_SEED_LEN + 2];
  uint8_t in2[RRLWR_MAX_SEED_LEN + 2];
  uint8_t in3[RRLWR_MAX_SEED_LEN + 2];

  set_seed_coeff_lane(in0, seed, seed_len, coeff, 0);
  set_seed_coeff_lane(in1, seed, seed_len, coeff, 1);
  set_seed_coeff_lane(in2, seed, seed_len, coeff, 2);
  set_seed_coeff_lane(in3, seed, seed_len, coeff, 3);

  #ifdef RRLWR_SIGN_BREAKDOWN
  uint64_t ts = cpucycles();
  #endif
  shake128x4(buf + 0 * outlen,
             buf + 1 * outlen,
             buf + 2 * outlen,
             buf + 3 * outlen,
             outlen,
             in0, in1, in2, in3,
             (size_t)seed_len + 2);
  #ifdef RRLWR_SIGN_BREAKDOWN
  if(sign_test_measure_awin_base) {
    sign_test_add_cycles(SIGN_TEST_KEYGEN_AWIN_BASE_SHAKE128X4, ts);
  }
  ts = cpucycles();
  #endif

  poly_unpack(r, buf, bitlen);

  #ifdef RRLWR_SIGN_BREAKDOWN
  if(sign_test_measure_awin_base) {
    sign_test_add_cycles(SIGN_TEST_KEYGEN_AWIN_BASE_UNPACK, ts);
  }
  #endif
}

void ring_uniform_public_x4(ring_element *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len)
{
  for(unsigned char i = 0; i < RRLWR_K; i++) {
    poly_uniform_public_x4(&r->x[i], bitlen, seed, seed_len, i);
  }
}

static void poly_uniform_secret_x4(poly *r,
                                   int32_t bitlen,
                                   const unsigned char *seed,
                                   int32_t seed_len,
                                   unsigned char coeff)
{
  const size_t outlen = (size_t)bitlen * (RRLWR_N >> 5);
  uint8_t buf[4 * RRLWR_MAX_QUARTER_OUTLEN];
  uint8_t in0[RRLWR_MAX_SEED_LEN + 2];
  uint8_t in1[RRLWR_MAX_SEED_LEN + 2];
  uint8_t in2[RRLWR_MAX_SEED_LEN + 2];
  uint8_t in3[RRLWR_MAX_SEED_LEN + 2];

  set_seed_coeff_lane(in0, seed, seed_len, coeff, 0);
  set_seed_coeff_lane(in1, seed, seed_len, coeff, 1);
  set_seed_coeff_lane(in2, seed, seed_len, coeff, 2);
  set_seed_coeff_lane(in3, seed, seed_len, coeff, 3);

  shake256x4(buf + 0 * outlen,
             buf + 1 * outlen,
             buf + 2 * outlen,
             buf + 3 * outlen,
             outlen,
             in0, in1, in2, in3,
             (size_t)seed_len + 2);

  poly_unpack(r, buf, bitlen);
}

void ring_uniform_secret_x4(ring_element *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len)
{
  for(unsigned char i = 0; i < RRLWR_K; i++) {
    poly_uniform_secret_x4(&r->x[i], bitlen, seed, seed_len, i);
  }
}

static void poly_uniform_secret(poly *r,
                                int32_t bitlen,
                                const unsigned char *seed,
                                int32_t seed_len,
                                unsigned char coeff)
{
  unsigned char xof_bytes_buffer[RRLWR_MAX_POLY_OUTLEN];
  unsigned char seed_buffer[RRLWR_MAX_SEED_LEN + 2];

  for(int32_t i = 0; i < seed_len; i++) {
    seed_buffer[i] = seed[i];
  }
  seed_buffer[seed_len] = coeff;
  seed_buffer[seed_len + 1] = 0;

  RRLWR_XOF_SECRET(xof_bytes_buffer, bitlen * (RRLWR_N >> 3), seed_buffer, seed_len + 2);
  poly_unpack(r, xof_bytes_buffer, bitlen);
}

void ring_uniform_secret(ring_element *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len)
{
  for(unsigned char i = 0; i < RRLWR_K; i++) {
    poly_uniform_secret(&r->x[i], bitlen, seed, seed_len, i);
  }
}

void ring_uniform_Awin_base(ring_element_Awin *aw, int32_t bitlen, const unsigned char *seed, int32_t seed_len)
{
  for(unsigned char i = 0; i < RRLWR_K; i++) {
    poly_uniform_public_x4(&aw->x[RRLWR_K - 1 - i], bitlen, seed, seed_len, i);
  }
}
