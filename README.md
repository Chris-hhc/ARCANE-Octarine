# ARCANE Octarine

This repository contains two aligned implementations of ARCANE Octarine:

- `avx2/`: optimized AVX2 implementation.
- `ref/`: scalar C reference implementation.

The default AVX2 path uses:

- AVX2 NTT/INTT assembly in `arith/ntt.S` and `arith/intt.S`.
- Awin matrix layout and row-dot multiplication in `arith/ring.c` and
  `arith/pointwise.S`.
- AVX2 polynomial helpers in `arith/poly_avx.S`.
- AVX2 pack/unpack dispatch in `arith/packing.c` and `arith/packing_avx.S`.
- 4-seed `shake128x4` sampling in `arith/uniform.c`.

The reference path is scalar C only. It uses the same 4-seed sampling layout for
KAT alignment, but without AVX2 kernels.

## Project Layout

```text
RRLWR_SIGN/
  README.md
  Makefile
  .gitignore
  avx2/
    Makefile
    arith/
    KAT/
    test/
    utils/
  ref/
    Makefile
    arith/
    KAT/
    test/
    utils/
```

The top-level `Makefile` is a convenience wrapper. It forwards common targets to
both implementations:

| Target | Action |
| --- | --- |
| `make` | Build `test`, `speed`, and `KAT` for both `avx2/` and `ref/` |
| `make test` | Build unit and functional tests for both implementations |
| `make speed` | Build speed tests for both implementations |
| `make KAT` | Build KAT generators for both implementations |
| `make breakdown` | Build breakdown benchmarks for both implementations |
| `make clean` | Clean generated binaries and output in both implementations |
| `make avx2-speed` | Run a target only in `avx2/`; works for `all/test/speed/KAT/breakdown/clean` |
| `make ref-speed` | Run a target only in `ref/`; works for `all/test/speed/KAT/breakdown/clean` |

All benchmark numbers below are **median cycles/ticks**. Speedup is computed as:

```text
speedup = REF median / AVX2 median
```

The tests were run serially for all security levels to reduce measurement noise.

---

## Test CPU

CPU information was collected with `lscpu`.

| Field | Value |
| --- | --- |
| Architecture | x86_64 |
| CPU model | Intel(R) Xeon(R) CPU E5-2686 v4 @ 2.30GHz |
| CPU family / model | 6 / 79 |
| CPUs | 2 |
| Cores per socket | 2 |
| Threads per core | 1 |
| Socket(s) | 1 |
| L1d cache | 64 KiB (2 instances) |
| L1i cache | 64 KiB (2 instances) |
| L2 cache | 512 KiB (2 instances) |
| L3 cache | 45 MiB (1 instance) |
| Hypervisor | Xen |
| AVX2 / BMI2 | supported |

## Build And Benchmark

Build both implementations from the repository root:

```sh
make clean
make speed breakdown
```

Build only one implementation when needed:

```sh
make avx2-speed avx2-breakdown
make ref-speed ref-breakdown
```

Run the benchmark binaries serially:

```sh
cd avx2
./test/test_speed_SIGN128
./test/test_speed_SIGN256
./test/test_speed_SIGN512
./test/test_breakdown_SIGN128
./test/test_breakdown_SIGN256
./test/test_breakdown_SIGN512

cd ../ref
./test/test_speed_SIGN128
./test/test_speed_SIGN256
./test/test_speed_SIGN512
./test/test_breakdown_SIGN128
./test/test_breakdown_SIGN256
./test/test_breakdown_SIGN512
```

## Whole-Signature Stage Comparison

These numbers come from `test/test_speed_SIGN.c`.

| Security | Stage | AVX2 median | REF median | Speedup |
| --- | --- | ---: | ---: | ---: |
| SIGN128 | keygen | 79,689 | 453,264 | 5.69x |
| SIGN128 | sign | 202,639 | 1,369,972 | 6.76x |
| SIGN128 | verify | 102,289 | 642,224 | 6.28x |
| SIGN256 | keygen | 154,830 | 947,913 | 6.12x |
| SIGN256 | sign | 382,162 | 2,679,901 | 7.01x |
| SIGN256 | verify | 196,268 | 1,285,736 | 6.55x |
| SIGN512 | keygen | 383,782 | 2,758,914 | 7.19x |
| SIGN512 | sign | 1,230,276 | 9,612,832 | 7.81x |
| SIGN512 | verify | 475,594 | 3,543,268 | 7.45x |

## Compare With Dilithium

These numbers are **median cycles/ticks** from ARCANE Octarine
`avx2/test/test_speed_SIGN128`, ARCANE Octarine
`avx2/test/test_speed_SIGN256`, Dilithium `avx2/test/test_speed2`, and
Dilithium `avx2/test/test_speed5`.

| Comparison | Stage | ARCANE Octarine median | Dilithium median | Faster |
| --- | --- | ---: | ---: | --- |
| SIGN128 vs Dilithium2 | keygen/keypair | **79,470** | 87,330 | ARCANE Octarine, 1.10x |
| SIGN128 vs Dilithium2 | sign | **201,883** | 205,427 | ARCANE Octarine, 1.02x |
| SIGN128 vs Dilithium2 | verify | 102,485 | **91,562** | Dilithium, 1.12x |
| SIGN256 vs Dilithium5 | keygen/keypair | **154,568** | 236,033 | ARCANE Octarine, 1.53x |
| SIGN256 vs Dilithium5 | sign | **380,788** | 441,321 | ARCANE Octarine, 1.16x |
| SIGN256 vs Dilithium5 | verify | **197,319** | 238,195 | ARCANE Octarine, 1.21x |

Reference implementation comparison:

| Comparison | Stage | ARCANE Octarine REF median | Dilithium REF median | Faster |
| --- | --- | ---: | ---: | --- |
| SIGN128 vs Dilithium2 | keygen/keypair | 453,634 | **257,243** | Dilithium, 1.76x |
| SIGN128 vs Dilithium2 | sign | 1,365,431 | **948,244** | Dilithium, 1.44x |
| SIGN128 vs Dilithium2 | verify | 642,722 | **276,792** | Dilithium, 2.32x |
| SIGN256 vs Dilithium5 | keygen/keypair | 946,967 | **694,840** | Dilithium, 1.36x |
| SIGN256 vs Dilithium5 | sign | 2,670,530 | **1,917,170** | Dilithium, 1.39x |
| SIGN256 vs Dilithium5 | verify | 1,282,368 | **722,102** | Dilithium, 1.78x |

## Kernel Breakdown

These numbers come from `test/test_breakdown_SIGN.c`. Some stage names differ
between implementations because the AVX2 path uses Awin layout and the reference
path uses the direct scalar ring multiplication path.

### SIGN128

| Kernel/stage | AVX2 median | REF median | Speedup |
| --- | ---: | ---: | ---: |
| keygen total | 81,767 | 455,325 | 5.57x |
| keygen random/hash | 8,786 | 9,122 | 1.04x |
| keygen A sampling/prepare | 22,388 | 27,325 | 1.22x |
| keygen secret sampling | 2,482 | 6,897 | 2.78x |
| keygen A*s1 | 30,848 | 390,415 | 12.66x |
| keygen pack s1 | 534 | 1,080 | 2.02x |
| keygen round s2 | 920 | 1,595 | 1.73x |
| keygen pack b | 1,782 | 4,376 | 2.46x |
| keygen hash/store | 13,227 | 13,322 | 1.01x |
| sign total | 206,885 | 1,368,126 | 6.61x |
| sign prepare A | 22,593 | 141,427 | 6.26x |
| sign unpack+NTT secret | 19,363 | 168,860 | 8.72x |
| sign hash mu/rhopp | 12,700 | 13,007 | 1.02x |
| sign sample y | 16,381 | 46,666 | 2.85x |
| sign NTT y x2 | 26,315 | 225,393 | 8.57x |
| sign A*y | 35,360 | 332,457 | 9.40x |
| sign w1/hash ctilde | 19,315 | 21,090 | 1.09x |
| sign sample c+NTT | 15,426 | 113,061 | 7.33x |
| sign c*s2/check w | 16,454 | 147,388 | 8.96x |
| sign c*s1/check y | 8,128 | 73,516 | 9.04x |
| sign c*b0/hint | 8,995 | 74,600 | 8.29x |
| sign pack signature | 3,720 | 7,316 | 1.97x |
| verify total | 103,583 | 643,358 | 6.21x |
| verify unpack/check | 1,460 | 4,717 | 3.23x |
| verify prepare A | 22,252 | 27,729 | 1.25x |
| verify A*z | 30,345 | 389,386 | 12.83x |
| verify sample c+NTT | 7,753 | 56,703 | 7.31x |
| verify unpack hint | 699 | 850 | 1.22x |
| verify c*b1/w1 | 15,561 | 130,795 | 8.41x |
| verify hash compare | 24,681 | 25,114 | 1.02x |

### SIGN256

| Kernel/stage | AVX2 median | REF median | Speedup |
| --- | ---: | ---: | ---: |
| keygen total | 159,128 | 986,945 | 6.20x |
| keygen random/hash | 8,802 | 9,289 | 1.06x |
| keygen A sampling/prepare | 49,662 | 53,866 | 1.08x |
| keygen secret sampling | 3,397 | 7,624 | 2.24x |
| keygen A*s1 | 65,956 | 877,696 | 13.31x |
| keygen pack s1 | 994 | 1,748 | 1.76x |
| keygen round s2 | 1,890 | 2,791 | 1.48x |
| keygen pack b | 4,505 | 8,122 | 1.80x |
| keygen hash/store | 22,747 | 22,968 | 1.01x |
| sign total | 388,588 | 2,790,125 | 7.18x |
| sign prepare A | 49,620 | 295,854 | 5.96x |
| sign unpack+NTT secret | 38,659 | 356,688 | 9.23x |
| sign hash mu/rhopp | 12,681 | 13,271 | 1.05x |
| sign sample y | 32,711 | 95,023 | 2.90x |
| sign NTT y x2 | 54,542 | 479,730 | 8.80x |
| sign A*y | 79,930 | 793,263 | 9.92x |
| sign w1/hash ctilde | 31,492 | 35,700 | 1.13x |
| sign sample c+NTT | 16,111 | 120,442 | 7.48x |
| sign c*s2/check w | 25,364 | 229,995 | 9.07x |
| sign c*s1/check y | 16,210 | 152,567 | 9.41x |
| sign c*b0/hint | 17,768 | 154,726 | 8.71x |
| sign pack signature | 8,523 | 12,755 | 1.50x |
| verify total | 198,564 | 1,338,997 | 6.74x |
| verify unpack/check | 2,954 | 11,086 | 3.75x |
| verify prepare A | 48,725 | 54,668 | 1.12x |
| verify A*z | 64,983 | 878,318 | 13.52x |
| verify sample c+NTT | 8,047 | 60,369 | 7.50x |
| verify unpack hint | 1,549 | 1,982 | 1.28x |
| verify c*b1/w1 | 31,841 | 275,488 | 8.65x |
| verify hash compare | 39,358 | 39,910 | 1.01x |

### SIGN512

| Kernel/stage | AVX2 median | REF median | Speedup |
| --- | ---: | ---: | ---: |
| keygen total | 394,692 | 2,762,108 | 7.00x |
| keygen random/hash | 9,228 | 9,467 | 1.03x |
| keygen A sampling/prepare | 124,761 | 123,651 | 0.99x |
| keygen secret sampling | 7,051 | 14,997 | 2.13x |
| keygen A*s1 | 185,244 | 2,531,426 | 13.67x |
| keygen pack s1 | 2,815 | 4,335 | 1.54x |
| keygen round s2 | 8,164 | 9,624 | 1.18x |
| keygen pack b | 6,566 | 14,259 | 2.17x |
| keygen hash/store | 48,398 | 48,721 | 1.01x |
| sign total | 1,225,664 | 9,559,070 | 7.80x |
| sign prepare A | 123,697 | 726,370 | 5.87x |
| sign unpack+NTT secret | 96,803 | 910,223 | 9.40x |
| sign hash mu/rhopp | 12,926 | 13,917 | 1.08x |
| sign sample y | 131,118 | 343,137 | 2.62x |
| sign NTT y x2 | 201,633 | 1,773,285 | 8.79x |
| sign A*y | 357,933 | 3,944,737 | 11.02x |
| sign w1/hash ctilde | 73,327 | 84,304 | 1.15x |
| sign sample c+NTT | 35,425 | 192,186 | 5.43x |
| sign c*s2/check w | 83,280 | 784,574 | 9.42x |
| sign c*s1/check y | 40,398 | 398,895 | 9.87x |
| sign c*b0/hint | 44,044 | 394,910 | 8.97x |
| sign pack signature | 18,527 | 28,787 | 1.55x |
| verify total | 477,119 | 3,546,821 | 7.43x |
| verify unpack/check | 7,910 | 13,589 | 1.72x |
| verify prepare A | 121,662 | 124,560 | 1.02x |
| verify A*z | 179,149 | 2,557,195 | 14.27x |
| verify sample c+NTT | 12,256 | 64,538 | 5.27x |
| verify unpack hint | 3,055 | 4,104 | 1.34x |
| verify c*b1/w1 | 79,678 | 700,204 | 8.79x |
| verify hash compare | 70,745 | 71,616 | 1.01x |

## Keygen Matrix-Multiply Detail

The largest keygen speedup comes from `A*s1`. The scalar reference computes
forward NTTs for both operands and then performs scalar multiply/inverse NTT.
The AVX2 implementation samples `A` into the Awin layout, prepares it once, and
uses AVX2 row-dot multiplication.

| Security | Detail | AVX2 median | REF median | Speedup |
| --- | --- | ---: | ---: | ---: |
| SIGN128 | `A*s1.mul_invntt_x2` | 17,686 | 165,592 | 9.36x |
| SIGN128 | `A*s1.mul_invntt_p1` | 7,980 | 77,612 | 9.73x |
| SIGN128 | `A*s1.mul_invntt_p2` | 7,830 | 77,504 | 9.90x |
| SIGN128 | `A*s1.crt` | 1,742 | 10,298 | 5.91x |
| SIGN256 | `A*s1.mul_invntt_x2` | 38,444 | 394,387 | 10.26x |
| SIGN256 | `A*s1.mul_invntt_p1` | 17,468 | 186,421 | 10.67x |
| SIGN256 | `A*s1.mul_invntt_p2` | 17,362 | 186,466 | 10.74x |
| SIGN256 | `A*s1.crt` | 3,500 | 21,102 | 6.03x |
| SIGN512 | `A*s1.mul_invntt_x2` | 116,135 | 1,331,954 | 11.47x |
| SIGN512 | `A*s1.mul_invntt_p1` | 53,925 | 620,427 | 11.51x |
| SIGN512 | `A*s1.mul_invntt_p2` | 53,067 | 623,757 | 11.75x |
| SIGN512 | `A*s1.crt` | 9,012 | 66,286 | 7.36x |

## Takeaways

- Whole-operation speedup is 5.69x to 7.81x in `test_speed_SIGN`.
- The main computational speedups are in NTT-domain operations and matrix-vector
  multiplication: `A*s1`, `A*y`, `A*z`, `c*s*`, and `c*b*`.
- Hash-only stages such as `random_hash`, `hash_store`, and `hash_compare`
  remain near 1x because both implementations use the same scalar SHA3/SHAKE
  path there.
- For SIGN512, `A` sampling/prepare is almost equal between AVX2 and REF in the
  top-level keygen breakdown because the dominant cost is XOF generation; the
  large speedup comes afterward in `A*s1`.
