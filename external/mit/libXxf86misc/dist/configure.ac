AC_PREREQ([2.57])

AC_INIT(libXxf86misc, 1.0.1, [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg], libXxf86misc)
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE

AM_CONFIG_HEADER(src/config.h)

# Check for progs
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for pkg-config packages
PKG_CHECK_MODULES(XXF86MISC, xproto x11 xextproto xext xf86miscproto)

XORG_CHECK_MALLOC_ZERO
XORG_MANPAGE_SECTIONS
XORG_RELEASE_VERSION

AC_OUTPUT([Makefile
	   src/Makefile
	   man/Makefile
	   xxf86misc.pc])
