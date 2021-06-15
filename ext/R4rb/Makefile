SHELL = /bin/sh
srcdir = .
VPATH = $(srcdir)
all:  R4rb.bundle

R4rb.bundle: R4rb.c
	@echo Now Making R4rb-orig extend module
	@$(MAKE) -f Makefile-orig

clean:
	@$(MAKE) -f Makefile-orig clean

distclean:
	@$(MAKE) -f Makefile-orig distclean

install:  R4rb.bundle
	@$(MAKE) -f Makefile-orig install

site-install:  R4rb.bundle
	@$(MAKE) -f Makefile-orig site-install

