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

static zend_class_entry *php_midgard_sitegroup_class;

/* Object constructor */
static PHP_METHOD(midgard_sitegroup, __construct)
{
	RETVAL_FALSE;
	CHECK_MGD;
	MidgardSitegroup *sg;
	zval *zvalue = NULL;
	GValue *gval = NULL;
	zval *zval_object = getThis();
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z",
				&zvalue) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 1) {
		
		if (Z_TYPE_P(zvalue) == IS_STRING || Z_TYPE_P(zvalue) == IS_LONG) {
		
			gval = php_midgard_zval2gvalue(zvalue);
		
		} else { 
			
			php_error(E_WARNING, "Expected string, int argument");
			WRONG_PARAM_COUNT;
		}
	}

	sg = midgard_sitegroup_new(mgd_handle()->_mgd, (const GValue*) gval);

	if (gval != NULL) {
	
		g_value_unset(gval);
		g_free(gval);
	}

	if (!sg) {

		php_midgard_error_exception_throw(mgd_handle());
		return;
	}

	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);

	php_gobject->gobject = G_OBJECT(sg);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_sitegroup___construct, 0)
        ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_sitegroup, list_names)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, ""
				) == FAILURE) {
		return;
	}

	gchar **names = midgard_sitegroup_list(mgd_handle()->_mgd);
	
	zval *ret_arr;
	MAKE_STD_ZVAL(ret_arr);
	array_init(ret_arr);

	if(names == NULL) {
		RETURN_ZVAL(ret_arr, 1, 0);
	}

	guint i = 0;

	while(names[i] != NULL) {
		add_next_index_string(ret_arr, names[i], 1);
		i++;	  
	}

	g_strfreev(names);

	RETURN_ZVAL(ret_arr, 1, 0);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_sitegroup_list_names, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_sitegroup, create)
{
	CHECK_MGD;

	gboolean rv;
	zval *zval_object = getThis();
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, ""
				) == FAILURE) {
		return;
	}
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	
	MidgardSitegroup *sg = MIDGARD_SITEGROUP(php_gobject->gobject);
	
	rv = midgard_sitegroup_create(sg);
	
	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_sitegroup_create, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_sitegroup, update)
{
	CHECK_MGD;
	
	gboolean rv;
	zval *zval_object = getThis();
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, ""
				) == FAILURE) {
		return;
	}
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	
	MidgardSitegroup *sg = MIDGARD_SITEGROUP(php_gobject->gobject);
	
	rv = midgard_sitegroup_update(sg);
	
	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_sitegroup_update, 0)
ZEND_END_ARG_INFO()

void php_midgard_sitegroup_init(int module_number)
{
	static php_midgard_function_entry sitegroup_methods[] = {
		PHP_ME(midgard_sitegroup,    __construct,	
				arginfo_midgard_sitegroup___construct,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_sitegroup,    list_names,	
				arginfo_midgard_sitegroup_list_names,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_sitegroup,    create,		
				arginfo_midgard_sitegroup_create,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_sitegroup,    update,		
				arginfo_midgard_sitegroup_update,	ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};
	
	static zend_class_entry php_midgard_sitegroup_class_entry;
	TSRMLS_FETCH();
	
	INIT_CLASS_ENTRY(
			php_midgard_sitegroup_class_entry,
			"midgard_sitegroup", sitegroup_methods);
	
	php_midgard_sitegroup_class =
		zend_register_internal_class(&php_midgard_sitegroup_class_entry TSRMLS_CC);

	/* Set function to initialize underlying data */
	php_midgard_sitegroup_class->create_object = php_midgard_gobject_new;
	php_midgard_sitegroup_class->serialize = __serialize_object_hook;
	php_midgard_sitegroup_class->unserialize = __unserialize_object_hook;
}
