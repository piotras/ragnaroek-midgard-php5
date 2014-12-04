/* Copyright (C) 2006,2007 Piotr Pokora <piotrek.pokora@gmail.com>
 * 
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

static zend_class_entry *php_midgard_reflection_property_class;

#define _GET_MRP_OBJECT \
	zval *zval_object = getThis(); \
        php_midgard_gobject *php_gobject = \
                (php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC); \
        MidgardReflectionProperty *mrp = MIDGARD_REFLECTION_PROPERTY(php_gobject->gobject); \
        if(!mrp) php_error(E_ERROR, "Can not find underlying reflector instance");

#define _NOSCHEMA_CLASS_ERR \
	{ php_error(E_WARNING,"%s method can not be called."\
			"midgard_reflection_property initialized with non schema class", \
			get_active_function_name(TSRMLS_C));\
	RETURN_FALSE; }

/* Object constructor */
static PHP_METHOD(midgard_reflection_property, __construct)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *classname = NULL;
	guint classname_length;
	zend_class_entry *ce_base;	
	zval *zval_object = getThis();

	if (zend_parse_parameters(1 TSRMLS_CC, "s", 
				&classname, &classname_length) == FAILURE) 
		return;

	PHP_MIDGARD_PARSE_CLASS_ARGUMENT(classname, MIDGARD_TYPE_DBOBJECT, TRUE, &ce_base);
	
	MidgardObjectClass *klass = 
		MIDGARD_OBJECT_GET_CLASS_BY_NAME(ce_base->name);
	
	if(!klass){
		
		php_error(E_WARNING, 
				"%s is not registered Midgard schema class", ce_base->name);
		return;
	}	

	MidgardReflectionProperty *mrp = midgard_reflection_property_new(klass);

	if(!mrp) {

		php_midgard_error_exception_throw(mgd_handle());
		return;
	}

	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	php_gobject->gobject = G_OBJECT(mrp);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp___construct, 0, 0, 1)
        ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflection_property, get_midgard_type)
{
	gchar *property_name; 
	guint property_name_length;
	RETVAL_FALSE;
	CHECK_MGD;
	
	if (zend_parse_parameters(1 TSRMLS_CC, "s",
				&property_name, &property_name_length) == FAILURE) 
		return;
	
	_GET_MRP_OBJECT;

	RETURN_LONG(midgard_reflection_property_get_midgard_type(mrp, property_name));				
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_get_midgard_type, 0, 0, 1)
        ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflection_property, is_link)
{
	gchar *property_name; 
	guint property_name_length;
	RETVAL_FALSE;
	CHECK_MGD;
	
	if (zend_parse_parameters(1 TSRMLS_CC, "s",
				&property_name, &property_name_length) == FAILURE) 
		return;
	
	_GET_MRP_OBJECT;

	RETURN_BOOL(midgard_reflection_property_is_link(mrp, property_name));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_is_link, 0, 0, 1)
        ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflection_property, get_link_name)
{
	gchar *property_name; 
	guint property_name_length;
	RETVAL_FALSE;
	CHECK_MGD;
	
	if (zend_parse_parameters(1 TSRMLS_CC, "s",
				&property_name, &property_name_length) == FAILURE) 
		return;
	
	_GET_MRP_OBJECT;
	
	const gchar *linkname = 
		midgard_reflection_property_get_link_name(mrp, property_name);

	if(linkname)
		RETURN_STRING((gchar *)linkname, 1);

	RETURN_NULL();						
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_get_link_name, 0, 0, 1)
        ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflection_property, get_link_target)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *property_name;
	guint property_name_length;
		
	if (zend_parse_parameters(1 TSRMLS_CC, "s",
				&property_name, &property_name_length) == FAILURE)  
		return;

	_GET_MRP_OBJECT;

	const gchar *linktarget =
		midgard_reflection_property_get_link_target(mrp, property_name);

	if(linktarget)
		RETURN_STRING((gchar *)linktarget, 1);
	
	RETURN_FALSE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_get_link_target, 0, 0, 1)
        ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflection_property, description)
{
	gchar *property_name; 
	guint property_name_length;
	RETVAL_FALSE;
	CHECK_MGD;
	
	if (zend_parse_parameters(1 TSRMLS_CC, "s",
				&property_name, &property_name_length) == FAILURE) 
		return;

	_GET_MRP_OBJECT;

	const gchar *description =
		midgard_reflection_property_description(mrp, property_name);

	if(description)
		RETURN_STRING((gchar *)description, 1);
	
	RETURN_NULL();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_description, 0, 0, 1)
        ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflection_property, is_multilang)
{
	gchar *property_name; 
	guint property_name_length;
	RETVAL_FALSE;
	CHECK_MGD;
	
	if (zend_parse_parameters(1 TSRMLS_CC, "s",
				&property_name, &property_name_length) == FAILURE) 
		return;

	_GET_MRP_OBJECT;
	
	RETURN_BOOL(midgard_reflection_property_is_multilang(
				mrp, property_name));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_is_multilang, 0, 0, 1)
        ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflection_property, get_user_value)
{
	gchar *property_name;
	guint property_name_length;
	gchar *name;
	guint name_length;
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
				&property_name, &property_name_length, &name, &name_length) == FAILURE)
		return;

	_GET_MRP_OBJECT;

	const gchar *value = midgard_reflection_property_get_user_value(mrp, (const gchar *)property_name, (const gchar *)name);
	
	if (value)
		RETURN_STRING((gchar *)value, 1);

	RETURN_NULL();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_get_user_value, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()


void php_midgard_reflection_property_init(int module_number) 
{
	static php_midgard_function_entry reflection_property_methods[] = {
		PHP_ME(midgard_reflection_property,	__construct,
				arginfo_mrp___construct,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflection_property,	get_midgard_type,
				arginfo_mrp_get_midgard_type,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflection_property,	is_link,
				arginfo_mrp_is_link,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflection_property,	get_link_name,
				arginfo_mrp_get_link_name,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflection_property,	get_link_target,
				arginfo_mrp_get_link_target,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflection_property,	description,
				arginfo_mrp_description,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflection_property,	is_multilang,
				arginfo_mrp_is_multilang,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflection_property,     get_user_value,
   				arginfo_mrp_get_user_value,     ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};
	
	static zend_class_entry reflection_property_class_entry;
	TSRMLS_FETCH();

	INIT_CLASS_ENTRY(
			reflection_property_class_entry, 
			"midgard_reflection_property", reflection_property_methods);
	php_midgard_reflection_property_class =
		zend_register_internal_class(&reflection_property_class_entry TSRMLS_CC);

	/* Set function to initialize underlying data */
	php_midgard_reflection_property_class->create_object = php_midgard_gobject_new;
}
