/* $Id: php_midgard.h 27410 2014-09-01 07:39:28Z piotras $
Copyright (C) 1999 Jukka Zitting <jukka.zitting@iki.fi>
Copyright (C) 2000 The Midgard Project ry
Copyright (C) 2000 Emile Heyns, Aurora SA <emile@iris-advies.com>

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef G_LOG_DOMAIN
# undef G_LOG_DOMAIN
# define G_LOG_DOMAIN "midgard-core"
#endif

#include <midgard/midgard.h>

extern guint global_loghandler;

#ifndef PHP_MIDGARD_H
#define PHP_MIDGARD_H

#include "config.h"

#ifdef PHP_WIN32
# include "config.w32.h"
#endif

#ifndef PHP_CONFIG_H
# define PHP_CONFIG_H
# include "php_config.h"
#endif

/* Do not load Apache regex when php compiled with 'system' as regex's TYPE */
#if REGEX == 0
# ifndef _REGEX_H
#  define _REGEX_H 1
# endif
#endif

#include "php.h"
#include "php_globals.h"
#include <zend_interfaces.h>
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "SAPI.h"
//#include "ext/session/php_session.h"
#include "ext/standard/php_standard.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/info.h"

#if HAVE_MIDGARD

# include <midgard/midgard.h>
# include "php_midgard_object.h"
# include "mgd_internal.h"
# include "mgd_oop.h"
# include <midgard/midgard_apache.h>

#ifdef ZTS
# include "TSRM.h"
#endif

/* PHP > 5.4 doesn't provide function_entry */
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
# define php_midgard_function_entry zend_function_entry
#else
# define php_midgard_function_entry function_entry
#endif

#define MIDGARD_GLOBAL_MIDGARD			"_MIDGARD"
#define MIDGARD_GLOBAL_MIDCOM			"_MIDCOM"
#define MIDGARD_GLOBAL_MIDGARD_CONNECTION	"_MIDGARD_CONNECTION"

/* LEGACY WORKAROUNDS */
midgard_request_config *mgd_rcfg();
midgard_directory_config *mgd_dcfg();
midgard *mgd_handle();
extern void mgd_handle_set(midgard *mgd);
extern void mgd_handle_singleton_set(MidgardConnection *mgd);
extern MidgardConnection * mgd_handle_singleton_get();
void _make_midgard_global();
int mgd_get_errno();
void mgd_reset_errno();
void mgd_set_errno(MgdErrorGeneric mgd_errno);
int is_table_multilang(const char *table);
void mgd_php_store_elt (const char *name, const char *value, void *userdata);

extern zend_module_entry midgard_module_entry;
extern void php_midgard_register_auto_globals(void); 

# ifdef ZTS
typedef struct {

  midgard_request_config *rcfg;
  midgard_directory_config *dcfg;
  midgard *mgd;
  int mgd_errno;   
  MidgardConnection *midgard_singleton;
  
} midgard_globals;
# endif

# define phpext_midgard_ptr &midgard_module_entry

# ifdef PHP_WIN32
#  define PHP_MIDGARD_API __declspec(dllexport)
# else
#  define PHP_MIDGARD_API
# endif

PHP_MINIT_FUNCTION(midgard);
PHP_MSHUTDOWN_FUNCTION(midgard);
PHP_RINIT_FUNCTION(midgard);
PHP_RSHUTDOWN_FUNCTION(midgard);
PHP_MINFO_FUNCTION(midgard);

PHP_FUNCTION(confirm_midgard_compiled);	/* For testing, remove later. */

ZEND_BEGIN_MODULE_GLOBALS(midgard)
	midgard_request_config *rcfg;
	midgard_directory_config *dcfg;
	midgard *mgd;
	int mgd_errno;
	MidgardConnection *midgard_singleton;
ZEND_END_MODULE_GLOBALS(midgard)

/* ZEND_EXTERN_MODULE_GLOBALS(midgard)  */

# ifdef ZTS
#  define MGDG(v) TSRMG(midgard_globals_id, zend_midgard_globals *, v)
# else
#  define MGDG(v) (midgard_globals.v)
# endif

#else
# define phpext_midgard_ptr NULL
#endif

MGD_FUNCTION(ret_type, import_object, (type param));
MGD_FUNCTION(ret_type, get_sitegroup_size, (type param));

#ifndef PHP_MIDGARD_LEGACY_API
PHP_FUNCTION(mgd_get_midgard);
PHP_FUNCTION(mgd_preparse);
PHP_FUNCTION(mgd_snippet);
PHP_FUNCTION(mgd_errstr);
PHP_FUNCTION(mgd_errno);
PHP_FUNCTION(mgd_version);
PHP_FUNCTION(mgd_auth_midgard);
PHP_FUNCTION(mgd_unsetuid);
PHP_FUNCTION(mgd_issetuid);
PHP_FUNCTION(mgd_is_guid);
PHP_FUNCTION(mgd_format);
PHP_FUNCTION(mgd_set_lang);
PHP_FUNCTION(mgd_get_lang);
PHP_FUNCTION(mgd_set_default_lang);
PHP_FUNCTION(mgd_get_default_lang);
PHP_FUNCTION(mgd_debug_start);
PHP_FUNCTION(mgd_debug_stop);
PHP_FUNCTION(mgd_config_init);
PHP_FUNCTION(mgd_cache_invalidate);
PHP_FUNCTION(mgd_show_element);
PHP_FUNCTION(mgd_template);
PHP_FUNCTION(mgd_set_errno);
PHP_FUNCTION(mgd_stat_attachment);

extern MidgardClass MidgardAttachment;
extern MidgardClass midgardsitegroup;

#endif

#define MGD_PHP_PCLASS_NAME \
  ce = Z_OBJCE_P(getThis()); \
  if(ce && ce->parent) {\
    do {\
      ce = ce->parent; \
    }while(ce->parent); \
  } 

extern void php_midgard_log_errors(const gchar *domain, GLogLevelFlags level,
	const gchar *msg, gpointer userdata);

/* MidgardConnection */
void php_midgard_connection_connect_callbacks(MidgardConnection *mgd);
void php_midgard_connection_disconnect_callbacks(MidgardConnection *mgd);

/* Underlying GObject bindings */
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
zval *php_midgard_gobject_read_property(zval *zobject, zval *prop, int type, const zend_literal *key TSRMLS_DC);
int php_midgard_gobject_has_property(zval *zobject, zval *prop, int type, const zend_literal *key TSRMLS_DC);
void php_midgard_gobject_write_property(zval *zobject, zval *prop, zval *value, const zend_literal *key TSRMLS_DC);
#else
zval *php_midgard_gobject_read_property(zval *zobject, zval *prop, int type TSRMLS_DC);
int php_midgard_gobject_has_property(zval *zobject, zval *prop, int type TSRMLS_DC);
void php_midgard_gobject_write_property(zval *zobject, zval *prop, zval *value TSRMLS_DC);
#endif
void php_midgard_zendobject_register_properties (zval *zobject, GObject *gobject);
HashTable *php_midgard_zendobject_get_properties (zval *zobject TSRMLS_DC);

extern MgdObject *php_midgard_get_midgard_object(zval *zobj);
extern void php_midgard_array_from_objects(GObject **objects, const gchar *class_name, zval *zarray);

extern GValue *php_midgard_zval2gvalue(zval *zvalue);
extern gboolean php_midgard_gvalue2zval(GValue *gvalue, zval *zvalue);

extern GParameter *php_midgard_array_to_gparameter(zval *params, guint *n_params);

extern zend_class_entry *php_midgard_get_baseclass_ptr_by_name(const gchar *name);
extern gboolean php_midgard_is_derived_from_class(const gchar *classname, 
	GType basetype, gboolean check_parent, zend_class_entry **base_class TSRMLS_DC);

#define PHP_MIDGARD_PARSE_CLASS_ARGUMENT(__name, __type, __check_parent, __base_ce) { \
	gboolean __isderived = \
		php_midgard_is_derived_from_class(__name, \
			__type, __check_parent, __base_ce TSRMLS_CC); \
	if (!__isderived) { \
		php_error(E_WARNING, "Expected %s derived class", g_type_name(__type)); \
		php_midgard_error_exception_force_throw(mgd_handle(), MGD_ERR_INVALID_OBJECT); \
		WRONG_PARAM_COUNT \
	} \
}

/* closures */
void php_midgard_object_connect_class_closures(GObject *object, zval *zobject);
void php_midgard_gobject_closure_hash_new();
void php_midgard_gobject_closure_hash_reset();
void php_midgard_gobject_closure_hash_free();

zend_class_entry *midgard_php_register_internal_class(const gchar *class_name, GType class_type, zend_class_entry ce);

/* Base midgard-core classes */
extern void php_midgard_collector_init(int module_number);
extern void php_midgard_replicator_init(int module_number);
extern void php_midgard_object_class_init(int module_number);
extern void php_midgard_blob_init(int module_number);
extern void php_midgard_connection_init(int module_number);
extern void php_midgard_config_init(int module_number);
extern void php_midgard_object_init(int module_number);
extern void php_midgard_query_builder_init(int module_number);
extern void php_midgard_reflection_property_init(int module_number);
extern void php_midgard_user_init(int module_number);
extern void php_midgard_dbus_init(int module_numer);
extern void php_midgard_sitegroup_init(int module_number);

/* Exceptions */
extern gboolean php_midgard_error_exception_throw(midgard *mgd);
extern void php_midgard_error_exception_force_throw(midgard *mgd, gint errcode);

#define _MIDGARD_UPDATE_LONG(key, val) \
    if(zend_hash_find(&EG(symbol_table), "_MIDGARD",\
                sizeof("_MIDGARD"), (void **) &hash) == SUCCESS &&\
            Z_TYPE_PP(hash) == IS_ARRAY &&\
            zend_hash_find(Z_ARRVAL_PP(hash), key,\
                strlen(key)+1, (void **) &vkey) == SUCCESS) {\
        convert_to_long_ex(vkey); \
        (*vkey)->value.lval = val; \
        }\

#define NOT_STATIC_METHOD() \
	if (!getThis()) { \
		php_error(E_WARNING, \
			"%s() is not a static method", \
			get_active_function_name(TSRMLS_C)); \
		return; \
	}

zend_class_entry *php_midgard_get_baseclass_ptr(zend_class_entry *ce);
zend_class_entry *php_midgard_get_baseclass_ptr_by_name(const gchar *name);

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3 
#define CHECK_MGD \
	if (!mgd_handle()) \
		php_error(E_ERROR, "Can not find MidgardConnection"); \
	mgd_reset_errno(); \
	const char *_check_cname_space = NULL; \
	const char *_check_class_name = get_active_class_name(&_check_cname_space TSRMLS_CC); \
	g_log("midgard-core", G_LOG_LEVEL_INFO, \
		" %s%s%s(...)", _check_class_name, _check_cname_space, \
		get_active_function_name(TSRMLS_C));
#else
#define CHECK_MGD \
	if (!mgd_handle()) \
		php_error(E_ERROR, "Can not find MidgardConnection"); \
	mgd_reset_errno(); \
	char *_check_cname_space = NULL; \
	const char *_check_class_name = get_active_class_name(&_check_cname_space TSRMLS_CC); \
	g_log("midgard-core", G_LOG_LEVEL_INFO, \
		" %s%s%s(...)", _check_class_name, _check_cname_space, \
		get_active_function_name(TSRMLS_C));
#endif


#endif	/* PHP_MIDGARD_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
