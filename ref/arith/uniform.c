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

/// @brief Generate a polynomial with coefficients pseudo-randomly generated in the uniform distribution [-bitlen/2, bitlen/2-1]
///        The seed (of length seed_len) is appended with coeff that is used as ring coefficient index.
#define RRLWR_MAX_SAMPLING_BITLEN (24)  // Support sampling bit lengths up to 24 bits, only required to define buffer size
#define RRLWR_MAX_SEED_LEN        (128) // Support seed lengths up to 128 bytes, only required to define buffer size
#define RRLWR_MAX_OUTLEN          ((RRLWR_MAX_SAMPLING_BITLEN * RRLWR_N * RRLWR_K / 4) / 8)
#define RRLWR_XOF_BUFLEN          (((RRLWR_MAX_OUTLEN + SHAKE128_RATE - 1) / SHAKE128_RATE) * SHAKE128_RATE)

static void set_seed_nonce(unsigned char *out,
                           const unsigned char *seed,
                           int32_t seed_len,
                           unsigned char nonce)
{
  memcpy(out, seed, (size_t)seed_len);
  out[seed_len] = nonce;
}

static void squeeze_seed_nonce(unsigned char *out,
                               size_t nblocks,
                               const unsigned char *seed,
                               int32_t seed_len,
                               unsigned char nonce)
{
  unsigned char seed_buffer[RRLWR_MAX_SEED_LEN + 1];
  keccak_state state;

  set_seed_nonce(seed_buffer, seed, seed_len, nonce);
  shake128_absorb_once(&state, seed_buffer, (size_t)seed_len + 1);
  shake128_squeezeblocks(out, nblocks, &state);
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
  unsigned char buf[4 * RRLWR_XOF_BUFLEN];

  squeeze_seed_nonce(buf + 0 * lane_len, nblocks, seed, seed_len, nonce0);
  squeeze_seed_nonce(buf + 1 * lane_len, nblocks, seed, seed_len, nonce1);
  squeeze_seed_nonce(buf + 2 * lane_len, nblocks, seed, seed_len, nonce2);
  squeeze_seed_nonce(buf + 3 * lane_len, nblocks, seed, seed_len, nonce3);

  for(unsigned int i = 0; i < npolys; i++) {
    poly_unpack(r[i], buf + i * outlen, bitlen);
  }
}

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
