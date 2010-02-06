# vim:set et ts=4:
#
# ibus-handwrite - The Ibus handwrite engine
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
# Copyright (c) 2010 microcai <microcai@fedoraproject.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

# if not 1, append datestamp to the version number.
m4_define([package_name], [ibus-handwrite])
m4_define([ibus_released], [0])
m4_define([ibus_major_version], [1])
m4_define([ibus_minor_version], [2])
m4_define([ibus_micro_version], [0])
m4_define(ibus_maybe_datestamp,
    m4_esyscmd([if test x]ibus_released[ != x1; then date +.%Y%m%d | tr -d '\n\r'; fi]))

m4_define([ibus_version],
    ibus_major_version.ibus_minor_version.ibus_micro_version[]ibus_maybe_datestamp)

AC_INIT([package_name], [ibus_version], [http://code.google.com/p/ibus/issues/entry], [package_name])
AM_INIT_AUTOMAKE
AC_GNU_SOURCE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# define PACKAGE_VERSION_* variables
AS_VERSION
AS_NANO
AM_SANITY_CHECK
AM_MAINTAINER_MODE
AC_PROG_CC
AC_ISC_POSIX
AC_HEADER_STDC
PKG_PROG_PKG_CONFIG

AC_ARG_WITH(debian,[AC_HELP_STRING([--with-debian],[enable debian specific build])],
				[AM_CONDITIONAL([WITH_DEBIAN],true)],
				[AM_CONDITIONAL([WITH_DEBIAN],false)])
				
# check ibus
PKG_CHECK_MODULES(IBUS, [
    ibus-1.0 >= 1.2
])

PKG_CHECK_MODULES(GTK2, [
    gtk+-2.0 >= 1.2
])

PKG_CHECK_MODULES(ZINNIA, [
    zinnia >= 0.05
])
# define GETTEXT_* variables
GETTEXT_PACKAGE="$PACKAGE_NAME"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define to the read-only architecture-independent data directory.])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.16.1)

AC_DEFINE(MICROCAI_WITHEMAIL,"microcai@fedoraproject.org",[author microcai and his email])
# OUTPUT files
AC_CONFIG_FILES(po/Makefile.in
Makefile
ibus-handwrite.spec
src/Makefile
src/handwrite.xml.in
icons/Makefile
m4/Makefile
data/Makefile
)

AC_OUTPUT
