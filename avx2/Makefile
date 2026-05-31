CC = gcc
CFLAGS += -O3
CFLAGS += -mavx2
CFLAGS += -mbmi2
CFLAGS += -Wall -Wextra -Wpedantic -Wmissing-prototypes -Wredundant-decls \
  -Wshadow -Wpointer-arith -fomit-frame-pointer
RM = /bin/rm

BASE_FOLDER = .
ARITH_FOLDER = ./arith
UTILS_FOLDER = ./utils
KECCAK4X_FOLDER = $(UTILS_FOLDER)/keccak4x
INCLUDES = -I$(BASE_FOLDER) -I$(UTILS_FOLDER) -I$(KECCAK4X_FOLDER) -I$(ARITH_FOLDER)

SECURITY_LEVELS = 128 256 512

SOURCES_RRLWR = $(ARITH_FOLDER)/fprime.c $(ARITH_FOLDER)/poly.c $(ARITH_FOLDER)/ring.c $(ARITH_FOLDER)/packing.c $(ARITH_FOLDER)/uniform.c $(ARITH_FOLDER)/ntt.S $(ARITH_FOLDER)/intt.S $(ARITH_FOLDER)/pointwise.S $(ARITH_FOLDER)/poly_avx.S $(ARITH_FOLDER)/packing_avx.S sign_ring.c sign.c crt.c crt_avx.S
SOURCES_UTILS = $(UTILS_FOLDER)/fips202.c $(UTILS_FOLDER)/fips202x4.c $(KECCAK4X_FOLDER)/KeccakP-1600-times4-SIMD256.c
SOURCES = $(SOURCES_RRLWR) $(SOURCES_UTILS)
HEADERS_RRLWR = $(ARITH_FOLDER)/fprime.h $(ARITH_FOLDER)/poly.h $(ARITH_FOLDER)/ring.h $(ARITH_FOLDER)/packing.h $(ARITH_FOLDER)/uniform.h $(ARITH_FOLDER)/ntt.h $(ARITH_FOLDER)/ntt_param.h $(ARITH_FOLDER)/intt_param.h $(ARITH_FOLDER)/ntt_avx_macros.inc parameters.h sign_ring.h sign.h crt.h
HEADERS_UTILS = $(UTILS_FOLDER)/fips202.h $(UTILS_FOLDER)/fips202x4.h $(KECCAK4X_FOLDER)/KeccakP-1600-times4-SnP.h $(KECCAK4X_FOLDER)/KeccakP-align.h $(KECCAK4X_FOLDER)/KeccakP-brg_endian.h $(KECCAK4X_FOLDER)/KeccakP-SIMD256-config.h
HEADERS = $(HEADERS_RRLWR) $(HEADERS_UTILS)

TEST_TARGETS = $(addprefix test/test_SIGN,$(SECURITY_LEVELS))
UNIT_TEST_TARGETS = $(addprefix test/unit_tests_SIGN,$(SECURITY_LEVELS))
SPEED_TARGETS = $(addprefix test/test_speed_SIGN,$(SECURITY_LEVELS))
KAT_TARGETS = $(addprefix KAT/KAT_SIGN,$(SECURITY_LEVELS))
BREAKDOWN_TARGETS = $(addprefix test/test_breakdown_SIGN,$(SECURITY_LEVELS))
PACKING_AVX_TARGETS = $(addprefix test/test_packing_avx_SIGN,$(SECURITY_LEVELS))
TARGETS = $(TEST_TARGETS) $(UNIT_TEST_TARGETS) $(SPEED_TARGETS) $(KAT_TARGETS) $(BREAKDOWN_TARGETS) $(PACKING_AVX_TARGETS)

.PHONY: all test speed KAT breakdown packing-avx-test clean

all: test speed KAT breakdown

test: $(TEST_TARGETS) $(UNIT_TEST_TARGETS)

speed: $(SPEED_TARGETS)

KAT: $(KAT_TARGETS)

breakdown: $(BREAKDOWN_TARGETS)

packing-avx-test: $(PACKING_AVX_TARGETS)

$(TEST_TARGETS): test/test_SIGN%: $(SOURCES) $(HEADERS) test/test_SIGN.c
	$(CC) $(INCLUDES) -DRRLWR_SECURITY_LEVEL=$* -DMEASURE_HEURISTICS $(CFLAGS) $(SOURCES) $(UTILS_FOLDER)/drng.c test/test_SIGN.c -o $@

$(UNIT_TEST_TARGETS): test/unit_tests_SIGN%: $(SOURCES) $(HEADERS) test/unit_tests_SIGN.c
	$(CC) $(INCLUDES) -DRRLWR_SECURITY_LEVEL=$* $(CFLAGS) $(SOURCES) $(UTILS_FOLDER)/drng.c test/unit_tests_SIGN.c -o $@

$(SPEED_TARGETS): test/test_speed_SIGN%: $(SOURCES) $(HEADERS) test/cpucycles.h test/cpucycles.c test/speed_print.c test/speed_print.h test/test_speed_SIGN.c
	$(CC) $(INCLUDES) -DRRLWR_SECURITY_LEVEL=$* $(CFLAGS) $(SOURCES) $(UTILS_FOLDER)/drng.c test/cpucycles.c test/speed_print.c test/test_speed_SIGN.c -o $@

$(KAT_TARGETS): KAT/KAT_SIGN%: $(SOURCES) $(HEADERS) KAT/KAT_SIG.c
	$(CC) $(INCLUDES) -DRRLWR_SECURITY_LEVEL=$* $(CFLAGS) $(SOURCES) $(UTILS_FOLDER)/drng.c KAT/KAT_SIG.c -o $@

$(BREAKDOWN_TARGETS): test/test_breakdown_SIGN%: $(filter-out sign.c,$(SOURCES)) sign_test.c sign_test.h $(HEADERS) test/cpucycles.h test/cpucycles.c test/test_breakdown_SIGN.c
	$(CC) $(INCLUDES) -DRRLWR_SECURITY_LEVEL=$* -DRRLWR_SIGN_BREAKDOWN $(CFLAGS) $(filter-out sign.c,$(SOURCES)) sign_test.c $(UTILS_FOLDER)/drng.c test/cpucycles.c test/test_breakdown_SIGN.c -o $@

$(PACKING_AVX_TARGETS): test/test_packing_avx_SIGN%: $(ARITH_FOLDER)/packing.c $(ARITH_FOLDER)/packing_avx.S $(HEADERS) test/cpucycles.h test/cpucycles.c test/test_packing_avx.c
	$(CC) $(INCLUDES) -DRRLWR_SECURITY_LEVEL=$* -fno-tree-vectorize $(CFLAGS) $(ARITH_FOLDER)/packing.c $(ARITH_FOLDER)/packing_avx.S test/cpucycles.c test/test_packing_avx.c -o $@

clean:
	-$(RM) -f $(TARGETS)
