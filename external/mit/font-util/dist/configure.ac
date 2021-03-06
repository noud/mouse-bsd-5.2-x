dnl  Copyright 2005 Red Hat, Inc.
dnl
dnl  Permission to use, copy, modify, distribute, and sell this software
dnl  and its documentation for any purpose is hereby granted without
dnl  fee, provided that the above copyright notice appear in all copies
dnl  and that both that copyright notice and this permission notice
dnl  appear in supporting documentation, and that the name of Red Hat
dnl  not be used in advertising or publicity pertaining to distribution
dnl  of the software without specific, written prior permission.  Red
dnl  Hat makes no representations about the suitability of this software
dnl  for any purpose.  It is provided "as is" without express or implied
dnl  warranty.
dnl
dnl  RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
dnl  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
dnl  NO EVENT SHALL RED HAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
dnl  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
dnl  OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
dnl  NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
dnl  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
dnl
dnl Process this file with autoconf to create configure.

AC_PREREQ([2.57])
AC_INIT(font-util, [1.0.2], [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg], font-util)
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE

# Require xorg-macros: XORG_CWARNFLAGS, XORG_CHANGELOG
m4_ifndef([XORG_MACROS_VERSION], [AC_FATAL([must install xorg-macros 1.2 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.2)
AM_CONFIG_HEADER(config.h)
XORG_CWARNFLAGS

AC_PROG_CC
AC_PROG_INSTALL

mapdir=${libdir}/X11/fonts/util
AC_ARG_WITH(mapdir, AC_HELP_STRING([--with-mapdir=<pathname>],
	[Path to install font maps]), [mapdir="$withval"])
MAPDIR="$mapdir"
AC_SUBST(MAPDIR)

XORG_MANPAGE_SECTIONS
XORG_RELEASE_VERSION
XORG_CHANGELOG

AC_OUTPUT([Makefile fontutil.pc])
