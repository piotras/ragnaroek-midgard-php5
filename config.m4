dnl $Id: config.m4 18823 2008-11-18 11:47:01Z piotras $

dnl  Copyright (C) 2007 Piotr Pokora <piotrek.pokora@gmail.com>
dnl 
dnl  This program is free software; you can redistribute it and/or modify it
dnl  under the terms of the GNU Lesser General Public License as published
dnl  by the Free Software Foundation; either version 2 of the License, or
dnl  (at your option) any later version.
dnl 
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl 
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

PATH=${PATH}:/usr/local/bin:/opt/bin

dnl Check if pkg-config exists
PHP_ARG_WITH(midgard, for pkg-config ,
	[ --with-pkg-config[=FILE]	pkg-config binary. FILE is the optional pathname
                          to pkg-config])

if test "$PHP_PKGCONFIG" != "no"; then
	if test "$PHP_PKGCONFIG" != "yes"; then
		MIDGARD_CONFIG=`$php_shtool path pkg-config`
	else
		MIDGARD_CONFIG=$PHP_PKGCONFIG   
	fi

	if test ! -x $MIDGARD_CONFIG || test ! -f $MIDGARD_CONFIG; then
		AC_MSG_ERROR([pkg-config ($MIDGARD_CONFIG) not executable. 
			Please specify the full path, including the scriptname])
	fi
fi

dnl pkg-config --exists module returns nothing :(
dnl MIDGARD_EXISTS=1
dnl dnl Check if midgard module exists
dnl MIDGARD_EXISTS=`"$PKG_CONFIG" --exists midgard`
dnl if test $MIDGARD_EXISTS -ne 0 ; then
dnl 	AC_MSG_ERROR(midgard library not found, please install midgard-core)
dnl fi

dnl TODO, make it working correctly
dnl AC_CHECK_LIB(midgard, midgard_connection_new, [], [
dnl	AC_MSG_ERROR(midgard library not found please install midgard-core)
dnl 	])

MIDGARD_VERSION=`"$MIDGARD_CONFIG" --modversion midgard`
AC_MSG_RESULT(Midgard library found. Version $MIDGARD_VERSION)

case $MIDGARD_VERSION in
	(1.7.*|1.8.*)
	AC_MSG_ERROR([Midgard 1.9 not found. Please install midgard-core 1.9.x])
	;;
esac


LFLAGS="$LFLAGS -Pmgd -olex.yy.c"


if test "$php_always_shared" = "yes"; then
	AC_DEFINE(MGD_INCLUDE_PHP_CONFIG, 1, [ ])
fi

AC_MSG_CHECKING(whether to compile for php with system regex)
AC_ARG_WITH(php-regex, [  --with-php-regex	configure for regex used in PHP ], 
	PHP_MIDGARD_REGEX=[$]withval, PHP_MIDGARD_REGEX="php")
AC_MSG_RESULT($PHP_MIDGARD_REGEX)
if test "$PHP_MIDGARD_REGEX" != "php"; then
	AC_DEFINE(MIDGARD_PHP_REGEX, 1, [ ])
fi

_PHP_MIDGARD_LEGACY_API=no
AC_MSG_CHECKING([whether to compile legacy API (disabled by default)])
AC_ARG_WITH(legacy-api, [  --with-legacy-api    compile with legacy API ], _PHP_MIDGARD_LEGACY_API=[$]withval, _PHP_MIDGARD_LEGACY_API="no")
AC_MSG_RESULT($_PHP_MIDGARD_LEGACY_API)
if test "$_PHP_MIDGARD_LEGACY_API" != "no"; then
	AC_DEFINE(PHP_MIDGARD_LEGACY_API, 1, [ ])
fi

MIDGARD_INCLINE=`$MIDGARD_CONFIG --cflags midgard`
MIDGARD_LIBLINE=`$MIDGARD_CONFIG --libs midgard`

PHP_EVAL_INCLINE($MIDGARD_INCLINE)
PHP_EVAL_LIBLINE($MIDGARD_LIBLINE, MIDGARD_SHARED_LIBADD)

CFLAGS="$CFLAGS -Wall -fno-strict-aliasing"

AC_DEFINE(HAVE_MIDGARD, 1, [ ])
PHP_SUBST(MIDGARD_SHARED_LIBADD)
PHP_NEW_EXTENSION(midgard, midgard.c preparser.c preparse.c query_builder.c php_midgard_reflection_property.c php_midgard_collector.c php_midgard_object_parameter.c php_midgard_object_attachment.c php_midgard_config.c php_midgard_gobject_generic.c php_midgard_blob.c php_midgard_object.c php_midgard_object_class.c php_midgard_connection.c article.c parameter.c attachment.c oop.c topic.c element.c group.c host.c member.c calendar.c event.c eventmember.c page.c pageelement.c pagelink.c person.c snippet.c snippetdir.c style.c sitegroup.c language.c quota.c php_midgard_replicator.c php_midgard_user.c php_midgard_dbus.c php_midgard_sitegroup.c, $ext_shared)
