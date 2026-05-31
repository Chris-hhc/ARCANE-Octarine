# RRLWR SIGN

This repository contains two aligned implementations of RRLWR SIGN:

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
| SIGN128 | keygen | 81,620 | 453,264 | 5.55x |
| SIGN128 | sign | 214,101 | 1,369,972 | 6.40x |
| SIGN128 | verify | 106,610 | 642,224 | 6.02x |
| SIGN256 | keygen | 158,125 | 947,913 | 5.99x |
| SIGN256 | sign | 395,821 | 2,679,901 | 6.77x |
| SIGN256 | verify | 198,835 | 1,285,736 | 6.47x |
| SIGN512 | keygen | 401,493 | 2,758,914 | 6.87x |
| SIGN512 | sign | 1,309,686 | 9,612,832 | 7.34x |
| SIGN512 | verify | 484,236 | 3,543,268 | 7.32x |

## Kernel Breakdown

These numbers come from `test/test_breakdown_SIGN.c`. Some stage names differ
between implementations because the AVX2 path uses Awin layout and the reference
path uses the direct scalar ring multiplication path.

### SIGN128

| Kernel/stage | AVX2 median | REF median | Speedup |
| --- | ---: | ---: | ---: |
| keygen total | 83,837 | 455,325 | 5.43x |
| keygen random/hash | 9,047 | 9,122 | 1.01x |
| keygen A sampling/prepare | 22,454 | 27,325 | 1.22x |
| keygen secret sampling | 2,653 | 6,897 | 2.60x |
| keygen A*s1 | 32,280 | 390,415 | 12.09x |
| keygen pack s1 | 558 | 1,080 | 1.94x |
| keygen round s2 | 1,077 | 1,595 | 1.48x |
| keygen pack b | 1,862 | 4,376 | 2.35x |
| keygen hash/store | 13,126 | 13,322 | 1.01x |
| sign total | 215,560 | 1,368,126 | 6.35x |
| sign prepare A | 22,896 | 141,427 | 6.18x |
| sign unpack+NTT secret | 19,627 | 168,860 | 8.60x |
| sign hash mu/rhopp | 12,687 | 13,007 | 1.03x |
| sign sample y | 16,549 | 46,666 | 2.82x |
| sign NTT y x2 | 29,746 | 225,393 | 7.58x |
| sign A*y | 35,740 | 332,457 | 9.30x |
| sign w1/hash ctilde | 19,558 | 21,090 | 1.08x |
| sign sample c+NTT | 15,398 | 113,061 | 7.34x |
| sign c*s2/check w | 18,180 | 147,388 | 8.11x |
| sign c*s1/check y | 8,888 | 73,516 | 8.27x |
| sign c*b0/hint | 10,034 | 74,600 | 7.43x |
| sign pack signature | 4,451 | 7,316 | 1.64x |
| verify total | 106,803 | 643,358 | 6.02x |
| verify unpack/check | 1,657 | 4,717 | 2.85x |
| verify prepare A | 22,784 | 27,729 | 1.22x |
| verify A*z | 32,041 | 389,386 | 12.15x |
| verify sample c+NTT | 7,741 | 56,703 | 7.33x |
| verify unpack hint | 715 | 850 | 1.19x |
| verify c*b1/w1 | 15,723 | 130,795 | 8.32x |
| verify hash compare | 25,022 | 25,114 | 1.00x |

### SIGN256

| Kernel/stage | AVX2 median | REF median | Speedup |
| --- | ---: | ---: | ---: |
| keygen total | 160,365 | 986,945 | 6.15x |
| keygen random/hash | 9,289 | 9,289 | 1.00x |
| keygen A sampling/prepare | 48,248 | 53,866 | 1.12x |
| keygen secret sampling | 3,684 | 7,624 | 2.07x |
| keygen A*s1 | 67,765 | 877,696 | 12.95x |
| keygen pack s1 | 1,062 | 1,748 | 1.65x |
| keygen round s2 | 2,172 | 2,791 | 1.28x |
| keygen pack b | 4,331 | 8,122 | 1.88x |
| keygen hash/store | 22,758 | 22,968 | 1.01x |
| sign total | 403,694 | 2,790,125 | 6.91x |
| sign prepare A | 48,935 | 295,854 | 6.05x |
| sign unpack+NTT secret | 39,527 | 356,688 | 9.02x |
| sign hash mu/rhopp | 13,107 | 13,271 | 1.01x |
| sign sample y | 32,063 | 95,023 | 2.96x |
| sign NTT y x2 | 61,897 | 479,730 | 7.75x |
| sign A*y | 77,743 | 793,263 | 10.20x |
| sign w1/hash ctilde | 31,760 | 35,700 | 1.12x |
| sign sample c+NTT | 16,337 | 120,442 | 7.37x |
| sign c*s2/check w | 27,924 | 229,995 | 8.24x |
| sign c*s1/check y | 17,650 | 152,567 | 8.64x |
| sign c*b0/hint | 19,789 | 154,726 | 7.82x |
| sign pack signature | 10,645 | 12,755 | 1.20x |
| verify total | 201,753 | 1,338,997 | 6.64x |
| verify unpack/check | 3,206 | 11,086 | 3.46x |
| verify prepare A | 48,307 | 54,668 | 1.13x |
| verify A*z | 67,343 | 878,318 | 13.04x |
| verify sample c+NTT | 8,143 | 60,369 | 7.41x |
| verify unpack hint | 1,880 | 1,982 | 1.05x |
| verify c*b1/w1 | 32,231 | 275,488 | 8.55x |
| verify hash compare | 39,608 | 39,910 | 1.01x |

### SIGN512

| Kernel/stage | AVX2 median | REF median | Speedup |
| --- | ---: | ---: | ---: |
| keygen total | 405,384 | 2,762,108 | 6.81x |
| keygen random/hash | 9,360 | 9,467 | 1.01x |
| keygen A sampling/prepare | 123,190 | 123,651 | 1.00x |
| keygen secret sampling | 7,213 | 14,997 | 2.08x |
| keygen A*s1 | 197,528 | 2,531,426 | 12.82x |
| keygen pack s1 | 2,585 | 4,335 | 1.68x |
| keygen round s2 | 7,820 | 9,624 | 1.23x |
| keygen pack b | 6,606 | 14,259 | 2.16x |
| keygen hash/store | 48,531 | 48,721 | 1.00x |
| sign total | 1,216,440 | 9,559,070 | 7.86x |
| sign prepare A | 120,166 | 726,370 | 6.04x |
| sign unpack+NTT secret | 97,376 | 910,223 | 9.35x |
| sign hash mu/rhopp | 13,712 | 13,917 | 1.01x |
| sign sample y | 128,534 | 343,137 | 2.67x |
| sign NTT y x2 | 201,601 | 1,773,285 | 8.80x |
| sign A*y | 342,633 | 3,944,737 | 11.51x |
| sign w1/hash ctilde | 74,157 | 84,304 | 1.14x |
| sign sample c+NTT | 35,719 | 192,186 | 5.38x |
| sign c*s2/check w | 84,056 | 784,574 | 9.33x |
| sign c*s1/check y | 41,018 | 398,895 | 9.72x |
| sign c*b0/hint | 45,155 | 394,910 | 8.75x |
| sign pack signature | 22,658 | 28,787 | 1.27x |
| verify total | 486,038 | 3,546,821 | 7.30x |
| verify unpack/check | 8,636 | 13,589 | 1.57x |
| verify prepare A | 121,314 | 124,560 | 1.03x |
| verify A*z | 184,679 | 2,557,195 | 13.85x |
| verify sample c+NTT | 12,111 | 64,538 | 5.33x |
| verify unpack hint | 4,058 | 4,104 | 1.01x |
| verify c*b1/w1 | 81,103 | 700,204 | 8.63x |
| verify hash compare | 71,534 | 71,616 | 1.00x |

## Keygen Matrix-Multiply Detail

The largest keygen speedup comes from `A*s1`. The scalar reference computes
forward NTTs for both operands and then performs scalar multiply/inverse NTT.
The AVX2 implementation samples `A` into the Awin layout, prepares it once, and
uses AVX2 row-dot multiplication.

| Security | Detail | AVX2 median | REF median | Speedup |
| --- | --- | ---: | ---: | ---: |
| SIGN128 | `A*s1.mul_invntt_x2` | 19,072 | 165,592 | 8.68x |
| SIGN128 | `A*s1.mul_invntt_p1` | 8,608 | 77,612 | 9.02x |
| SIGN128 | `A*s1.mul_invntt_p2` | 8,498 | 77,504 | 9.12x |
| SIGN128 | `A*s1.crt` | 1,837 | 10,298 | 5.61x |
| SIGN256 | `A*s1.mul_invntt_x2` | 40,882 | 394,387 | 9.65x |
| SIGN256 | `A*s1.mul_invntt_p1` | 18,698 | 186,421 | 9.97x |
| SIGN256 | `A*s1.mul_invntt_p2` | 18,345 | 186,466 | 10.16x |
| SIGN256 | `A*s1.crt` | 3,683 | 21,102 | 5.73x |
| SIGN512 | `A*s1.mul_invntt_x2` | 127,590 | 1,331,954 | 10.44x |
| SIGN512 | `A*s1.mul_invntt_p1` | 58,386 | 620,427 | 10.63x |
| SIGN512 | `A*s1.mul_invntt_p2` | 59,337 | 623,757 | 10.51x |
| SIGN512 | `A*s1.crt` | 9,731 | 66,286 | 6.81x |

## Takeaways

- Whole-operation speedup is 5.55x to 7.34x in `test_speed_SIGN`.
- The main computational speedups are in NTT-domain operations and matrix-vector
  multiplication: `A*s1`, `A*y`, `A*z`, `c*s*`, and `c*b*`.
- Hash-only stages such as `random_hash`, `hash_store`, and `hash_compare`
  remain near 1x because both implementations use the same scalar SHA3/SHAKE
  path there.
- For SIGN512, `A` sampling/prepare is almost equal between AVX2 and REF in the
  top-level keygen breakdown because the dominant cost is XOF generation; the
  large speedup comes afterward in `A*s1`.
