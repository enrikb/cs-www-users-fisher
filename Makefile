DESTDIR ?= $(CURDIR)/destdir

all:
	$(MAKE) -C mkfilter
	$(MAKE) -C software/mkfilter/current

install: $(DESTDIR)

install clean:
	DESTDIR=$(DESTDIR)/ $(MAKE) -C mkfilter $@
	DESTDIR=$(DESTDIR)/ $(MAKE) -C software/mkfilter/current $@

$(DESTDIR):
	mkdir -p "$@" 2>/dev/null || true

.PHONY: all install
