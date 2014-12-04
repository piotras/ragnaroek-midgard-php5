/* Copyright (C) 2006, 2007 Piotr Pokora <piotrek.pokora@gmail.com>
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

static zend_class_entry *php_midgard_collector_class;

#define _GET_COLLECTOR_OBJECT \
        zval *zval_object = getThis(); \
        php_midgard_gobject *php_gobject = \
                (php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC); \
        MidgardCollector *collector = MIDGARD_COLLECTOR(php_gobject->gobject); \
        if(!collector) php_error(E_ERROR, "Can not find underlying collector instance");

/* Object constructor */
static PHP_METHOD(midgard_collector, __construct)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *classname, *propname;	
	guint classname_length, propname_length;
	zval *value;
	midgard *mgd = mgd_handle();
	zend_class_entry *ce_base;
	zval *zval_object = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssz",
				&classname, &classname_length, 
				&propname, &propname_length,
				&value) == FAILURE) {
		return;
	}

	PHP_MIDGARD_PARSE_CLASS_ARGUMENT(classname, MIDGARD_TYPE_DBOBJECT, TRUE, &ce_base);

	GValue *gvalue = php_midgard_zval2gvalue(value);

	MidgardCollector *object =
		midgard_collector_new(mgd->_mgd, ce_base->name, propname, gvalue);

	if(!object)
		return;

	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	php_gobject->gobject = G_OBJECT(object);

	/* Set user defined class. We might need it when execute is invoked */
	zend_class_entry **ce;
	zend_lookup_class(classname, classname_length, &ce TSRMLS_CC);
	php_gobject->user_ce = *ce;
	php_gobject->user_class_name = (*ce)->name;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector___construct, 0, 0, 3)
        ZEND_ARG_INFO(0, classname)
        ZEND_ARG_INFO(0, property)
        ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, set_key_property)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *propname;
	guint propname_length;
	zval *zvalue;
	gboolean rv;
	GValue *gvalue = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z",
				&propname, &propname_length, &zvalue)
			== FAILURE) {
		return;
	}

	_GET_COLLECTOR_OBJECT;

	rv = midgard_collector_set_key_property(collector, propname, gvalue);
	
	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_set_key_property, 0, 0, 1)
        ZEND_ARG_INFO(0, property)
        ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, add_value_property)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *propname;
	guint propname_length;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&propname, &propname_length) == FAILURE) 
		return;
	
	_GET_COLLECTOR_OBJECT;

	RETURN_BOOL(midgard_collector_add_value_property(collector, propname));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_add_value_property, 0, 0, 1)
        ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, set)
{
	CHECK_MGD;
	RETVAL_TRUE;
	gchar *key, *subkey;
	guint key_length, subkey_length;
	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssz", 
				&key, &key_length, 
				&subkey, &subkey_length, &zvalue) == FAILURE) 
		return;

	_GET_COLLECTOR_OBJECT;

	GValue *gvalue = php_midgard_zval2gvalue(zvalue);
	gboolean rv = 
		midgard_collector_set(collector, key, subkey, gvalue);	

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_set, 0, 0, 3)
        ZEND_ARG_INFO(0, key)
        ZEND_ARG_INFO(0, subkey)
        ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static void __colector_update_zend_hash(GQuark key_id,
		                gpointer data, gpointer user_data)
{
	zval *zend_hash = (zval *) user_data;
	GValue *gvalue = (GValue *) data;
	
	if(gvalue == NULL)
		return;
	
	zval *zvalue;
	MAKE_STD_ZVAL(zvalue);
	/* FIXME, we need to get underlying object here */
	php_midgard_gvalue2zval(gvalue, zvalue);
	
	add_assoc_zval(zend_hash,
			(gchar *)g_quark_to_string(key_id),
			zvalue);
	return;
}

static PHP_METHOD(midgard_collector, get)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *key;
	guint key_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&key, &key_length) == FAILURE) 
		return;

	_GET_COLLECTOR_OBJECT;

	zval *ret_arr;
	MAKE_STD_ZVAL(ret_arr);
	array_init(ret_arr);

	GData *keyslist = midgard_collector_get(collector, (const gchar *)key);
	
	if(keyslist != NULL){
		g_datalist_foreach(&keyslist,
				__colector_update_zend_hash,
				ret_arr);
	}

	RETURN_ZVAL(ret_arr, 1, 0);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_get, 0, 0, 1)
        ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, get_subkey)
{
	RETVAL_FALSE;
	CHECK_MGD;	
	const gchar *key, *subkey;
	guint key_length, subkey_length;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
				&key, &key_length, &subkey, &subkey_length) == FAILURE) 
		return;
	
	_GET_COLLECTOR_OBJECT;

	GValue *gvalue = 
		midgard_collector_get_subkey(collector, key, subkey);
	
	if(!gvalue)
		return;

	zval *_ret;
	MAKE_STD_ZVAL(_ret);
	zval_add_ref(&_ret);
	php_midgard_gvalue2zval(gvalue, _ret);

	RETURN_ZVAL(_ret, 1, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_get_subkey, 0, 0, 2)
        ZEND_ARG_INFO(0, key)
        ZEND_ARG_INFO(0, subkey)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, remove_key)
{
	RETVAL_FALSE;
	CHECK_MGD;
	const gchar *key;
	guint key_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&key, &key_length) == FAILURE) 
		return;

	_GET_COLLECTOR_OBJECT;

	RETURN_BOOL(midgard_collector_remove_key(collector, key));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_remove_key, 0, 0, 1)
        ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, merge)
{
	CHECK_MGD;
	RETVAL_FALSE;
	zval *zobject;
	zend_bool zbool = FALSE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|b", 
				&zobject, php_midgard_collector_class, &zbool) == FAILURE) 
		return;

	_GET_COLLECTOR_OBJECT;

	php_midgard_gobject *php_gobject_param =
		(php_midgard_gobject *)zend_object_store_get_object(zobject TSRMLS_CC);
	MidgardCollector *mc = MIDGARD_COLLECTOR(php_gobject_param->gobject);
	
	RETURN_BOOL(midgard_collector_merge(collector, mc, zbool));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_merge, 0, 0, 1)
        ZEND_ARG_OBJ_INFO(0, key, midgard_collector, 0)
        ZEND_ARG_INFO(0, overwrite)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, list_keys)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") != SUCCESS) 
		return;

	_GET_COLLECTOR_OBJECT;

	array_init(return_value);
	
	gchar **keys =
		midgard_collector_list_keys(collector);
	
	if(!keys)
		return;
	
	guint i = 0;
	while(keys[i] != NULL) {
		add_assoc_string(return_value, (gchar *)keys[i], "", 1);
		i++;
	}	

	g_free(keys);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_collector_list_keys, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, add_constraint)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *name, *op;
	guint name_length, op_length;
	zval *value;
	gboolean rv;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssz", 
				&name, &name_length, 
				&op, &op_length, &value) != SUCCESS) 
		return;
	
	GValue *gvalue = php_midgard_zval2gvalue(value);
	
	_GET_COLLECTOR_OBJECT;

	rv = midgard_collector_add_constraint(collector, name, op, gvalue);
	
	g_value_unset(gvalue);
	g_free(gvalue);

	RETURN_BOOL(rv);
}

static PHP_METHOD(midgard_collector, begin_group)
{
	CHECK_MGD;
	RETVAL_FALSE;
	gchar *type;
	guint type_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&type, &type_length) != SUCCESS) 
		return;

	_GET_COLLECTOR_OBJECT;

	RETURN_BOOL(midgard_collector_begin_group(collector, type));
}

static PHP_METHOD(midgard_collector, end_group)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") != SUCCESS) 
		return;

	_GET_COLLECTOR_OBJECT;

	RETURN_BOOL(midgard_collector_end_group(collector));
}

static PHP_METHOD(midgard_collector, add_order)
{
	RETVAL_FALSE;
	CHECK_MGD;
	const gchar *field, *order = "ASC";
	guint field_length, order_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s",
				&field, &field_length, &order, &order_length) != SUCCESS) 
		return;
	
	_GET_COLLECTOR_OBJECT;

	RETURN_BOOL(midgard_collector_add_order(collector, field, order));
}

static PHP_METHOD(midgard_collector, set_offset)
{
	RETVAL_FALSE;
	CHECK_MGD;
	long offset;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &offset) != SUCCESS) 
		return;
	
	if (offset < 0) {

		php_error(E_WARNING, "Ignoring a negative query offset");
		return;

	}

	_GET_COLLECTOR_OBJECT;
	
	midgard_collector_set_offset(collector, offset);

	RETURN_TRUE;
}

static PHP_METHOD(midgard_collector, set_limit)
{
	RETVAL_FALSE;
	CHECK_MGD;
	long limit;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &limit) != SUCCESS) 
		return;
	
	if (limit < 0) {

		php_error(E_WARNING, "Ignoring a negative query limit");
		return;
	}

	_GET_COLLECTOR_OBJECT;

	midgard_collector_set_limit(collector, limit);

	RETURN_TRUE;
}

static PHP_METHOD(midgard_collector, set_lang)
{
	RETVAL_FALSE;
	CHECK_MGD;
	long lang;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &lang) != SUCCESS) 
		return;
	
	if (lang < 0) {

		php_error(E_WARNING, "Ignoring a negative language id");
		return;

	}

	_GET_COLLECTOR_OBJECT;
	
	midgard_collector_set_lang(collector, lang);

	RETURN_TRUE;
}

static PHP_METHOD(midgard_collector, execute)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") != SUCCESS)
		return;

	_GET_COLLECTOR_OBJECT;
	
	RETURN_BOOL(midgard_collector_execute(collector));		
}

static PHP_METHOD(midgard_collector, count)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") != SUCCESS)
		return;

	php_error (E_WARNING, "midgard_collector count() not implemented");

	RETURN_LONG (0);
}

void php_midgard_collector_init(int module_number)
{
	static php_midgard_function_entry collector_methods[] = {
		PHP_ME(midgard_collector,	__construct,		arginfo_midgard_collector___construct,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,       set_key_property,       arginfo_midgard_collector_set_key_property,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,       add_value_property,	arginfo_midgard_collector_add_value_property, 	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,       set,			arginfo_midgard_collector_set,			ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,       get,			arginfo_midgard_collector_get,			ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,	get_subkey,		arginfo_midgard_collector_get_subkey,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,	merge,			arginfo_midgard_collector_merge,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,	list_keys,		arginfo_midgard_collector_list_keys,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,	remove_key,		arginfo_midgard_collector_remove_key,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,	add_constraint,		NULL, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,	begin_group,		NULL, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,	end_group,		NULL, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,	add_order,		NULL, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,	set_offset,		NULL, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,	set_limit,		NULL, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,	set_lang,		NULL, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,	execute,		NULL, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector,	count,			NULL, ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};
	
	static zend_class_entry php_midgard_collector_class_entry;

	INIT_CLASS_ENTRY(
			php_midgard_collector_class_entry,
			"midgard_collector", collector_methods);

	php_midgard_collector_class = 
		midgard_php_register_internal_class(
				"midgard_collector", 
				MIDGARD_TYPE_COLLECTOR,
				php_midgard_collector_class_entry);	

	/* Set function to initialize underlying data */
	php_midgard_collector_class->create_object = php_midgard_gobject_new;
}
