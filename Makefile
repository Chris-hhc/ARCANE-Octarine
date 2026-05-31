SUBDIRS = avx2 ref

.DEFAULT_GOAL := all

.PHONY: all test speed KAT breakdown clean avx2 ref FORCE

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

avx2-%: FORCE
	$(MAKE) -C avx2 $*

ref-%: FORCE
	$(MAKE) -C ref $*

FORCE:
