SUBDIRS = src 

desktopdir= $(datadir)/applications
desktop_DATA = jd.desktop

icondir = $(datadir)/pixmaps
icon_DATA = jd.png

EXTRA_DIST = AUTHORS TODO README configure 

SVN_REPOSITORY = $(shell LANG=C svn info | $(AWK) '/^URL/ {print $$2}' 2>/dev/null)
SVN_REVISION = $(shell LANG=C svnversion -n 2>/dev/null)

all: svnversion.h

.PHONY: svnversion.h

svnversion.h:
	-rm -f $@.new

	@echo '/* This file is generated from Makefile by make. */' >> $@.new
	@echo '#ifndef _SVNVERSION_H' >> $@.new
	@echo '#define _SVNVERSION_H' >> $@.new
	@echo '' >> $@.new

	@echo '// Version information of SVN.' >> $@.new
	@if test -n "$(SVN_REPOSITORY)"; \
	then \
	    echo 'SVN: Repository = "$(SVN_REPOSITORY)"'; \
	    echo '#define SVN_REPOSITORY "$(SVN_REPOSITORY)"' >> $@.new; \
	fi

	@if test -n "$(SVN_REVISION)" && test "$(SVN_REVISION)" != 'exported'; \
	then \
	    echo 'SVN: Revision = "$(SVN_REVISION)"'; \
	    echo '#define SVN_REVISION "$(SVN_REVISION)"' >> $@.new; \
	fi

	@echo '' >> $@.new
	@echo '#endif' >> $@.new

	@diff -Nq $@ $@.new >/dev/null 2>&1; \
	if test $$? -eq 1; \
	then \
	    echo 'SVN: Modified.'; \
	    mv -f $@.new $@; \
	else \
	    echo 'SVN: Not modified.'; \
	    rm -f $@.new; \
	    test -e $@ || touch $@; \
	fi
