srcdir = /home/pp/svn/ragnaroek/midgard/apis/php5
builddir = /home/pp/svn/ragnaroek/midgard/apis/php5
top_srcdir = /home/pp/svn/ragnaroek/midgard/apis/php5
top_builddir = /home/pp/svn/ragnaroek/midgard/apis/php5
EGREP = /bin/grep -E
SED = /bin/sed
CONFIGURE_COMMAND = './configure'
CONFIGURE_OPTIONS =
SHLIB_SUFFIX_NAME = so
SHLIB_DL_SUFFIX_NAME = so
ZEND_EXT_TYPE = zend_extension
RE2C = re2c
AWK = gawk
MIDGARD_SHARED_LIBADD = -lmidgard -lmysqlclient -lz -lm -ldl -lxml2 -ldbus-glib-1 -lssl -lcrypto -ldbus-1 -lgobject-2.0 -lglib-2.0
shared_objects_midgard = midgard.lo preparser.lo preparse.lo query_builder.lo php_midgard_reflection_property.lo php_midgard_collector.lo php_midgard_object_parameter.lo php_midgard_object_attachment.lo php_midgard_config.lo php_midgard_gobject_generic.lo php_midgard_blob.lo php_midgard_object.lo php_midgard_object_class.lo php_midgard_connection.lo article.lo parameter.lo attachment.lo oop.lo topic.lo element.lo group.lo host.lo member.lo calendar.lo event.lo eventmember.lo page.lo pageelement.lo pagelink.lo person.lo snippet.lo snippetdir.lo style.lo sitegroup.lo language.lo quota.lo php_midgard_replicator.lo php_midgard_user.lo php_midgard_dbus.lo php_midgard_sitegroup.lo
PHP_PECL_EXTENSION = midgard
PHP_MODULES = $(phplibdir)/midgard.la
PHP_ZEND_EX =
all_targets = $(PHP_MODULES) $(PHP_ZEND_EX)
install_targets = install-modules install-headers
prefix = /usr
exec_prefix = $(prefix)
libdir = ${exec_prefix}/lib
prefix = /usr
phplibdir = /home/pp/svn/ragnaroek/midgard/apis/php5/modules
phpincludedir = /usr/include/php5
CC = cc
CFLAGS = -g -O2 -Wall -fno-strict-aliasing
CFLAGS_CLEAN = $(CFLAGS)
CPP = cc -E
CPPFLAGS = -DHAVE_CONFIG_H
CXX =
CXXFLAGS =
CXXFLAGS_CLEAN = $(CXXFLAGS)
EXTENSION_DIR = /usr/lib/php5/20121212
PHP_EXECUTABLE = /usr/bin/php
EXTRA_LDFLAGS =
EXTRA_LIBS =
INCLUDES = -I/usr/include/php5 -I/usr/include/php5/main -I/usr/include/php5/TSRM -I/usr/include/php5/Zend -I/usr/include/php5/ext -I/usr/include/php5/ext/date/lib -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I/usr/include/glib-2.0 -I/usr/lib/i386-linux-gnu/glib-2.0/include -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/i386-linux-gnu/dbus-1.0/include -I/usr/include/midgard -I/usr/include/mysql
LFLAGS = -Pmgd -olex.yy.c
LDFLAGS =
SHARED_LIBTOOL =
LIBTOOL = $(SHELL) $(top_builddir)/libtool
SHELL = /bin/bash
INSTALL_HEADERS =
mkinstalldirs = $(top_srcdir)/build/shtool mkdir -p
INSTALL = $(top_srcdir)/build/shtool install -c
INSTALL_DATA = $(INSTALL) -m 644

DEFS = -DPHP_ATOM_INC -I$(top_builddir)/include -I$(top_builddir)/main -I$(top_srcdir)
COMMON_FLAGS = $(DEFS) $(INCLUDES) $(EXTRA_INCLUDES) $(CPPFLAGS) $(PHP_FRAMEWORKPATH)

all: $(all_targets) 
	@echo
	@echo "Build complete."
	@echo "Don't forget to run 'make test'."
	@echo

build-modules: $(PHP_MODULES) $(PHP_ZEND_EX)

build-binaries: $(PHP_BINARIES)

libphp$(PHP_MAJOR_VERSION).la: $(PHP_GLOBAL_OBJS) $(PHP_SAPI_OBJS)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -rpath $(phptempdir) $(EXTRA_LDFLAGS) $(LDFLAGS) $(PHP_RPATHS) $(PHP_GLOBAL_OBJS) $(PHP_SAPI_OBJS) $(EXTRA_LIBS) $(ZEND_EXTRA_LIBS) -o $@
	-@$(LIBTOOL) --silent --mode=install cp $@ $(phptempdir)/$@ >/dev/null 2>&1

libs/libphp$(PHP_MAJOR_VERSION).bundle: $(PHP_GLOBAL_OBJS) $(PHP_SAPI_OBJS)
	$(CC) $(MH_BUNDLE_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(PHP_GLOBAL_OBJS:.lo=.o) $(PHP_SAPI_OBJS:.lo=.o) $(PHP_FRAMEWORKS) $(EXTRA_LIBS) $(ZEND_EXTRA_LIBS) -o $@ && cp $@ libs/libphp$(PHP_MAJOR_VERSION).so

install: $(all_targets) $(install_targets)

install-sapi: $(OVERALL_TARGET)
	@echo "Installing PHP SAPI module:       $(PHP_SAPI)"
	-@$(mkinstalldirs) $(INSTALL_ROOT)$(bindir)
	-@if test ! -r $(phptempdir)/libphp$(PHP_MAJOR_VERSION).$(SHLIB_DL_SUFFIX_NAME); then \
		for i in 0.0.0 0.0 0; do \
			if test -r $(phptempdir)/libphp$(PHP_MAJOR_VERSION).$(SHLIB_DL_SUFFIX_NAME).$$i; then \
				$(LN_S) $(phptempdir)/libphp$(PHP_MAJOR_VERSION).$(SHLIB_DL_SUFFIX_NAME).$$i $(phptempdir)/libphp$(PHP_MAJOR_VERSION).$(SHLIB_DL_SUFFIX_NAME); \
				break; \
			fi; \
		done; \
	fi
	@$(INSTALL_IT)

install-binaries: build-binaries $(install_binary_targets)

install-modules: build-modules
	@test -d modules && \
	$(mkinstalldirs) $(INSTALL_ROOT)$(EXTENSION_DIR)
	@echo "Installing shared extensions:     $(INSTALL_ROOT)$(EXTENSION_DIR)/"
	@rm -f modules/*.la >/dev/null 2>&1
	@$(INSTALL) modules/* $(INSTALL_ROOT)$(EXTENSION_DIR)

install-headers:
	-@if test "$(INSTALL_HEADERS)"; then \
		for i in `echo $(INSTALL_HEADERS)`; do \
			i=`$(top_srcdir)/build/shtool path -d $$i`; \
			paths="$$paths $(INSTALL_ROOT)$(phpincludedir)/$$i"; \
		done; \
		$(mkinstalldirs) $$paths && \
		echo "Installing header files:          $(INSTALL_ROOT)$(phpincludedir)/" && \
		for i in `echo $(INSTALL_HEADERS)`; do \
			if test "$(PHP_PECL_EXTENSION)"; then \
				src=`echo $$i | $(SED) -e "s#ext/$(PHP_PECL_EXTENSION)/##g"`; \
			else \
				src=$$i; \
			fi; \
			if test -f "$(top_srcdir)/$$src"; then \
				$(INSTALL_DATA) $(top_srcdir)/$$src $(INSTALL_ROOT)$(phpincludedir)/$$i; \
			elif test -f "$(top_builddir)/$$src"; then \
				$(INSTALL_DATA) $(top_builddir)/$$src $(INSTALL_ROOT)$(phpincludedir)/$$i; \
			else \
				(cd $(top_srcdir)/$$src && $(INSTALL_DATA) *.h $(INSTALL_ROOT)$(phpincludedir)/$$i; \
				cd $(top_builddir)/$$src && $(INSTALL_DATA) *.h $(INSTALL_ROOT)$(phpincludedir)/$$i) 2>/dev/null || true; \
			fi \
		done; \
	fi

PHP_TEST_SETTINGS = -d 'open_basedir=' -d 'output_buffering=0' -d 'memory_limit=-1'
PHP_TEST_SHARED_EXTENSIONS =  ` \
	if test "x$(PHP_MODULES)" != "x"; then \
		for i in $(PHP_MODULES)""; do \
			. $$i; $(top_srcdir)/build/shtool echo -n -- " -d extension=$$dlname"; \
		done; \
	fi; \
	if test "x$(PHP_ZEND_EX)" != "x"; then \
		for i in $(PHP_ZEND_EX)""; do \
			. $$i; $(top_srcdir)/build/shtool echo -n -- " -d $(ZEND_EXT_TYPE)=$(top_builddir)/modules/$$dlname"; \
		done; \
	fi`
PHP_DEPRECATED_DIRECTIVES_REGEX = '^(magic_quotes_(gpc|runtime|sybase)?|(zend_)?extension(_debug)?(_ts)?)[\t\ ]*='

test: all
	@if test ! -z "$(PHP_EXECUTABLE)" && test -x "$(PHP_EXECUTABLE)"; then \
		INI_FILE=`$(PHP_EXECUTABLE) -d 'display_errors=stderr' -r 'echo php_ini_loaded_file();' 2> /dev/null`; \
		if test "$$INI_FILE"; then \
			$(EGREP) -h -v $(PHP_DEPRECATED_DIRECTIVES_REGEX) "$$INI_FILE" > $(top_builddir)/tmp-php.ini; \
		else \
			echo > $(top_builddir)/tmp-php.ini; \
		fi; \
		INI_SCANNED_PATH=`$(PHP_EXECUTABLE) -d 'display_errors=stderr' -r '$$a = explode(",\n", trim(php_ini_scanned_files())); echo $$a[0];' 2> /dev/null`; \
		if test "$$INI_SCANNED_PATH"; then \
			INI_SCANNED_PATH=`$(top_srcdir)/build/shtool path -d $$INI_SCANNED_PATH`; \
			$(EGREP) -h -v $(PHP_DEPRECATED_DIRECTIVES_REGEX) "$$INI_SCANNED_PATH"/*.ini >> $(top_builddir)/tmp-php.ini; \
		fi; \
		TEST_PHP_EXECUTABLE=$(PHP_EXECUTABLE) \
		TEST_PHP_SRCDIR=$(top_srcdir) \
		CC="$(CC)" \
			$(PHP_EXECUTABLE) -n -c $(top_builddir)/tmp-php.ini $(PHP_TEST_SETTINGS) $(top_srcdir)/run-tests.php -n -c $(top_builddir)/tmp-php.ini -d extension_dir=$(top_builddir)/modules/ $(PHP_TEST_SHARED_EXTENSIONS) $(TESTS); \
		TEST_RESULT_EXIT_CODE=$$?; \
		rm $(top_builddir)/tmp-php.ini; \
		exit $$TEST_RESULT_EXIT_CODE; \
	else \
		echo "ERROR: Cannot run tests without CLI sapi."; \
	fi

clean:
	find . -name \*.gcno -o -name \*.gcda | xargs rm -f
	find . -name \*.lo -o -name \*.o | xargs rm -f
	find . -name \*.la -o -name \*.a | xargs rm -f 
	find . -name \*.so | xargs rm -f
	find . -name .libs -a -type d|xargs rm -rf
	rm -f libphp$(PHP_MAJOR_VERSION).la $(SAPI_CLI_PATH) $(SAPI_CGI_PATH) $(SAPI_MILTER_PATH) $(SAPI_LITESPEED_PATH) $(SAPI_FPM_PATH) $(OVERALL_TARGET) modules/* libs/*

distclean: clean
	rm -f Makefile config.cache config.log config.status Makefile.objects Makefile.fragments libtool main/php_config.h main/internal_functions_cli.c main/internal_functions.c stamp-h sapi/apache/libphp$(PHP_MAJOR_VERSION).module sapi/apache_hooks/libphp$(PHP_MAJOR_VERSION).module buildmk.stamp Zend/zend_dtrace_gen.h Zend/zend_dtrace_gen.h.bak Zend/zend_config.h TSRM/tsrm_config.h
	rm -f php5.spec main/build-defs.h scripts/phpize
	rm -f ext/date/lib/timelib_config.h ext/mbstring/oniguruma/config.h ext/mbstring/libmbfl/config.h ext/mysqlnd/php_mysqlnd_config.h
	rm -f scripts/man1/phpize.1 scripts/php-config scripts/man1/php-config.1 sapi/cli/php.1 sapi/cgi/php-cgi.1 ext/phar/phar.1 ext/phar/phar.phar.1
	rm -f sapi/fpm/php-fpm.conf sapi/fpm/init.d.php-fpm sapi/fpm/php-fpm.service sapi/fpm/php-fpm.8 sapi/fpm/status.html
	rm -f ext/iconv/php_have_bsd_iconv.h ext/iconv/php_have_glibc_iconv.h ext/iconv/php_have_ibm_iconv.h ext/iconv/php_have_iconv.h ext/iconv/php_have_libiconv.h ext/iconv/php_iconv_aliased_libiconv.h ext/iconv/php_iconv_supports_errno.h ext/iconv/php_php_iconv_h_path.h ext/iconv/php_php_iconv_impl.h
	rm -f ext/phar/phar.phar ext/phar/phar.php
	if test "$(srcdir)" != "$(builddir)"; then \
	  rm -f ext/phar/phar/phar.inc; \
	fi
	$(EGREP) define'.*include/php' $(top_srcdir)/configure | $(SED) 's/.*>//'|xargs rm -f

.PHONY: all clean install distclean test
.NOEXPORT:
midgard.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/midgard.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/midgard.c -o midgard.lo 
preparser.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/preparser.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/preparser.c -o preparser.lo 
preparse.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/preparse.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/preparse.c -o preparse.lo 
query_builder.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/query_builder.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/query_builder.c -o query_builder.lo 
php_midgard_reflection_property.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_reflection_property.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_reflection_property.c -o php_midgard_reflection_property.lo 
php_midgard_collector.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_collector.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_collector.c -o php_midgard_collector.lo 
php_midgard_object_parameter.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_object_parameter.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_object_parameter.c -o php_midgard_object_parameter.lo 
php_midgard_object_attachment.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_object_attachment.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_object_attachment.c -o php_midgard_object_attachment.lo 
php_midgard_config.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_config.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_config.c -o php_midgard_config.lo 
php_midgard_gobject_generic.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_gobject_generic.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_gobject_generic.c -o php_midgard_gobject_generic.lo 
php_midgard_blob.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_blob.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_blob.c -o php_midgard_blob.lo 
php_midgard_object.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_object.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_object.c -o php_midgard_object.lo 
php_midgard_object_class.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_object_class.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_object_class.c -o php_midgard_object_class.lo 
php_midgard_connection.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_connection.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_connection.c -o php_midgard_connection.lo 
article.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/article.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/article.c -o article.lo 
parameter.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/parameter.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/parameter.c -o parameter.lo 
attachment.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/attachment.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/attachment.c -o attachment.lo 
oop.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/oop.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/oop.c -o oop.lo 
topic.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/topic.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/topic.c -o topic.lo 
element.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/element.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/element.c -o element.lo 
group.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/group.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/group.c -o group.lo 
host.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/host.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/host.c -o host.lo 
member.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/member.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/member.c -o member.lo 
calendar.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/calendar.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/calendar.c -o calendar.lo 
event.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/event.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/event.c -o event.lo 
eventmember.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/eventmember.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/eventmember.c -o eventmember.lo 
page.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/page.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/page.c -o page.lo 
pageelement.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/pageelement.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/pageelement.c -o pageelement.lo 
pagelink.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/pagelink.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/pagelink.c -o pagelink.lo 
person.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/person.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/person.c -o person.lo 
snippet.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/snippet.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/snippet.c -o snippet.lo 
snippetdir.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/snippetdir.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/snippetdir.c -o snippetdir.lo 
style.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/style.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/style.c -o style.lo 
sitegroup.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/sitegroup.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/sitegroup.c -o sitegroup.lo 
language.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/language.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/language.c -o language.lo 
quota.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/quota.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/quota.c -o quota.lo 
php_midgard_replicator.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_replicator.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_replicator.c -o php_midgard_replicator.lo 
php_midgard_user.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_user.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_user.c -o php_midgard_user.lo 
php_midgard_dbus.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_dbus.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_dbus.c -o php_midgard_dbus.lo 
php_midgard_sitegroup.lo: /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_sitegroup.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/pp/svn/ragnaroek/midgard/apis/php5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/pp/svn/ragnaroek/midgard/apis/php5/php_midgard_sitegroup.c -o php_midgard_sitegroup.lo 
$(phplibdir)/midgard.la: ./midgard.la
	$(LIBTOOL) --mode=install cp ./midgard.la $(phplibdir)

./midgard.la: $(shared_objects_midgard) $(MIDGARD_SHARED_DEPENDENCIES)
	$(LIBTOOL) --mode=link $(CC) $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ -export-dynamic -avoid-version -prefer-pic -module -rpath $(phplibdir) $(EXTRA_LDFLAGS) $(shared_objects_midgard) $(MIDGARD_SHARED_LIBADD)

