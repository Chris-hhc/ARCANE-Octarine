SUBDIRS = avx2 ref sm3_ref

.DEFAULT_GOAL := all

.PHONY: all test speed KAT breakdown clean avx2 ref sm3_ref FORCE

all test speed KAT breakdown:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

avx2:
	$(MAKE) -C avx2

ref:
	$(MAKE) -C ref

sm3_ref:
	$(MAKE) -C sm3_ref

avx2-%: FORCE
	$(MAKE) -C avx2 $*

ref-%: FORCE
	$(MAKE) -C ref $*

sm3_ref-%: FORCE
	$(MAKE) -C sm3_ref $*

FORCE:
