SUBDIRS = src 

desktopdir= $(datadir)/applications
dist_desktop_DATA = jd.desktop

icondir = $(datadir)/pixmaps
dist_icon_DATA = jd.png

EXTRA_DIST = AUTHORS TODO README configure jd.spec

jd_BUILDINFO_HEADER = buildinfo.h
jd_CONFIGURE_ARGS = @ac_configure_args@

all: $(jd_BUILDINFO_HEADER)

.PHONY: $(jd_BUILDINFO_HEADER)

$(jd_BUILDINFO_HEADER):
	@-rm -f $@.new

	@echo '/* This file is generated from Makefile by make. */' >> $@.new
	@echo '#ifndef _BUILDINFO_H' >> $@.new
	@echo '#define _BUILDINFO_H' >> $@.new

	@echo '' >> $@.new
	@echo '// Build information.' >> $@.new

	@if test -n "$(CC)"; \
	then \
	    CC_INFO=$(CC); \
	    CC_VERSION=`LANG=C $(CC) -dumpversion 2>/dev/null`; \
	    test -n "$${CC_VERSION}" && CC_INFO="$(CC)($${CC_VERSION})"; \
	    echo "#define CC_INFO \"$${CC_INFO}\"" >> $@.new; \
	fi
	@if test -n "$(CXX)"; \
	then \
	    CXX_INFO=$(CXX); \
	    CXX_VERSION=`LANG=C $(CXX) -dumpversion 2>/dev/null`; \
	    test -n "$${CXX_VERSION}" && CXX_INFO="$(CXX)($${CXX_VERSION})"; \
	    echo "#define CXX_INFO \"$${CXX_INFO}\"" >> $@.new; \
	fi

	@if test -n "$(jd_CONFIGURE_ARGS)"; \
	then \
	    echo "#define CONFIGURE_ARGS \"$(jd_CONFIGURE_ARGS)\"" >> $@.new; \
	fi

	@echo '' >> $@.new
	@echo '// Version information of SVN.' >> $@.new

	@if test -n "$(SVN)" -a -n "$(AWK)"; \
	then \
	    SVN_REPOSITORY=`LANG=C "$(SVN)" info 2>/dev/null | $(AWK) '/^URL/ {print $$2}' 2>/dev/null`; \
	    if test -n "$${SVN_REPOSITORY}"; \
	    then \
	        echo "SVN: Repository = \"$${SVN_REPOSITORY}\""; \
	        echo "#define SVN_REPOSITORY \"$${SVN_REPOSITORY}\"" >> $@.new; \
	    fi \
	fi

	@if test -n "$(SVNVERSION)"; \
	then \
	    SVN_REVISION=`LANG=C "$(SVNVERSION)" -n 2>/dev/null`; \
	    if test -n "$${SVN_REVISION}" -a "$${SVN_REVISION}" != "exported"; \
	    then \
	        echo "SVN: Revision = \"$${SVN_REVISION}\""; \
	        echo "#define SVN_REVISION \"$${SVN_REVISION}\"" >> $@.new; \
	    fi \
	fi

	@echo '' >> $@.new
	@echo '// Version information of GIT.' >> $@.new

	@GIT_HASH="" ; \
	GIT_DATE="" ; \
	GIT_DIRTY=0; \
	GIT_STATUS="" ; \
	if test -n "$(GIT)" ; \
	then \
		GIT_HASH=`LANG=C.utf8 "$(GIT)" log --pretty=format:%h -n 1 2>/dev/null`; \
		GIT_DATE=`LANG=C.utf8 "$(GIT)" log --pretty=format:%ad --date=format:%Y%m%d -n 1 2>/dev/null`; \
		GIT_STATUS="`LANG=C.utf8 "$(GIT)" status -uno -s 2>/dev/null | head -n 1`" ; \
		if test -n "$${GIT_HASH}" ; \
		then \
			echo "GIT: Hash = \"$${GIT_HASH}\"" ; \
		fi ; \
		if test -n "$${GIT_DATE}" ; \
		then \
			echo "GIT: Date = \"$${GIT_DATE}\"" ; \
		fi ; \
		if test -n "$${GIT_STATUS}" ; \
		then \
			echo "GIT: There are changes not committed yet." ; \
			GIT_DIRTY=1 ; \
		fi \
	fi ; \
	echo "#define GIT_HASH \"$${GIT_HASH}\"" >> $@.new ; \
	echo "#define GIT_DATE \"$${GIT_DATE}\"" >> $@.new ; \
	echo "#define GIT_DIRTY $${GIT_DIRTY}" >> $@.new

	@echo '' >> $@.new
	@echo '#endif' >> $@.new

	@if test ! -e $@; \
	then \
	    echo '$@: Created.'; \
	    mv -f $@.new $@; \
	elif test -z "$(XSUM)"; \
	then \
	    echo '$@: Refreshed.'; \
	    mv -f $@.new $@; \
	else \
	    HASH1=`cat $@ | $(XSUM)`; \
	    HASH2=`cat $@.new | $(XSUM)`; \
	    if test "$${HASH1}" = "$${HASH2}"; \
	    then \
	        echo '$@: Not modified.'; \
	        rm -f $@.new; \
	    else \
	        echo '$@: Modified.'; \
	        mv -f $@.new $@; \
	    fi \
	fi
