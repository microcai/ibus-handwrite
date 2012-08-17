# vim:set noet ts=4:
#
# ibus-handwrite - The handwrite engine for IBus
#
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

TABLEFILE = $(pkgdatadir)/data/table.txt

test: ibus-engine-handwrite
	$(builddir)/ibus-engine-handwrite \
		--icondir $(top_srcdir)/icons/ \
		--table $(top_srcdir)/data/table.txt \
		--locale /tmp/${prefix}/share/locale/ 
if WITH_ZINNIA		
test-jp: ibus-engine-handwrite
	$(builddir)/ibus-engine-handwrite \
		--icondir $(top_srcdir)/icons/ \
		--table $(top_srcdir)/data/table.txt \
		--locale /tmp/${prefix}/share/locale/ \
		--lang=jp
endif		
AM_CPPFLAGS= @ALL_NEED_CFLAGS@ -DPKGDATADIR=\"$(pkgdatadir)\" -DTABLEFILE=\"$(TABLEFILE)\"

LIBS += @ALL_NEED_LIBS@

libexec_PROGRAMS = ibus-engine-handwrite

ibus_engine_handwrite_SOURCES =  main.c	\
				engine.c \
				engine.h\
				global_var.h \
				handrecog_lucykila.c\
				handrecog.c \
				handrecog.h \
				UI.h \
				$(NULL)


ibus_engine_handwrite_SOURCES +=   UI_clutter.c \
 					  $(NULL)

if WITH_ZINNIA
ibus_engine_handwrite_SOURCES +=  handrecog_zinnia.c
AM_CPPFLAGS += -DWITH_ZINNIA
endif					  
					  

component_DATA = handwrite-zh.xml
if WITH_ZINNIA
component_DATA += handwrite-jp.xml
endif

componentdir = @datadir@/ibus/component

EXTRA_DIST = handwrite-zh.xml.in handwrite-jp.xml.in

CLEANFILES = \
	handwrite-zh.xml \
	handwrite-jp.xml \
	$(NULL)

handwrite-zh.xml: handwrite-zh.xml.in
	( \
		libexecdir=${libexecdir}; \
		pkgdatadir=${pkgdatadir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	) > $@

handwrite-jp.xml: handwrite-jp.xml.in
	( \
		libexecdir=${libexecdir}; \
		pkgdatadir=${pkgdatadir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	) > $@

