/* Copyright (C) 2007 Piotr Pokora <piotrek.pokora@gmail.com>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "php_midgard.h"
#include "php_midgard_gobject.h"

#include "php_midgard__helpers.h"

static zend_class_entry *php_midgard_connection_class;

static MidgardConnection *__midgard_connection_get_ptr(void)
{	
	zval **_mcg;
	TSRMLS_FETCH();

	if(zend_hash_find(&EG(symbol_table), "_MIDGARD_CONNECTION",
				sizeof("_MIDGARD_CONNECTION"), (void **) &_mcg) == FAILURE )
	{
		php_error(E_ERROR, "Can not find underlying MidgardConnection object");

	} else {

		if(Z_TYPE_PP(_mcg) != IS_OBJECT)
			php_error(E_ERROR, "_MIDGARD_CONNECTION superglobal is not an object");

		MidgardConnection *cnc = mgd_handle_singleton_get();

		if(cnc == NULL)
			php_error(E_ERROR, "MidgardConnection underlying GObject is NULL");

		return cnc;
	}

	return NULL;
}

/* Legacy */
static zval **__get_midgard_ptr(void)
{
	zval **_midgard;
	TSRMLS_FETCH();

	if(zend_hash_find(&EG(symbol_table), "_MIDGARD",
				sizeof("_MIDGARD"), (void **) &_midgard) == FAILURE )
	{
		php_error(E_ERROR, "Can not find _MIDGARD superglobal");

	} else {

		if(Z_TYPE_PP(_midgard) != IS_ARRAY)
			php_error(E_ERROR, "_MIDGARD superglobal is not an array");

		return _midgard;
	}

	return NULL;
}

/* MidgardConnection callbacks ( to update _MIDGARD ) */
static void __sitegroup_change_callback(MidgardConnection *mgd, gpointer ud)
{
	zval **_midgard = __get_midgard_ptr();

	if (!_midgard)
		return;

	zval *sgid;
	MAKE_STD_ZVAL(sgid);
	ZVAL_LONG(sgid, mgd_sitegroup(mgd_handle()));

	zend_hash_update(Z_ARRVAL_PP(_midgard), 
			"sitegroup", sizeof("sitegroup"),
			&sgid, sizeof(zval *), NULL);
}

static void __lang_change_callback(MidgardConnection *mgd, gpointer ud)
{
	zval **_midgard = __get_midgard_ptr();

	if (!_midgard)
		return;

	zval *langid;
	MAKE_STD_ZVAL(langid);
	ZVAL_LONG(langid, mgd_lang(mgd_handle()));

	zend_hash_update(Z_ARRVAL_PP(_midgard), 
			"lang", sizeof("lang"),
			&langid, sizeof(zval *), NULL);
}

static void __user_change_callback(MidgardConnection *mgd, gpointer ud)
{
	zval **_midgard = __get_midgard_ptr();

	MidgardUser *user = midgard_connection_get_user(mgd_handle()->_mgd);

	/* There's no user associated with connection, so return. */
	if(!user)
		return;

	MgdObject *person = midgard_user_get_person(user);

	/* TODO, we should handle such case here. 
	 * There should be person (always) if user is logged in.
	 * Though, I have no idea how to handle it as it's a callback. */
	if(person == NULL)
		return;

	if(person != NULL) {
		
		GValue pval = {0, };
		g_value_init(&pval, G_TYPE_UINT);
		g_object_get_property(G_OBJECT(person), "id", &pval);	
		zval *pid;
		MAKE_STD_ZVAL(pid);
		php_midgard_gvalue2zval(&pval, pid);
		g_value_unset(&pval);
		zend_hash_update(Z_ARRVAL_PP(_midgard),
				"user", sizeof("user"),
				&pid, sizeof(zval *), NULL);
	}

	zval *isroot;
	MAKE_STD_ZVAL(isroot);
	ZVAL_LONG(isroot, midgard_user_is_root(user));
	
	zend_hash_update(Z_ARRVAL_PP(_midgard),
			"root", sizeof("root"),
			&isroot, sizeof(zval *), NULL);
	
	zval *isadmin;
	MAKE_STD_ZVAL(isadmin);
	ZVAL_LONG(isadmin, midgard_user_is_admin(user));
	
	zend_hash_update(Z_ARRVAL_PP(_midgard),
			"admin", sizeof("admin"),
			&isadmin, sizeof(zval *), NULL);
	zval *sgid;
	MAKE_STD_ZVAL(sgid);
	ZVAL_LONG(sgid, mgd_sitegroup(mgd_handle()));

	zend_hash_update(Z_ARRVAL_PP(_midgard), 
			"sitegroup", sizeof("sitegroup"),
			&sgid, sizeof(zval *), NULL);
}

void php_midgard_connection_connect_callbacks(MidgardConnection *mgd)
{
	g_assert(mgd != NULL);

	g_signal_connect(G_OBJECT(mgd), "sitegroup-changed",
			G_CALLBACK(__sitegroup_change_callback), NULL);
	g_signal_connect(G_OBJECT(mgd), "auth-changed",
			G_CALLBACK(__user_change_callback), NULL);
	g_signal_connect(G_OBJECT(mgd), "lang-changed",
			G_CALLBACK(__lang_change_callback), NULL);
}

void php_midgard_connection_disconnect_callbacks(MidgardConnection *mgd)
{	
	g_assert(mgd != NULL);

	if(!G_IS_OBJECT(mgd))
		return;

	g_object_disconnect(G_OBJECT(mgd), "any_signal", 
			G_CALLBACK(__user_change_callback), NULL, NULL);
	g_object_disconnect(G_OBJECT(mgd), "any_signal", 
			G_CALLBACK(__lang_change_callback), NULL, NULL);
	g_object_disconnect(G_OBJECT(mgd), "any_signal", 
			G_CALLBACK(__sitegroup_change_callback), NULL, NULL);

	return;
}

/* Object constructor */
static PHP_METHOD(midgard_connection, __construct)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = NULL;
	zval *zval_object = getThis();
	zval *_mcg;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) 
		return;	

	/* Check if _MIDGARD_CONNECTION is already set */
	if(zend_hash_find(&EG(symbol_table), "_MIDGARD_CONNECTION",
				sizeof("_MIDGARD_CONNECTION"), (void **) &_mcg) == SUCCESS )
	{
		g_warning("_MIDGARD_CONNECTION superglobal (singleton) already initialized");
		/* FIXME , throw an exception ? */
		return;
	}

	mgd = midgard_connection_new();

	if(!mgd)
		return;

	if (global_loghandler) {

		g_log_remove_handler("midgard-core", global_loghandler);
		midgard_connection_set_loglevel(mgd, "warning", NULL);
		global_loghandler = midgard_connection_get_loghandler(mgd);	
	}

	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);

	php_gobject->gobject = G_OBJECT(mgd);

	mgd_handle_singleton_set(mgd);
	
	php_midgard_connection_connect_callbacks(mgd);

	if(zend_hash_find(&EG(symbol_table), "_MIDGARD_CONNECTION",
				sizeof("_MIDGARD_CONNECTION"), (void **) &_mcg) != SUCCESS )
	{
		_mcg = zval_object;
		zval_add_ref(&_mcg);	
		zend_hash_update(&EG(symbol_table), "_MIDGARD_CONNECTION", 
				sizeof("_MIDGARD_CONNECTION"), 
				(void **)&_mcg, sizeof(zval *), NULL);
	}
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection___construct, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, open)
{
	RETVAL_FALSE;
	gchar *cnf_name;
	guint cnf_name_length;
	gboolean rv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
				"s", &cnf_name, &cnf_name_length) == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();

	rv = midgard_connection_open(mgd, (const gchar *)cnf_name, NULL);

	if(rv) {
		
		mgd_handle_set(mgd->mgd);
		_make_midgard_global();

		guint loghandler = midgard_connection_get_loghandler(mgd);
		if(loghandler)
			g_log_remove_handler(G_LOG_DOMAIN, loghandler); 

		global_loghandler = 
			g_log_set_handler("midgard-core", G_LOG_LEVEL_MASK,
					php_midgard_log_errors, (gpointer)mgd);
		midgard_connection_set_loghandler(mgd, global_loghandler);
	}

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_open, 0, 0, 1)
        ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, open_config)
{
	RETVAL_FALSE;
	zval *cnf_object;
	gboolean rv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"z", &cnf_object) == FAILURE)
		return;
	
	MidgardConnection *mgd =__midgard_connection_get_ptr();
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(cnf_object TSRMLS_CC);
	
	MidgardConfig *config = MIDGARD_CONFIG(php_gobject->gobject);
	
	rv = midgard_connection_open_config(mgd, config, NULL);

	if(rv) {
		mgd_handle_set(mgd->mgd);
		_make_midgard_global();
	
		guint loghandler = midgard_connection_get_loghandler(mgd);
		if(loghandler)
			g_log_remove_handler(G_LOG_DOMAIN, loghandler); 

		global_loghandler = 
			g_log_set_handler("midgard-core", G_LOG_LEVEL_MASK,
					php_midgard_log_errors, (gpointer)mgd);
		midgard_connection_set_loghandler(mgd, global_loghandler);
	}

	RETURN_BOOL(rv);		
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_open_config, 0, 0, 1)
        ZEND_ARG_OBJ_INFO(0, config, midgard_config, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, set_sitegroup)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *name;
	guint name_length;
	gboolean rv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"s", &name, &name_length) == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();

	rv = midgard_connection_set_sitegroup(mgd, (const gchar *)name);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_set_sitegroup, 0, 0, 1)
        ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, get_sitegroup)
{
	RETVAL_NULL();
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;
	
	MidgardConnection *mgd =__midgard_connection_get_ptr();

	const gchar *name = 
		midgard_connection_get_sitegroup(mgd);

	if(name != NULL)
		RETURN_STRING((gchar *)name, 1);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_get_sitegroup, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, set_lang)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *lang;
	guint lang_length;
	gboolean rv;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
				"s", &lang, &lang_length) == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();

	rv = midgard_connection_set_lang(mgd, (const gchar *) lang);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_set_lang, 0, 0, 1)
        ZEND_ARG_INFO(0, langcode)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, get_lang)
{
	RETVAL_NULL();
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();
	
	const gchar *lang =
		midgard_connection_get_lang(mgd);
	
	if(lang != NULL)
		RETURN_STRING((gchar *)lang, 1);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_get_lang, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, set_default_lang)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *lang;
	guint lang_length;
	gboolean rv;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
				"s", &lang, &lang_length) == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();

	rv = midgard_connection_set_default_lang(mgd, (const gchar *) lang);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_set_default_lang, 0, 0, 1)
        ZEND_ARG_INFO(0, langcode)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, get_default_lang)
{
	RETVAL_NULL();
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();
	
	const gchar *lang =
		midgard_connection_get_default_lang(mgd);
	
	if(lang != NULL)
		RETURN_STRING((gchar *)lang, 1);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_get_default_lang, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, get_error)
{
	/* Disable debug log for function call.
	 * It resets error. Keep it like this for backward compatibility */
	/* CHECK_MGD; */ 

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();

	RETURN_LONG(midgard_connection_get_error(mgd));	
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_get_error, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, set_error)
{
	CHECK_MGD;
	gint errcode;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &errcode) == FAILURE)
		return;

	if(errcode > 0) {

		g_warning("Invalid not negative errcode value");
		return;
	}

	MidgardConnection *mgd =__midgard_connection_get_ptr();
	midgard_connection_set_error(mgd, errcode);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_set_error, 0, 0, 1)
        ZEND_ARG_INFO(0, errorcode)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, get_error_string)
{
	RETVAL_NULL();
	/* Disable debug log for function call.
	 * It resets error. Keep it like this for backward compatibility */
	/* CHECK_MGD; */ 

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();

	RETURN_STRING((gchar *)midgard_connection_get_error_string(mgd), 1);	
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_get_error_string, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, get_user)
{
	RETVAL_NULL();
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();	
	MidgardUser *user = midgard_connection_get_user(mgd);

	if(user == NULL)
		RETURN_NULL();

	zend_class_entry *user_ce =
		php_midgard_get_baseclass_ptr_by_name("midgard_user");
	
	php_midgard_gobject_new_with_gobject(return_value, user_ce, G_OBJECT(user), TRUE);

	Z_SET_ISREF_P(return_value);
	zval_add_ref(&return_value);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_get_user, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, set_loglevel)
{
	RETVAL_NULL();
	CHECK_MGD;
	const gchar *level;
	guint level_length;
	zval *callback;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z",
				&level, &level_length,
				&callback) == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();

	/* no support for callback atm */
	gboolean rv = midgard_connection_set_loglevel(mgd, level, php_midgard_log_errors);
	global_loghandler = midgard_connection_get_loghandler(mgd);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_set_loglevel, 0, 0, 1)
        ZEND_ARG_INFO(0, level)
	ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, connect)
{
	CHECK_MGD;
	
	php_midgard_gobject_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_connect, 0, 0, 2)
        ZEND_ARG_INFO(0, signal)
        ZEND_ARG_INFO(0, callback)
        ZEND_ARG_INFO(0, userdata)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, enable_dbus)
{
	CHECK_MGD;
	zend_bool toggle;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &toggle) == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();

	midgard_connection_enable_dbus (mgd, toggle);
	return;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_enable_dbus, 0, 0, 1)
        ZEND_ARG_INFO(0, toggle)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, enable_replication)
{
	CHECK_MGD;
	zend_bool toggle;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &toggle) == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();

	midgard_connection_enable_replication (mgd, toggle);
	return;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_enable_replication, 0, 0, 1)
        ZEND_ARG_INFO(0, toggle)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, enable_quota)
{
	CHECK_MGD;
	zend_bool toggle;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &toggle) == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();

	midgard_connection_enable_quota (mgd, toggle);
	return;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_enable_quota, 0, 0, 1)
        ZEND_ARG_INFO(0, toggle)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, is_enabled_dbus)
{
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();
	if (midgard_connection_is_enabled_dbus (mgd))
		RETURN_TRUE;

	RETURN_FALSE;
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_is_enabled_dbus, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, is_enabled_replication)
{
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();
	if (midgard_connection_is_enabled_replication (mgd))
		RETURN_TRUE;

	RETURN_FALSE;
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_is_enabled_replication, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, is_enabled_quota)
{
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr();
	if (midgard_connection_is_enabled_quota (mgd))
		RETURN_TRUE;

	RETURN_FALSE;
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_is_enabled_quota, 0)
ZEND_END_ARG_INFO()

int __serialize_cnc_hook(zval *zobject,
		unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC)
{
	g_warning("Unable to serialize midgard_connection object");
	return FAILURE;
}

int __unserialize_cnc_hook(zval **zobject, zend_class_entry *ce,
		const unsigned char *buffer, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC)
{
	/* can it happen at all? */
	g_warning("Unable to unserialize midgard_connection object");
	return FAILURE;
}

void php_midgard_connection_init(int module_number)
{
	static php_midgard_function_entry connection_methods[] = {
		PHP_ME(midgard_connection,	__construct,	
				arginfo_midgard_connection___construct,		ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_connection,	open,		
				arginfo_midgard_connection_open,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection,	open_config,	
				arginfo_midgard_connection_open_config,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection,	set_sitegroup,	
				arginfo_midgard_connection_set_sitegroup,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,	get_sitegroup,	
				arginfo_midgard_connection_get_sitegroup,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,	set_lang,	
				arginfo_midgard_connection_set_lang,		ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,	get_lang,	
				arginfo_midgard_connection_get_lang,		ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,	set_default_lang,
				arginfo_midgard_connection_set_default_lang,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,	get_default_lang,
				arginfo_midgard_connection_get_default_lang,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,	connect,	
				arginfo_midgard_connection_connect,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection,      get_error,      
				arginfo_midgard_connection_get_error,		ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,      set_error,      
				arginfo_midgard_connection_set_error,		ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,      get_error_string,      
				arginfo_midgard_connection_get_error_string,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,      get_user,       
				arginfo_midgard_connection_get_user,		ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,      set_loglevel, 
				arginfo_midgard_connection_set_loglevel,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,      enable_dbus, 
				arginfo_midgard_connection_enable_dbus,		ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,      enable_replication, 
				arginfo_midgard_connection_enable_replication,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,      enable_quota, 
				arginfo_midgard_connection_enable_quota,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,      is_enabled_dbus, 
				arginfo_midgard_connection_is_enabled_dbus,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,      is_enabled_replication, 
				arginfo_midgard_connection_is_enabled_replication, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection,      is_enabled_quota, 
				arginfo_midgard_connection_is_enabled_quota,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		{NULL, NULL, NULL}
	};
	
	static zend_class_entry php_midgard_connection_class_entry;
	TSRMLS_FETCH();
	
	INIT_CLASS_ENTRY(
			php_midgard_connection_class_entry,
			"midgard_connection", connection_methods);
	
	php_midgard_connection_class =
		zend_register_internal_class(&php_midgard_connection_class_entry TSRMLS_CC);

	/* Set function to initialize underlying data */
	php_midgard_connection_class->create_object = php_midgard_gobject_new;
	php_midgard_connection_class->serialize = __serialize_cnc_hook;
	php_midgard_connection_class->unserialize = __unserialize_cnc_hook;
}
