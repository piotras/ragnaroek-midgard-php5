/*
 * Copyright (C) 2006, 2007 Piotr Pokora <piotrek.pokora@gmail.com>
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

#define _FREE_PARAMETERS \
	guint _i; \
        for(_i = 0; _i < n_params; _i++) { \
		g_value_unset(&parameters[_i].value); \
	} \
        g_free(parameters); \

PHP_FUNCTION(_php_midgard_object_list_parameters)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	gchar *domain = NULL;
	guint domain_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"|s", &domain, &domain_length) != SUCCESS) {
		return;
	}

	MgdObject *mobj = php_midgard_get_midgard_object(zval_object);	
	MgdObject **objects = midgard_object_list_parameters(mobj, domain);
	
	zval *ret_arr;
	MAKE_STD_ZVAL(ret_arr);
	array_init(ret_arr);

	if(objects) {
		php_midgard_array_from_objects((GObject **)objects, "midgard_parameter", ret_arr);
		g_free(objects);
	}

	RETURN_ZVAL(ret_arr, 1, 0);
}

PHP_FUNCTION(_php_midgard_object_delete_parameters)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	zval *params = NULL;
	guint n_params = 0;
	gboolean rv = FALSE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"|z", &params) != SUCCESS) {
		return;
	}	

	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params);
	MgdObject *mobj = php_midgard_get_midgard_object(zval_object);
	rv = midgard_object_delete_parameters(mobj, n_params, parameters);

	_FREE_PARAMETERS;

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_purge_parameters)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	zval *params = NULL;
	guint n_params = 0;
	gboolean rv = FALSE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"|z", &params) != SUCCESS) {
		return;
	}	

	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params);
	MgdObject *mobj = php_midgard_get_midgard_object(zval_object);
	rv = midgard_object_purge_parameters(mobj, n_params, parameters);

	_FREE_PARAMETERS;

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_find_parameters)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	zval *params = NULL;
	guint n_params = 0;
	MgdObject **objects = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"|a", &params) != SUCCESS) {
		return;
	}	

	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params);
	MgdObject *mobj = php_midgard_get_midgard_object(zval_object);
	objects = midgard_object_find_parameters(mobj, n_params, parameters);

	_FREE_PARAMETERS;

	zval *ret_arr;
	MAKE_STD_ZVAL(ret_arr);
	array_init(ret_arr);
	
	if(objects) {	
		php_midgard_array_from_objects((GObject **)objects, "midgard_parameter", ret_arr);
		g_free(objects);
	}

	RETURN_ZVAL(ret_arr, 1, 0);
}

PHP_FUNCTION(_php_midgard_object_get_parameter)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *domain, *name;
	guint domain_length, name_length;		
	const GValue *gvalue;
	zval *zval_object = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"ss", 
				&domain, &domain_length,
				&name, &name_length) != SUCCESS) 
		return;

	MgdObject *mobj = php_midgard_get_midgard_object(zval_object);
	gvalue = midgard_object_get_parameter(mobj, domain, name);

	if(gvalue == NULL)
		RETURN_NULL();

	zval *retv;
	MAKE_STD_ZVAL(retv);
	php_midgard_gvalue2zval((GValue *)gvalue, retv);

	RETURN_ZVAL(retv, 1, 0);
}

PHP_FUNCTION(_php_midgard_object_set_parameter)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *domain, *name;
	guint domain_length, name_length;
	zend_bool zbool = FALSE;
	gboolean rv;	
	zval *zval_object = getThis();
	gchar *strval;
	guint strval_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"sss|z", 
				&domain, &domain_length,
				&name, &name_length,
				&strval, &strval_length, &zbool) != SUCCESS) {
		return;
	}

	MgdObject *mobj = php_midgard_get_midgard_object(zval_object);

	GValue *sval = g_new0(GValue, 1);
	g_value_init(sval, G_TYPE_STRING);
	g_value_set_string(sval, strval);
	rv = midgard_object_set_parameter(mobj, domain, name, (GValue *)sval, FALSE);
	
	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_parameter)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *domain, *name;
	guint domain_length, name_length;	
	zend_bool zbool = FALSE;
	guint _args = ZEND_NUM_ARGS();
	gboolean rv;
	const GValue *gvalue;
	zval *zval_object = getThis();
	gchar *strval;
	guint strval_length;	
	GValue *sval;

	if (zend_parse_parameters(_args TSRMLS_CC,
				"ss|sz", 
				&domain, &domain_length,
				&name, &name_length,
				&strval, &strval_length, &zbool) != SUCCESS) {
		return;
	}
	
	MgdObject *mobj = php_midgard_get_midgard_object(zval_object);

	if(mobj){

		switch(_args) {

			case 2:

				gvalue = midgard_object_get_parameter(mobj, domain, name);

				if(gvalue != NULL) {

					zval *retv;
					MAKE_STD_ZVAL(retv);
					php_midgard_gvalue2zval((GValue *)gvalue, retv);
					
					RETURN_ZVAL(retv, 1, 0);
				
				} else {

					RETURN_NULL();
				}
				break;

			case 3:	
			case 4:	
				sval = g_new0(GValue, 1);
				g_value_init(sval, G_TYPE_STRING);
				g_value_set_string(sval, strval);
				rv = midgard_object_set_parameter(mobj, domain, name, (GValue *)sval, zbool);
				RETURN_BOOL(rv);
				break;
		}
	}
}
