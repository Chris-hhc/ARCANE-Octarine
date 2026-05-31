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
#define RRLWR_MAX_SEED_LEN        (128) // Support seed lengths up to 128 bytes, only required to define buffer size
#define RRLWR_MAX_OUTLEN          ((RRLWR_MAX_SAMPLING_BITLEN * RRLWR_N * RRLWR_K / 4) / 8)
#define RRLWR_XOF_BUFLEN          (((RRLWR_MAX_OUTLEN + SHAKE128_RATE - 1) / SHAKE128_RATE) * SHAKE128_RATE)

static void set_seed_nonce(uint8_t *out,
                           const unsigned char *seed,
                           int32_t seed_len,
                           unsigned char nonce)
{
  memcpy(out, seed, (size_t)seed_len);
  out[seed_len] = nonce;
}

static void poly_uniform_kx(poly **r,
                            unsigned int npolys,
                            int32_t bitlen,
                            const unsigned char *seed,
                            int32_t seed_len,
                            unsigned char nonce0,
                            unsigned char nonce1,
                            unsigned char nonce2,
                            unsigned char nonce3)
{
  size_t outlen = (size_t)bitlen * (RRLWR_N >> 3);
  size_t nblocks = (((outlen >> 2) * npolys) + SHAKE128_RATE - 1) / SHAKE128_RATE;
  size_t lane_len = nblocks * SHAKE128_RATE;
  uint8_t buf[4 * RRLWR_XOF_BUFLEN];
  uint8_t in0[RRLWR_MAX_SEED_LEN + 1];
  uint8_t in1[RRLWR_MAX_SEED_LEN + 1];
  uint8_t in2[RRLWR_MAX_SEED_LEN + 1];
  uint8_t in3[RRLWR_MAX_SEED_LEN + 1];

  set_seed_nonce(in0, seed, seed_len, nonce0);
  set_seed_nonce(in1, seed, seed_len, nonce1);
  set_seed_nonce(in2, seed, seed_len, nonce2);
  set_seed_nonce(in3, seed, seed_len, nonce3);

  #ifdef RRLWR_SIGN_BREAKDOWN
  uint64_t ts = cpucycles();
  #endif
  shake128x4(buf + 0 * lane_len,
             buf + 1 * lane_len,
             buf + 2 * lane_len,
             buf + 3 * lane_len,
             lane_len,
             in0, in1, in2, in3,
             (size_t)seed_len + 1);
  #ifdef RRLWR_SIGN_BREAKDOWN
  if(sign_test_measure_awin_base) {
    sign_test_add_cycles(SIGN_TEST_KEYGEN_AWIN_BASE_SHAKE128X4, ts);
  }
  ts = cpucycles();
  #endif

  for(unsigned int i = 0; i < npolys; i++) {
    poly_unpack(r[i], buf + i * outlen, bitlen);
  }
  #ifdef RRLWR_SIGN_BREAKDOWN
  if(sign_test_measure_awin_base) {
    sign_test_add_cycles(SIGN_TEST_KEYGEN_AWIN_BASE_UNPACK, ts);
  }
  #endif
}

/// @brief Generate a polynomial with coefficients pseudo-randomly generated in the uniform distribution [-bitlen/2, bitlen/2-1]
///        The seed (of length seed_len) is appended with coeff that is used as ring coefficient index.
void poly_uniform(poly *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len, unsigned char coeff) {
  poly *polys[1] = {r};

  poly_uniform_kx(polys, 1, bitlen, seed, seed_len,
                  (unsigned char)(4 * coeff),
                  (unsigned char)(4 * coeff + 1),
                  (unsigned char)(4 * coeff + 2),
                  (unsigned char)(4 * coeff + 3));
}

void ring_uniform_from_nonce(ring_element *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len, unsigned int nonce) {
  poly *polys[RRLWR_K];

  for(unsigned int i = 0; i < RRLWR_K; i++) {
    polys[i] = &r->x[i];
  }

  poly_uniform_kx(polys, RRLWR_K, bitlen, seed, seed_len,
                  (unsigned char)(nonce + 0),
                  (unsigned char)(nonce + 1),
                  (unsigned char)(nonce + 2),
                  (unsigned char)(nonce + 3));
}

/// @brief Generate a ring element with coefficients pseudo-randomly generated in the uniform distribution [-bitlen/2, bitlen/2-1]
void ring_uniform(ring_element *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len) {
  ring_uniform_from_nonce(r, bitlen, seed, seed_len, 0);
}

void ring_uniform_Awin_base(ring_element_Awin *aw, int32_t bitlen, const unsigned char *seed, int32_t seed_len) {
  poly *polys[RRLWR_K];

  for(unsigned int i = 0; i < RRLWR_K; i++) {
    polys[i] = &aw->x[RRLWR_K - 1 - i];
  }

  poly_uniform_kx(polys, RRLWR_K, bitlen, seed, seed_len, 0, 1, 2, 3);
}
