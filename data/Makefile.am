# vim:set noet ts=4:
#
# ibus-handwrite - The Input Bus handwrite project
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
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

EXTRA_DIST = table.txt

modelsdir = $(pkgdatadir)/data

models_DATA = table.txt

if HAVE_TOMOE

else

EXTRA_DIST += handwriting-zh_CN.model

models_DATA +=  $(srcdir)/handwriting-zh_CN.model 
# $(srcdir)/handwriting-ja.model



#$(srcdir)/handwriting-ja.model :
#	cd $(srcdir) && wget http://ibus-handwrite.googlecode.com/svn/files/handwriting-ja.model

$(srcdir)/handwriting-zh_CN.model:
	cd $(srcdir) && wget -c http://ibus-handwrite.googlecode.com/svn/files/handwriting-zh_CN.model

endif
			