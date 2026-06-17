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
#define RRLWR_MAX_SEED_LEN        (129) // Support seed lengths up to 128 bytes, only required to define buffer size

// Uniform polynomial generation with 4 independent byte streams
static void poly_uniform_public_x4(poly *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len, unsigned char coeff)
{
  unsigned char xof_bytes_buffer0[RRLWR_MAX_SAMPLING_BITLEN*(RRLWR_N >> 3)]; // Allocate maximum length
  unsigned char xof_bytes_buffer1[RRLWR_MAX_SAMPLING_BITLEN*(RRLWR_N >> 3)]; // Allocate maximum length
  unsigned char xof_bytes_buffer2[RRLWR_MAX_SAMPLING_BITLEN*(RRLWR_N >> 3)]; // Allocate maximum length
  unsigned char xof_bytes_buffer3[RRLWR_MAX_SAMPLING_BITLEN*(RRLWR_N >> 3)]; // Allocate maximum length
  unsigned char seed_buffer0[RRLWR_MAX_SEED_LEN+2];                          // Allocate maximum length
  unsigned char seed_buffer1[RRLWR_MAX_SEED_LEN+2];                          // Allocate maximum length
  unsigned char seed_buffer2[RRLWR_MAX_SEED_LEN+2];                          // Allocate maximum length
  unsigned char seed_buffer3[RRLWR_MAX_SEED_LEN+2];                          // Allocate maximum length

  // Concatenate the seed with the ring element index i
  for(int32_t i = 0; i < seed_len; i++) {
    seed_buffer0[i] = seed[i];
    seed_buffer1[i] = seed[i];
    seed_buffer2[i] = seed[i];
    seed_buffer3[i] = seed[i];
  }
  seed_buffer0[seed_len] = coeff;
  seed_buffer1[seed_len] = coeff;
  seed_buffer2[seed_len] = coeff;
  seed_buffer3[seed_len] = coeff;
  seed_buffer0[seed_len+1] = 0;
  seed_buffer1[seed_len+1] = 1;
  seed_buffer2[seed_len+1] = 2;
  seed_buffer3[seed_len+1] = 3;

  // Generate the output byte stream
  RRLWR_XOF_PUBLIC(xof_bytes_buffer0, bitlen*(RRLWR_N >> 5), seed_buffer0, seed_len+2);
  RRLWR_XOF_PUBLIC(xof_bytes_buffer1, bitlen*(RRLWR_N >> 5), seed_buffer1, seed_len+2);
  RRLWR_XOF_PUBLIC(xof_bytes_buffer2, bitlen*(RRLWR_N >> 5), seed_buffer2, seed_len+2);
  RRLWR_XOF_PUBLIC(xof_bytes_buffer3, bitlen*(RRLWR_N >> 5), seed_buffer3, seed_len+2);

  // Unpack into polynomial coefficients
  subpoly_unpack(&r->coeffs[0*(RRLWR_N>>2)], RRLWR_N>>2, xof_bytes_buffer0, bitlen);
  subpoly_unpack(&r->coeffs[1*(RRLWR_N>>2)], RRLWR_N>>2, xof_bytes_buffer1, bitlen);
  subpoly_unpack(&r->coeffs[2*(RRLWR_N>>2)], RRLWR_N>>2, xof_bytes_buffer2, bitlen);
  subpoly_unpack(&r->coeffs[3*(RRLWR_N>>2)], RRLWR_N>>2, xof_bytes_buffer3, bitlen);
}

/// @brief Generate a ring element with coefficients pseudo-randomly generated in the uniform distribution [-bitlen/2, bitlen/2-1]
void ring_uniform_public_x4(ring_element *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len) {
  for(unsigned char i = 0; i < RRLWR_K; i++) {
    poly_uniform_public_x4(&r->x[i], bitlen, seed, seed_len, i);
  }
}

// Uniform polynomial generation with 4 independent byte streams
static void poly_uniform_secret_x4(poly *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len, unsigned char coeff)
{
  unsigned char xof_bytes_buffer0[RRLWR_MAX_SAMPLING_BITLEN*(RRLWR_N >> 5)]; // Allocate maximum length
  unsigned char xof_bytes_buffer1[RRLWR_MAX_SAMPLING_BITLEN*(RRLWR_N >> 5)]; // Allocate maximum length
  unsigned char xof_bytes_buffer2[RRLWR_MAX_SAMPLING_BITLEN*(RRLWR_N >> 5)]; // Allocate maximum length
  unsigned char xof_bytes_buffer3[RRLWR_MAX_SAMPLING_BITLEN*(RRLWR_N >> 5)]; // Allocate maximum length
  unsigned char seed_buffer0[RRLWR_MAX_SEED_LEN+2];                          // Allocate maximum length
  unsigned char seed_buffer1[RRLWR_MAX_SEED_LEN+2];                          // Allocate maximum length
  unsigned char seed_buffer2[RRLWR_MAX_SEED_LEN+2];                          // Allocate maximum length
  unsigned char seed_buffer3[RRLWR_MAX_SEED_LEN+2];                          // Allocate maximum length

  // Concatenate the seed with the ring element index i
  for(int32_t i = 0; i < seed_len; i++) {
    seed_buffer0[i] = seed[i];
    seed_buffer1[i] = seed[i];
    seed_buffer2[i] = seed[i];
    seed_buffer3[i] = seed[i];
  }
  seed_buffer0[seed_len] = coeff;
  seed_buffer1[seed_len] = coeff;
  seed_buffer2[seed_len] = coeff;
  seed_buffer3[seed_len] = coeff;
  seed_buffer0[seed_len+1] = 0;
  seed_buffer1[seed_len+1] = 1;
  seed_buffer2[seed_len+1] = 2;
  seed_buffer3[seed_len+1] = 3;

  // Generate the output byte stream
  RRLWR_XOF_SECRET(xof_bytes_buffer0, bitlen*(RRLWR_N >> 5), seed_buffer0, seed_len+2);
  RRLWR_XOF_SECRET(xof_bytes_buffer1, bitlen*(RRLWR_N >> 5), seed_buffer1, seed_len+2);
  RRLWR_XOF_SECRET(xof_bytes_buffer2, bitlen*(RRLWR_N >> 5), seed_buffer2, seed_len+2);
  RRLWR_XOF_SECRET(xof_bytes_buffer3, bitlen*(RRLWR_N >> 5), seed_buffer3, seed_len+2);

  // Unpack into polynomial coefficients
  subpoly_unpack(&r->coeffs[0*(RRLWR_N>>2)], RRLWR_N>>2, xof_bytes_buffer0, bitlen);
  subpoly_unpack(&r->coeffs[1*(RRLWR_N>>2)], RRLWR_N>>2, xof_bytes_buffer1, bitlen);
  subpoly_unpack(&r->coeffs[2*(RRLWR_N>>2)], RRLWR_N>>2, xof_bytes_buffer2, bitlen);
  subpoly_unpack(&r->coeffs[3*(RRLWR_N>>2)], RRLWR_N>>2, xof_bytes_buffer3, bitlen);
}

/// @brief Generate a ring element with coefficients pseudo-randomly generated in the uniform distribution [-bitlen/2, bitlen/2-1]
void ring_uniform_secret_x4(ring_element *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len) {
  for(unsigned char i = 0; i < RRLWR_K; i++) {
    poly_uniform_secret_x4(&r->x[i], bitlen, seed, seed_len, i);
  }
}

// Uniform polynomial generation with 4 independent byte streams
static void poly_uniform_secret(poly *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len, unsigned char coeff)
{
  unsigned char xof_bytes_buffer[RRLWR_MAX_SAMPLING_BITLEN*(RRLWR_N >> 3)]; // Allocate maximum length
  unsigned char seed_buffer[RRLWR_MAX_SEED_LEN+2];                          // Allocate maximum length

  // Concatenate the seed with the ring element index i
  for(int32_t i = 0; i < seed_len; i++) {
    seed_buffer[i] = seed[i];
  }
  seed_buffer[seed_len] = coeff;
  seed_buffer[seed_len+1] = 0;

  // Generate the output byte stream
  RRLWR_XOF_SECRET(xof_bytes_buffer, bitlen*(RRLWR_N >> 3), seed_buffer, seed_len+2);

  // Unpack into polynomial coefficients
  poly_unpack(r, xof_bytes_buffer, bitlen);
}

/// @brief Generate a ring element with coefficients pseudo-randomly generated in the uniform distribution [-bitlen/2, bitlen/2-1]
void ring_uniform_secret(ring_element *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len) {
  for(unsigned char i = 0; i < RRLWR_K; i++) {
    poly_uniform_secret(&r->x[i], bitlen, seed, seed_len, i);
  }
}
