Xtransincludedir = $(includedir)/X11/Xtrans
Xtransinclude_HEADERS = \
	Xtrans.h \
	Xtrans.c \
	Xtransint.h \
	Xtranslcl.c \
	Xtranssock.c \
	Xtranstli.c \
	Xtransutil.c \
	transport.c

aclocaldir = $(datadir)/aclocal
aclocal_DATA = xtrans.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xtrans.pc

EXTRA_DIST = xtrans.pc.in ${aclocal_DATA} ChangeLog

MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog
