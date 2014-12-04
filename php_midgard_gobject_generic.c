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
#include <TSRM.h>
#include <zend_interfaces.h>
#include <locale.h>

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || PHP_MAJOR_VERSION > 5
# define MGD_IS_CALLABLE(a, b, c) zend_is_callable(a, b, c TSRMLS_CC)
#else
# define MGD_IS_CALLABLE(a, b, c) zend_is_callable(a, b, c)
#endif

/* static prototypes */
static void php_midgard_datetime_from_string(const gchar *date, zval *zvalue);
static zval *php_midgard_string_from_datetime(zval *zvalue);
static gboolean __callback_call(zval *callback_array, zval **retval, guint n_params, zval ***params);
static void __php_midgard_closure_free(gpointer data);

static void php_midgard_closure_default_marshal(GClosure *closure,
		GValue *return_value, guint n_param_values,
		const GValue *param_values, gpointer invocation_hint,
		gpointer marshal_data);
static GClosure *php_midgard_closure_new_default(zval *callback, zval *zobject, zval *zval_array TSRMLS_DC);

php_midgard_gobject *php_midgard_zend_object_store_get_object(zval *zobject TSRMLS_DC);

/* GVALUE ROUTINES */

static gboolean php_midgard_gvalue_from_zval(zval *zvalue, GValue *gvalue) 
{
	g_assert(zvalue != NULL);

	HashTable *zhash;
	GValueArray *array;
	zval **value;
	HashPosition iterator;
	GValue *tmpval;
	zval*date_zval;
	zend_class_entry *dtce;
	gchar *lstring;
	TSRMLS_FETCH();

	switch(Z_TYPE_P(zvalue)) {
		
		case IS_ARRAY:			
			zhash = Z_ARRVAL_P(zvalue);
			array =	g_value_array_new(zend_hash_num_elements(zhash));
			
			zend_hash_internal_pointer_reset_ex(zhash, &iterator);
			while (zend_hash_get_current_data_ex(
						zhash, (void **)&value, &iterator) == SUCCESS) {
				
				tmpval = php_midgard_zval2gvalue(*value);
				g_value_array_append(array, tmpval);
				g_value_unset(tmpval);
				g_free(tmpval);
				zend_hash_move_forward_ex(zhash, &iterator);
			}
			
			g_value_init(gvalue, G_TYPE_VALUE_ARRAY);
			g_value_take_boxed(gvalue, array);
			break;

		case IS_BOOL:			
			g_value_init(gvalue, G_TYPE_BOOLEAN);
			g_value_set_boolean(gvalue, Z_BVAL_P(zvalue));
			break;

		case IS_LONG:			
			g_value_init(gvalue, G_TYPE_INT);
			g_value_set_int(gvalue, Z_LVAL_P(zvalue));	
			break;

		case IS_DOUBLE:			
			g_value_init(gvalue, G_TYPE_FLOAT);
			lstring = setlocale(LC_NUMERIC, "0");
			setlocale(LC_NUMERIC, "C");
			g_value_set_float(gvalue, (gfloat)Z_DVAL_P(zvalue));
			setlocale(LC_ALL, lstring);
			break;

		case IS_STRING:			
			g_value_init(gvalue, G_TYPE_STRING);
			g_value_set_string(gvalue, Z_STRVAL_P(zvalue));
			break;
	
		case IS_OBJECT:
			
			dtce = php_midgard_get_baseclass_ptr_by_name((const gchar *)"DateTime");

			/* DateTime object, convert to string value */
			if(Z_OBJCE_P(zvalue) == dtce) {
				
				date_zval = php_midgard_string_from_datetime(zvalue);
				g_value_init(gvalue, G_TYPE_STRING);
				g_value_set_string(gvalue, Z_STRVAL_P(date_zval));
				zval_dtor(date_zval);		  			

			} else {
					
				php_midgard_gobject *php_gobject =
					php_midgard_zend_object_store_get_object(
							zvalue TSRMLS_CC);
			
				if(php_gobject && php_gobject->gobject) {

					if(!G_IS_OBJECT(php_gobject->gobject)) {
						
						g_warning("zval2gvalue conversion failed");
						return FALSE;
					}
				
					GObject *gobject = G_OBJECT(php_gobject->gobject);
						
					g_value_init(gvalue, G_TYPE_OBJECT);
					g_value_set_object(gvalue, gobject);
				}
			}
			break;

		default:
			/* FIXME, we can not fallback to string type */
			convert_to_string(zvalue);
			g_value_init(gvalue, G_TYPE_STRING);
			g_value_set_string(gvalue, Z_STRVAL_P(zvalue));
	}	

	return TRUE;
}

GValue *php_midgard_zval2gvalue(zval *zvalue)
{
	g_assert(zvalue != NULL);

	GValue *gvalue = g_new0(GValue, 1);
	
	if(!php_midgard_gvalue_from_zval(zvalue, gvalue))
		return NULL;

	return gvalue;
}

gboolean php_midgard_gvalue2zval(GValue *gvalue, zval *zvalue)	
{
	g_assert(gvalue);
	g_assert(zvalue);
	gchar *tmpstr;
	double f, dpval, tmp_val;
	const gchar *gclass_name;
	guint i;
	GObject *gobject_property;
	GValueArray *array;
	GValue *arr_val;
	zval *zarr_val;

	if (gvalue == NULL || !G_IS_VALUE(gvalue)) {
		return FALSE;
	}

	/* Generic GValue */
	switch (G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(gvalue))) {
		
		case G_TYPE_STRING:
			tmpstr = (gchar *)g_value_get_string(gvalue);
			if(!tmpstr)
				tmpstr = "";
			ZVAL_STRING(zvalue, tmpstr, 1);
			return TRUE;
			break;

		case G_TYPE_INT:
			ZVAL_LONG(zvalue, g_value_get_int(gvalue));
			return TRUE;
			break;
		
		case G_TYPE_UINT:
			ZVAL_LONG(zvalue, g_value_get_uint(gvalue));
			return TRUE;
			break;
		
		case G_TYPE_BOOLEAN:
			ZVAL_BOOL(zvalue, g_value_get_boolean(gvalue));
			return TRUE;
			break;
		
		case G_TYPE_FLOAT:
			/* I follow code from PHP_ROUND_WITH_FUZZ macro. 
			 * If You find better way to add double property value,
			 * fell free to change this code. We do not have to worry
			 * about locale settings at this point ( I hope so ) */
			dpval = (gdouble)g_value_get_float(gvalue);
			tmp_val=dpval;
			f = pow(10.0, (double) 6);
			tmp_val *= f;
			if (tmp_val >= 0.0) {
				tmp_val = floor(tmp_val + 0.50000000001);
			} else {
				tmp_val = ceil(tmp_val - 0.50000000001);
			}
			tmp_val /= f;
			dpval = !zend_isnan(tmp_val) ? tmp_val : dpval;
			ZVAL_DOUBLE(zvalue, dpval);
			return TRUE;
			break;

		case G_TYPE_OBJECT:
		
			gobject_property = g_value_get_object(gvalue);

			if(gobject_property) {
				
				gclass_name = 
					G_OBJECT_TYPE_NAME(gobject_property);
					
				if (!gclass_name) 
					return FALSE;
				
				/* TODO , check zval ref and alloc */
				php_midgard_gobject_init(zvalue,
						gclass_name, gobject_property, FALSE);

				return TRUE;
			
			} else {

				ZVAL_NULL(zvalue);
				/* TODO, implement this, currently we do not 
				 * handle such case */
				/*zvalue->value.obj =
					php_midgard_gobject_new(*ce TSRMLS_CC);
					//zend_objects_new(&zobject, *ce TSRMLS_CC);
				Z_OBJ_HT_P(zvalue) = &php_midgard_gobject_handlers;
				zend_object_std_init(zobject, *ce TSRMLS_CC); */		
			}			

			break;
			
		case G_TYPE_BOXED:
			array_init(zvalue);

			if(G_VALUE_TYPE(gvalue) == G_TYPE_VALUE_ARRAY) {

				array = (GValueArray *) g_value_get_boxed(gvalue);

				if(array == NULL)
					return TRUE;

				for (i = 0; i < array->n_values; i++) {

					arr_val = g_value_array_get_nth(array, i);
					MAKE_STD_ZVAL(zarr_val);
					php_midgard_gvalue2zval(arr_val, zarr_val);
					add_index_zval(zvalue, i, zarr_val);
				}
			}
			break;
		
		default:
			ZVAL_NULL(zvalue);
			break;
	}
	
	return FALSE;
}

/* OBJECTS ROUTINES */

php_midgard_gobject *
php_midgard_zend_object_store_get_object(zval *zobject TSRMLS_DC)
{
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *) zend_object_store_get_object(zobject TSRMLS_CC);

	if(php_gobject) {

		if(php_gobject->gobject)
			return php_gobject;

		/* Important! We must ensure we have underlying GObject always! */
		/* Failed code case: custom construstor wich does not invoke parent's one */
		/* Or any other behaviour which makes underlying gobject
		 * not being initialized */
		if(php_gobject->gobject == NULL) {

			/* Enable this log when MidCOM 3 is stable */
			/* php_error(E_NOTICE, "Invalid object's constructor (%s)", Z_OBJCE_P(zobject)->name); */

			zend_class_entry *ce = 
				Z_OBJCE_P(zobject);
			while (ce->type != ZEND_INTERNAL_CLASS && ce->parent != NULL) {
				ce = ce->parent;
			}

			guint classname_length = strlen(
					ce->name);
			gchar *_classname = g_ascii_strdown(
					ce->name, classname_length);
			
			GType class_type = 
				g_type_from_name((const gchar *) _classname);
			
			if(g_type_parent(class_type) == MIDGARD_TYPE_OBJECT) {
		
				/* php_error(E_NOTICE, "IMPLICIT %s CONSTRUCTOR", _classname); */
				php_gobject->gobject = 
					(GObject *)midgard_object_new(
							mgd_handle(),
							(const gchar *) _classname, 
							NULL);	
			} else {
			
				g_warning("Creating new underlying '%s' GObject. Missed constructor?", 
						g_type_name(class_type));
				php_gobject->gobject = 
						g_object_new(class_type, NULL);
			
			}
			
			g_free(_classname);

			return php_gobject;
		}	
	}

	return NULL;
}

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
int php_midgard_gobject_has_property(zval *zobject, zval *prop, int type, const zend_literal *key TSRMLS_DC)
#else
int php_midgard_gobject_has_property(zval *zobject, zval *prop, int type TSRMLS_DC)
#endif
{
	int _retval = 0;
	zend_object_handlers *std_hnd;
	
	php_midgard_gobject *php_gobject =
		php_midgard_zend_object_store_get_object(zobject TSRMLS_CC);
	
	GObject *gobject =
		G_OBJECT(php_gobject->gobject);
	
	if(Z_STRVAL_P(prop) == NULL) {
		g_warning("Can not check property with NULL name");
		return _retval = 0;
	}

	if(g_str_equal(Z_STRVAL_P(prop), "")) {
		g_warning("Can not check property with empty name");
		return _retval = 0;
	}

	GParamSpec *pspec =
		g_object_class_find_property(
				G_OBJECT_GET_CLASS(gobject), Z_STRVAL_P(prop));

	if(pspec) {

		if(type == 2) {
			_retval = 1;
		} else {
			GValue pval = {0, };
			g_value_init(&pval, pspec->value_type);
			g_object_get_property(gobject, Z_STRVAL_P(prop), &pval);
			const gchar *emptystr = NULL;
			gint empty_int = 0;
			guint empty_uint = 0;

			switch(pspec->value_type) {
				
				case G_TYPE_STRING:
					_retval = 1;
					emptystr = g_value_get_string(&pval);
					if(!emptystr || *emptystr == '\0' 
							|| (*emptystr == '0' && *++emptystr == '\0')) 
						_retval = 0;	
					break;

				case G_TYPE_INT:
					_retval = 1;
					empty_int = g_value_get_int(&pval);
					if(empty_int == 0)
						_retval = 0;
					break;
				
				case G_TYPE_UINT:
					_retval = 1;
					empty_uint = g_value_get_uint(&pval);
					if(empty_uint == 0)
						_retval = 0;
					break;

				case G_TYPE_OBJECT:
					if(g_value_get_object(&pval))
						_retval = 1;
					break;

				default:
					_retval = 1;
					break;
			}

			g_value_unset(&pval);
		}
	} else {	
		std_hnd = zend_get_std_object_handlers();
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
		_retval = std_hnd->has_property(zobject, prop, type, key TSRMLS_CC);
#else
		_retval = std_hnd->has_property(zobject, prop, BP_VAR_NA TSRMLS_CC);
#endif
	}

	return _retval;
}

/* Read Zend object property using underlying GObject's one */
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
zval *php_midgard_gobject_read_property(zval *zobject, zval *prop, int type, const zend_literal *key TSRMLS_DC)
#else
zval *php_midgard_gobject_read_property(zval *zobject, zval *prop, int type TSRMLS_DC)
#endif
{
	zval *_retval = NULL;
	zend_object_handlers *std_hnd;
	gboolean is_native_property = FALSE;
	GParamSpec *pspec = NULL;
	int silent = (type == BP_VAR_IS);

	php_midgard_gobject *php_gobject =
		php_midgard_zend_object_store_get_object(zobject TSRMLS_CC);

	GObject *gobject =
		G_OBJECT(php_gobject->gobject);


	if(gobject && Z_STRVAL_P(prop) != NULL) {
		
		GObjectClass *klass = G_OBJECT_GET_CLASS(gobject);
		
		/* Find GObject property */
		if(G_IS_OBJECT_CLASS(klass)) {
			
			pspec =	g_object_class_find_property(klass, Z_STRVAL_P(prop));
			
			if(pspec != NULL)
				is_native_property = TRUE;
		}
	}

	/* If found, get property's gvalue. Create zval from it and return */
	if(is_native_property) {

		if (G_TYPE_FUNDAMENTAL (pspec->value_type) == G_TYPE_OBJECT) {

			zval **property;
			if(zend_hash_find(Z_OBJPROP_P(zobject), Z_STRVAL_P(prop), 
						Z_STRLEN_P(prop)+1 ,(void **) &property) == SUCCESS) {

				_retval = *property;
			
			} else {
			
				g_warning("Failed to read %s value. Property not initialized?", Z_STRVAL_P(prop));	
				MAKE_STD_ZVAL(_retval);
				ZVAL_NULL(_retval);
				return _retval;
			}
			
		} else {

			GValue pval = {0, };
			g_value_init(&pval, pspec->value_type);
			g_object_get_property(G_OBJECT(gobject), Z_STRVAL_P(prop), &pval);
			MAKE_STD_ZVAL(_retval);
			php_midgard_gvalue2zval(&pval, _retval);	
			g_value_unset(&pval);
		}

		/* if(pspec->value_type == MGD_TYPE_TIMESTAMP) g_warning("IS DATE"); */
	
	} else {
		
		/* Property is not found. Fallback to zend's property handler 
		 * Piotras: I have no idea what type should be passed instead 
		 * of BP_VAR_NA. The point is to throw warning when property
		 * is not registered for (sub)class. */
		std_hnd = zend_get_std_object_handlers();
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
		_retval = std_hnd->read_property(zobject, prop, silent ? BP_VAR_IS : BP_VAR_NA, key TSRMLS_CC);
#else
		_retval = std_hnd->read_property(zobject, prop, BP_VAR_NA TSRMLS_CC);
#endif
	}

	zval_add_ref(&_retval);
	return _retval;	
}

static void _convert_value(zval *value, GType vtype)
{
	switch(vtype) {

		case G_TYPE_STRING:
			if(Z_TYPE_P(value) != IS_STRING)
				convert_to_string(value);
			break;

		case G_TYPE_UINT:
		case G_TYPE_INT:
			if(Z_TYPE_P(value) != IS_LONG)
				convert_to_long(value);
			break;
		
		case G_TYPE_BOOLEAN:
			if(Z_TYPE_P(value) != IS_BOOL)
				convert_to_boolean(value);
			break;

		case G_TYPE_FLOAT:
			if(Z_TYPE_P(value) != IS_DOUBLE)
				convert_to_double(value);
			break;

		case G_TYPE_OBJECT:
			if(Z_TYPE_P(value) == IS_NULL)
				return;
			//if(Z_TYPE_P(value) != IS_OBJECT)
			//	convert_to_object(value);
			break;
	}

	return;
}

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
void php_midgard_gobject_write_property(zval *zobject, zval *prop, zval *value, const zend_literal *key TSRMLS_DC)
#else
void php_midgard_gobject_write_property(zval *zobject, zval *prop, zval *value TSRMLS_DC)
#endif
{
	php_midgard_gobject *php_gobject = 
		php_midgard_zend_object_store_get_object(zobject TSRMLS_CC);

	GObject *gobject = G_OBJECT(php_gobject->gobject);

	/* Find GObject property */
	GParamSpec *pspec =
		g_object_class_find_property(G_OBJECT_GET_CLASS(gobject), Z_STRVAL_P(prop));	
		
	/* If found, set property's gvalue.*/
	if(pspec) {

		/* Property type might be initialized with IS_STRING or unset type.
		 * Check property's type and convert if needed. */
		_convert_value(value, pspec->value_type);	

		GValue *gvalue = 
			php_midgard_zval2gvalue(value);
		
		if(gvalue) {
			
			g_object_set_property(gobject, Z_STRVAL_P(prop), gvalue);
			
			if(Z_TYPE_P(value) != IS_OBJECT) {
				g_value_unset(gvalue);
			}
			
			g_free(gvalue); 		
		}
	}

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 3
	zend_object_handlers *std_hnd = zend_get_std_object_handlers();
	std_hnd->write_property(zobject, prop, value TSRMLS_CC);
	zval_add_ref(&prop);
#endif
	
	return;
}

void php_midgard_gobject_unset_property(zval *object, zval *member TSRMLS_DC)
{
	zend_class_entry *ce = Z_OBJCE_P(object);

	SEPARATE_ARG_IF_REF(member);
	zend_call_method_with_1_params(&object, ce, &ce->__unset, ZEND_UNSET_FUNC_NAME, NULL, member);
	zval_ptr_dtor(&member);
}

/* "Register" object's properties ( constructor time ) */
void php_midgard_zendobject_register_properties (
		zval *zobject, GObject *gobject)
{
	if(gobject == NULL)
		return;

	guint n_prop, i, cnl;
	zval *tmp_object;
	const gchar *gclass_name;
	gchar *classname;
	zend_class_entry **ce;
	GValue pval = {0, };
	TSRMLS_FETCH();

	GParamSpec **props = 
		g_object_class_list_properties(
				G_OBJECT_GET_CLASS(gobject), &n_prop);

	for (i = 0; i < n_prop; i++) {	

		switch(props[i]->value_type) {

			case G_TYPE_STRING:
				add_property_string(zobject, 
						(gchar*)props[i]->name, "", 1);
				break;

			case G_TYPE_UINT:
			case G_TYPE_INT:
				add_property_long(zobject,
						(gchar*)props[i]->name, 0);
				break;

			case G_TYPE_BOOLEAN:
				add_property_bool(zobject,
						(gchar*)props[i]->name, FALSE);
				break;

			case G_TYPE_FLOAT:
				add_property_double(zobject, 
						(gchar*)props[i]->name, 0);
				break;

			case G_TYPE_OBJECT:
				g_value_init(&pval, props[i]->value_type);
				g_object_get_property(gobject,
						(gchar*)props[i]->name, &pval);
				gclass_name = g_type_name(G_OBJECT_TYPE(
							G_OBJECT(g_value_get_object(&pval))));
				if(gclass_name) {
					cnl = strlen(gclass_name);
					classname = 
						g_ascii_strdown(gclass_name, cnl);
					if (zend_hash_find(CG(class_table),
								classname, 
								cnl+1, 
								(void **) &ce)  == SUCCESS) {
						MAKE_STD_ZVAL(tmp_object);
						object_init_ex(tmp_object, *ce);
						php_midgard_zendobject_register_properties(tmp_object, G_OBJECT(g_value_get_object(&pval)));
						add_property_zval(zobject, 
								(gchar*)props[i]->name, 
								tmp_object);
					}

				}
				break;

			default:
				add_property_unset(zobject, (gchar*)props[i]->name);
				break;
		}
	}

	g_free(props);

	return;
}

/* Get object's properties */
HashTable *php_midgard_zendobject_get_properties (
		zval *zobject TSRMLS_DC)
{
	php_midgard_gobject *php_gobject = 
		php_midgard_zend_object_store_get_object(zobject TSRMLS_CC);

	if(!G_IS_OBJECT(php_gobject->gobject))
		php_error(E_ERROR, "Underlying object is not GObject");
		/* php_error(E_ERROR, "Underlying object(%x) is not GObject", php_gobject->gobject); */
	
	GObject *gobject = G_OBJECT(php_gobject->gobject);

	/* Properties already set, we will update properties hash in 
	 * write_property handler's hook */
	if(php_gobject->properties)
		return php_gobject->zo.properties;

	GParamSpec **props; 
	guint propn, i;
	GValue pval = {0, };
	zval *tmp;

	props = g_object_class_list_properties(
			G_OBJECT_GET_CLASS(gobject), &propn);

	for (i = 0; i < propn; i++) {
	
		if(g_object_class_find_property(
					G_OBJECT_GET_CLASS(gobject), 
					props[i]->name)) {	

			g_value_init(&pval, props[i]->value_type);
			g_object_get_property(gobject, 
					(gchar*)props[i]->name, &pval);

			/* TODO , add this functionality if needed.
			 *
			 * GType base_type = G_TYPE_FUNDAMENTAL(props[i]->value_type);
			 * if(base_type == G_TYPE_OBJECT) {
			 *	
			 *	MgdObject *new_object = 
			 *		midgard_object_new(mgd_handle(),
			 *			g_param_spec_get_nick(props[i]), NULL);
			 *
			 *	g_value_set_object(&pval, new_object);
			 *	
			 *      // Create new zend object and add it to store
			 * }
			 *
			 */ 
			
			MAKE_STD_ZVAL(tmp);
			php_midgard_gvalue2zval(&pval, tmp);
			zend_hash_update(php_gobject->zo.properties, 
					props[i]->name, strlen(props[i]->name)+1, 
					(void *)&tmp, sizeof(zval *), NULL);
			g_value_unset(&pval);
		}
	}
	
	g_free(props);

	/* "Cache" result */
	php_gobject->properties = TRUE;

	return php_gobject->zo.properties;
}

static void __php_midgard_gobject_dtor(void *object TSRMLS_DC);

static void __object_properties_dtor(zend_object *zo)
{
	HashPosition iterator;
	zval **zvalue = NULL;
	HashTable *props = zo->properties;	
	TSRMLS_FETCH();

	zend_hash_internal_pointer_reset_ex(props, &iterator);
	while (zend_hash_get_current_data_ex(props, (void **)&zvalue, &iterator) == SUCCESS) {

		if((*zvalue)) {
	
			if(Z_TYPE_PP(zvalue) == IS_OBJECT) {	
				/* Piotras: I have no idea how to correctly destroy properties.
				 * Midgard seems to be the only one extension which provides 
				 * properties of object type. */
				/* zend_object *pzo = zend_objects_get_address(*zvalue TSRMLS_DC);
				zend_object_std_dtor(pzo TSRMLS_CC);	*/	
				zend_objects_store_del_ref(*zvalue TSRMLS_CC);
			}
		}

		zend_hash_move_forward_ex(props, &iterator);
	}

	return;
}

/* Object destructor */
static void __php_midgard_gobject_dtor(void *object TSRMLS_DC)
{
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *) object;

	if(!object)
		return;

	if(&php_gobject->zo == NULL)
		return;

	if(php_gobject->gobject == NULL) {
		
		/* This may be disabled, it's just for debugging purpose */
		/* php_error(E_NOTICE, "__php_midgard_gobject_dtor. Underlying GObject is NULL"); */
	
	} else if(G_IS_OBJECT(php_gobject->gobject)) {

		if(G_OBJECT_TYPE_NAME(G_OBJECT(php_gobject->gobject)) != NULL){

			/* php_error(E_NOTICE, "%s DTOR (%p)", G_OBJECT_TYPE_NAME(G_OBJECT(php_gobject->gobject)), (void*)php_gobject->gobject); */

			if(G_IS_OBJECT(php_gobject->gobject)) {
				/* TODO, find a way to destroy properties of object type.
				 * Memory usage will be a bit abused, but I really have no idea how it should be implemented */
				//__object_properties_dtor(&php_gobject->zo); /* FSZ */		
				g_object_unref(G_OBJECT(php_gobject->gobject));
				php_gobject->gobject = NULL;
			}
		}
	}

	zend_object_std_dtor(&php_gobject->zo TSRMLS_CC);

	php_gobject->gobject = NULL;
	efree(php_gobject);

	php_gobject = NULL;
	object = NULL;
}

void php_midgard_init_properties_objects(zval *zobject)
{
	if(zobject == NULL)
		return;

	GParamSpec **pspecs;
	guint n_param, i;
	TSRMLS_FETCH();

	php_midgard_gobject *php_gobject =
		php_midgard_zend_object_store_get_object(zobject TSRMLS_CC);
	GObject *gobject = G_OBJECT(php_gobject->gobject);

	if(!gobject)
		return;

	pspecs = g_object_class_list_properties(G_OBJECT_GET_CLASS(gobject), &n_param);

	if(!pspecs) 
		return;

	for(i = 0; i < n_param; i++) {

		if(G_TYPE_FUNDAMENTAL(pspecs[i]->value_type) != G_TYPE_OBJECT)
			continue;

		zval *prop_zobject;
		MAKE_STD_ZVAL(prop_zobject);

		GValue oval = {0, };
		g_value_init(&oval, G_TYPE_OBJECT);
		g_object_get_property(gobject, pspecs[i]->name, &oval);
		GObject *prop_gobject = g_value_get_object(&oval);	
	
		zend_class_entry *ce = 
			php_midgard_get_baseclass_ptr_by_name(G_OBJECT_TYPE_NAME(prop_gobject));

		object_init_ex(prop_zobject, ce);
		zval_add_ref(&prop_zobject);
		
		php_midgard_gobject *php_gobject =
			(php_midgard_gobject *)zend_object_store_get_object(prop_zobject TSRMLS_CC);
		php_gobject->gobject = prop_gobject;

		zend_update_property(Z_OBJCE_P(zobject), zobject, 
				pspecs[i]->name, strlen(pspecs[i]->name), 
				prop_zobject TSRMLS_CC);

		g_value_unset(&oval);
	}

	g_free(pspecs);
}

/* Object constructor */
zend_object_value php_midgard_gobject_new(zend_class_entry *class_type TSRMLS_DC)
{
	php_midgard_gobject *php_gobject;
	zend_object_value retval;	

	php_gobject = ecalloc(1, sizeof(php_midgard_gobject));
	zend_object_std_init(&php_gobject->zo, class_type TSRMLS_CC);

	/* php_error(E_NOTICE, "CONSTRUCTOR %s", class_type->name); */

	php_gobject->gobject = NULL;
	php_gobject->properties = FALSE;

	/* Do not free these members. Those are owned by Zend. We just re use them. */
	php_gobject->user_ce = NULL;
	php_gobject->user_class_name = NULL;

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	ALLOC_HASHTABLE((&php_gobject->zo)->properties);
	zend_hash_init((&php_gobject->zo)->properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	
	object_properties_init(&(php_gobject->zo), class_type);
#else
	zval *tmp
	zend_hash_copy(php_gobject->zo.properties, 
			&class_type->default_properties, 
			(copy_ctor_func_t) zval_add_ref,
			(void *) &tmp, sizeof(zval *));
#endif

	retval.handle = zend_objects_store_put(php_gobject,
			(zend_objects_store_dtor_t)zend_objects_destroy_object,
			__php_midgard_gobject_dtor,
			NULL TSRMLS_CC);
	
	retval.handlers = &php_midgard_gobject_handlers;

	return retval;
}

void php_midgard_gobject_init(zval *zvalue, 
		const gchar *classname, GObject *gobject, gboolean dtor)
{
	zend_class_entry **ce = NULL;
	TSRMLS_FETCH();

	 if(zvalue == NULL)
		 MAKE_STD_ZVAL(zvalue);
	
	 zend_lookup_class((gchar *)classname, strlen(classname), &ce TSRMLS_CC);

	 if(*ce == NULL)
		 php_error(E_ERROR, "Class '%s' is not registered", classname); 

	 php_midgard_gobject_new_with_gobject(zvalue, *ce, gobject, dtor);
}

void php_midgard_gobject_new_with_gobject(
		zval *zvalue, zend_class_entry *ce, GObject *gobject, gboolean dtor)
{
	php_midgard_gobject *php_gobject;
	TSRMLS_FETCH();

	zvalue->type = IS_OBJECT;

	php_gobject = ecalloc(1, sizeof(php_midgard_gobject));
	php_gobject->gobject = gobject;
	php_gobject->properties = FALSE;

	zend_object_std_init(&php_gobject->zo, ce TSRMLS_CC);	

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	ALLOC_HASHTABLE((&php_gobject->zo)->properties);
	zend_hash_init((&php_gobject->zo)->properties, 0, NULL, ZVAL_PTR_DTOR, 0);

	object_properties_init(&(php_gobject->zo), ce);
#else
	zval *tmp;
	zend_hash_copy(php_gobject->zo.properties,
			&ce->default_properties,
			(copy_ctor_func_t) zval_add_ref,
			(void *) &tmp, sizeof(zval *));
#endif

	if(dtor) {
		
		zvalue->value.obj.handle = zend_objects_store_put(php_gobject,
			(zend_objects_store_dtor_t)zend_objects_destroy_object,
			__php_midgard_gobject_dtor,
			NULL TSRMLS_CC);
	} else {

		zvalue->value.obj.handle = zend_objects_store_put(php_gobject,
			(zend_objects_store_dtor_t)zend_objects_destroy_object,
			NULL,
			NULL TSRMLS_CC);
	}
	
	Z_OBJ_HT_P(zvalue) = &php_midgard_gobject_handlers;

	php_midgard_init_properties_objects(zvalue);
	/* php_error(E_NOTICE, "IMPLICIT CONSTRUCTOR %s (%p)", ce->name, gobject); */
}

/* Other routines */
zend_class_entry *php_midgard_get_baseclass_ptr(zend_class_entry *ce)
{
	g_assert(ce);
	
	if(!ce->parent)
		return ce;
	else
		ce = php_midgard_get_baseclass_ptr(ce->parent);
	return ce;
}

zend_class_entry *php_midgard_get_baseclass_ptr_by_name(const gchar *name)
{
	g_assert(name != NULL);

	zend_class_entry **ce, *rce;
	gchar *lower_class_name;
	guint name_length = strlen(name);
	TSRMLS_FETCH();

	lower_class_name = g_ascii_strdown(name, name_length);
	
	int ret = zend_lookup_class(lower_class_name, name_length, &ce TSRMLS_CC);
	
	if (ret == FAILURE)
		rce = NULL;
	else 
		rce = php_midgard_get_baseclass_ptr(*ce);

	g_free(lower_class_name);

	return rce;
}

gboolean php_midgard_is_derived_from_class(const gchar *classname, GType basetype, 
		gboolean check_parent, zend_class_entry **base_class TSRMLS_DC)
{
	if (classname == NULL || *classname == '\0')
		return FALSE;

	gboolean isderived = FALSE;

	zend_class_entry *ce = php_midgard_get_baseclass_ptr_by_name(classname);

	if (ce == NULL) {

		php_error(E_WARNING, "Can not find zend class pointer for given %s class name", classname);
		return isderived;
	}

	*base_class = ce;

	GType classtype = g_type_from_name((const gchar *)ce->name);

	if (classtype == basetype)
		isderived = TRUE;

	if (check_parent == TRUE)
		isderived = g_type_is_a(classtype, basetype);

	return isderived;
}

void php_midgard_array_from_objects(GObject **objects, const gchar *class_name, zval *zarray)
{
	if(!objects)
		return;

	zend_class_entry **ce;          
	TSRMLS_FETCH();

	zend_hash_find(CG(class_table), (gchar *)class_name, strlen(class_name)+1, (void **) &ce);
	
	guint i = 0;
	
	while(objects[i] != NULL) {
		
		zval *zobject;
		MAKE_STD_ZVAL(zobject);
		php_midgard_gobject_new_with_gobject(zobject, *ce, G_OBJECT(objects[i]), TRUE);
		zend_hash_next_index_insert(
				HASH_OF(zarray), &zobject, sizeof(zval *), NULL);

		if(MIDGARD_IS_OBJECT(objects[i]))
			g_signal_emit(objects[i], 
					MIDGARD_OBJECT_GET_CLASS(objects[i])->signal_action_loaded_hook, 0);
		i++;
	}
	
	return;
}

MgdObject *php_midgard_get_midgard_object(zval *zobj)
{
	if(!zobj)
		return NULL;

	TSRMLS_FETCH();
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zobj TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);

	if(!mobj) php_error(E_ERROR, "Can not find underlying midgard gobject");

	return mobj;
}

/* DATETIME OBJECT */

static zval ***__copy_args(zval *args)
{
	zval ***params = NULL;
	guint i;
	guint argc = 0;

	if(args != NULL) {
		
		argc = zend_hash_num_elements(Z_ARRVAL_P(args));
		params = (zval ***)emalloc(argc * sizeof(zval **));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(args));
		
		for(i = 0; i < argc; i++) {
			
			zend_hash_get_current_data(Z_ARRVAL_P(args), (void **)&params[i]);
			zend_hash_move_forward(Z_ARRVAL_P(args));
		}

		return params;
	}

	return NULL;
}

static void php_midgard_datetime_from_string(const gchar *date, zval *zvalue)
{
	/* DateTimeZone object */
	zend_class_entry *dtz_ce = 
		php_midgard_get_baseclass_ptr_by_name("DateTimeZone");
	TSRMLS_FETCH();
	
	zval *dtz_params;
	MAKE_STD_ZVAL(dtz_params);
	array_init(dtz_params);
	/* Initialize object for UTC timezone */
	add_index_string(dtz_params, 0, "UTC", 0);
	
	zval *dtz_object;
	MAKE_STD_ZVAL(dtz_object);
	object_init_ex(dtz_object, dtz_ce);
	zval *dtz_constructor;
	MAKE_STD_ZVAL(dtz_constructor);
	ZVAL_STRING(dtz_constructor, "__construct", 0);

	/* Copy constructor arguments */
	zval ***args = __copy_args(dtz_params);

	zval *retval = NULL;
	call_user_function_ex(NULL, &dtz_object, dtz_constructor, &retval, 1, args, 0, NULL TSRMLS_CC);
	efree(args);

	/* DateTime object */
	zend_class_entry *dt_ce = 
		php_midgard_get_baseclass_ptr_by_name("DateTime");
	
	zval *dt_params;
	MAKE_STD_ZVAL(dt_params);
	array_init(dt_params);
	/* Initialize object for given time string and timezone*/
	if(date == NULL) date = "now";
	add_index_string(dt_params, 0, (gchar *)date, 1);
	add_index_zval(dt_params, 1, dtz_object);
	
	object_init_ex(zvalue, dt_ce);
	zval *dt_constructor;
	MAKE_STD_ZVAL(dt_constructor);
	ZVAL_STRING(dt_constructor, "__construct", 0);

	/* Copy constructor arguments */
	args = __copy_args(dt_params);

	call_user_function_ex(NULL, &zvalue, dt_constructor, &retval, 2, args, 0, NULL TSRMLS_CC);
	efree(args);
	
	return;
}

static zval *php_midgard_string_from_datetime(zval *zvalue)
{
	TSRMLS_FETCH();

	if(Z_TYPE_P(zvalue) != IS_OBJECT) {
		
		g_warning("Can not format ISO datetime string. Value is not an object");
		return NULL;
	}

		
	/* get datetime ISO8601 constant */
	zval iso_constant;
 	zend_get_constant("DateTime::ISO8601", 17, &iso_constant TSRMLS_CC);

	zval *retval;	
	zend_call_method_with_1_params(&zvalue, Z_OBJCE_P(zvalue), 
			NULL, "format", &retval, &iso_constant);

	return retval;
}

/* SIGNALS */

typedef struct {
	GClosure closure;
	zval *callback;
	zval *args;
	zval *zval_array;
	guint argc;
	guint type;
	zval *zobject;
	zval *connected;
	gchar *src_filename;
	guint src_lineno;
} php_mgd_closure;

static void php_midgard_closure_invalidate(gpointer data, GClosure *closure)
{
	php_mgd_closure *mgdclosure = (php_mgd_closure *) closure;
	
	if(mgdclosure->args != NULL)
		efree(mgdclosure->args);
	mgdclosure->args = NULL;

	if(mgdclosure->src_filename)
		efree(mgdclosure->src_filename);
	mgdclosure->src_filename = NULL;

	mgdclosure->callback = NULL;
	mgdclosure->zobject = NULL;
	mgdclosure->connected = NULL;
	mgdclosure->src_lineno = 0;
}	

void php_midgard_gobject_connect(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *callback;
	gchar *sname = NULL;
	guint sname_length;
	guint signal_id;
	zval *zval_object = getThis();
	zval *zval_array = NULL;
	GQuark signal_detail;
	GClosure *closure = NULL;

	/* Keep '!' as passed object parameter ( or params array ) can be NULL */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,  "sa|a!",
				&sname, &sname_length,
				&callback, &zval_array)
			== FAILURE) {
		return;
	}

	/* Get underlying GObject instance */
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	GObject *object = G_OBJECT(php_gobject->gobject);

	if (!g_signal_parse_name(sname, G_OBJECT_TYPE(object), 
				&signal_id, &signal_detail, TRUE)) {
		php_error(E_WARNING, "%s signal name is invalid", sname);
		return;
		/* TODO , should we handle exception here? */
	}

	closure = php_midgard_closure_new_default(callback, zval_object, zval_array TSRMLS_CC);

	if(!closure) {
		php_error(E_WARNING, "Can not create new closure");
		return;
	}

	g_signal_connect_closure_by_id(object, signal_id, signal_detail, closure, FALSE);
}

/* CLASS CLOSURES */

/* Workaround, as there's no GLib API to get class default closure by signal name or id */

static GHashTable *__classes_hash = NULL;

void __destroy_hash(gpointer data)
{
	if(!data)
		return;

	GHashTable *hash = (GHashTable*) data;

	g_hash_table_destroy(hash);
}

void php_midgard_gobject_closure_hash_new()
{
	if(__classes_hash == NULL)
		__classes_hash = 
			g_hash_table_new_full(g_str_hash, g_str_equal, 
					g_free, NULL);
}

static php_mgd_closure * __class_closure_lookup(GType class_type, guint signal_id)
{
	if(signal_id == 0)
		return NULL;

	if(__classes_hash == NULL)
		php_midgard_gobject_closure_hash_new();

	GHashTable *closures_hash;
	closures_hash = g_hash_table_lookup(__classes_hash, g_type_name(class_type));
	
	if(!closures_hash) 
		return NULL;

	gchar *sname = g_strdup(g_signal_name(signal_id));
	g_strdelimit (sname, G_STR_DELIMITERS ":^", '_'); 
	
	php_mgd_closure *pmc = g_hash_table_lookup(closures_hash, sname); 
	g_free(sname);

	return pmc;
}

void __php_midgard_closure_free(gpointer data)
{
	GClosure *closure = (GClosure *) data;	

	if(closure) 
		g_closure_unref(closure);
}

void __free_hash_foreach(gpointer key, gpointer val, gpointer ud)
{
	GHashTable *hash = (GHashTable *) val;
	
	if(hash)  	
		g_hash_table_destroy(hash);
}

void php_midgard_gobject_closure_hash_free()
{
	if(__classes_hash != NULL) {
		g_hash_table_foreach(__classes_hash, __free_hash_foreach, NULL);
		g_hash_table_destroy(__classes_hash);
		__classes_hash = NULL;
	}
	
	return;
}

static void __register_class_closure(const gchar *class_name, const gchar *signal, php_mgd_closure *closure)
{
	if(__classes_hash == NULL)
		php_midgard_gobject_closure_hash_new();

	gchar *sname = g_strdup(signal);
	g_strdelimit (sname, G_STR_DELIMITERS ":^", '_'); /* FIXME, it should be fast, so no conversion here */

	guint signal_id = g_signal_lookup(sname, g_type_from_name(class_name));

	if(signal_id == 0) {
		
		g_warning("'%s' is not registered as event for '%s'",
				sname, class_name);
		g_free(sname);
		return;
	}

	GHashTable *closures_hash = 
		g_hash_table_lookup(__classes_hash, class_name);

	if(!closures_hash) {
		closures_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, __php_midgard_closure_free);
	}
	
	g_hash_table_insert(closures_hash, (gpointer) sname, closure);
	g_hash_table_insert(__classes_hash, (gpointer) g_strdup(class_name), closures_hash);	
}

static void php_midgard_closure_default_marshal(GClosure *closure,
		GValue *return_value, guint n_param_values,
		const GValue *param_values, gpointer invocation_hint,
		gpointer marshal_data)
{
	php_mgd_closure *mgdclosure = (php_mgd_closure *) closure;

	zval *retval = NULL;
	zval ***params = NULL;
	guint i;
	zval *args = mgdclosure->args;
	guint argc = 0;

	if(args != NULL) {

		argc = zend_hash_num_elements(Z_ARRVAL_P(args));
		params = (zval ***)emalloc(argc * sizeof(zval **));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(args));

		for(i = 0; i < argc; i++) {
				
			zend_hash_get_current_data(Z_ARRVAL_P(args), (void **)&params[i]);
			zend_hash_move_forward(Z_ARRVAL_P(args));
		}
	}	

	__callback_call(mgdclosure->callback, &retval, argc, params);
	efree(params);
}

static GClosure *php_midgard_closure_new_default(zval *callback, zval *zobject, zval *zval_array TSRMLS_DC)
{
	GClosure *closure;
	php_mgd_closure *mgdclosure;

	if(zobject == NULL) {
		
		closure = g_closure_new_simple(sizeof(php_mgd_closure), NULL);
	
	} else {

		php_midgard_gobject *php_gobject =
			(php_midgard_gobject *)zend_object_store_get_object(zobject TSRMLS_CC);
		
		if(g_type_from_name(Z_OBJCE_P(zobject)->name)) {

			GObject *object = G_OBJECT(php_gobject->gobject);
			closure = g_closure_new_object(sizeof(php_mgd_closure), object);
		
		} else {
			
			closure = g_closure_new_simple(sizeof(php_mgd_closure), NULL);
		}
	}
	
	if(!closure) 
		php_error(E_ERROR, "Couldn't create new closure");

	mgdclosure = (php_mgd_closure*) closure;
	zval_add_ref(&callback);
	mgdclosure->callback = callback;
	mgdclosure->src_filename = estrdup(zend_get_executed_filename(TSRMLS_C));
	mgdclosure->src_lineno = zend_get_executed_lineno(TSRMLS_C);
	mgdclosure->zobject = zobject;
	mgdclosure->args = NULL;

	zval *new_array = NULL;
	MAKE_STD_ZVAL(new_array);
	array_init(new_array);	
	
	guint i, argc = 0;
	zval **tmp_zval;
	
	zval *_tmp;
	if(zobject == NULL) {
		MAKE_STD_ZVAL(_tmp);
		zval_add_ref(&_tmp);
		add_index_zval(new_array, 0, _tmp); /* Allocate index 0 for object */
	} else {
		/* Add reference to php object, we must ensure it's alive till callback execution */
		zval_add_ref (&zobject);
		add_index_zval(new_array, 0, zobject);
	}

	if(zval_array)
		argc = zend_hash_num_elements(Z_ARRVAL_P(zval_array));
	else 
		argc = 0;

	if(zval_array) {
		
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(zval_array));
		for(i = 0; i < argc; i++) {
			
			zend_hash_get_current_data(Z_ARRVAL_P(zval_array), (void **)&tmp_zval);
			add_index_zval(new_array, i+1, *tmp_zval); 
			zend_hash_move_forward(Z_ARRVAL_P(zval_array));
		}
	}

	if(zval_array)
		zval_add_ref(&zval_array);
	zval_add_ref(&new_array);
	mgdclosure->args = new_array;
	
	g_closure_add_invalidate_notifier(closure, NULL, php_midgard_closure_invalidate);
	g_closure_set_marshal((GClosure *)mgdclosure, php_midgard_closure_default_marshal);

	return (GClosure *)mgdclosure;
}
void php_midgard_object_class_connect_default(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *callback;
	gchar *sname = NULL;
	gchar *class_name = NULL;
	guint sname_length;
	guint class_name_length;
	guint signal_id;	
	zval *zobject = NULL;
	zval *zval_array = NULL;
	GQuark signal_detail;
	GClosure *closure = NULL;
	
	/* Keep '!' as passed object parameter ( or params array ) can be NULL */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,  "ssa|a!",
				&class_name, &class_name_length,
				&sname, &sname_length,
				&callback, &zval_array)
			== FAILURE) {
		return;
	}

	GType class_type = g_type_from_name((const gchar *)class_name);

	if(class_type == 0) {
		g_warning("Class %s is not registered in GType system", class_name);
		return;
	}

	if (!g_signal_parse_name(sname, class_type,
				&signal_id, &signal_detail, TRUE)) {
		php_error(E_WARNING, "%s signal name is invalid", sname);
		return;
		/* TODO , should we handle exception here? */
	}	

	closure = php_midgard_closure_new_default(callback, zobject, zval_array TSRMLS_CC);

	if(!closure) {
		php_error(E_WARNING, "Can not create new closure");
		return;
	}
	
	php_mgd_closure *dclosure = (php_mgd_closure *) closure;
	dclosure->zval_array = zval_array;
	__register_class_closure(class_name, sname, (php_mgd_closure *)dclosure);
}

void php_midgard_object_connect_class_closures(GObject *object, zval *zobject)
{
	php_mgd_closure *closure = NULL;

	/* TODO, add error handling , IS_OBJECT , etc */
	if(zobject == NULL) {		
		g_warning("Connect to class closure: failed to get zend object");
		return;
	}

	if(object == NULL) {
		g_warning("Connect to class closure: failed to get underlying object");
		return;
	}

	/* Add reference to zend object. Zend will segfault without this while calling destructor */
	//zval_add_ref(&zobject);

	guint i = 0;
	guint n_ids, *ids;	
	TSRMLS_FETCH();

	/* Use MIDGARD_TYPE_OBJECT type explicitly! 
	 * Ancestor type is not taken into account in GLib's list_ids! */
	ids = g_signal_list_ids(MIDGARD_TYPE_OBJECT, &n_ids);

	if(n_ids == 0)
		return;

	for(i = 0; i < n_ids; i++) {
		
		closure = __class_closure_lookup(G_OBJECT_TYPE(object), ids[i]);	

		if(closure) {
			zval_add_ref(&zobject);	
			php_mgd_closure *dclosure = (php_mgd_closure *) 
				php_midgard_closure_new_default(closure->callback, zobject, closure->zval_array TSRMLS_CC);
			
			g_signal_connect_closure(object, g_signal_name(ids[i]), (GClosure *)dclosure, FALSE);
			add_index_zval(dclosure->args, 0, zobject);
		}
	}

	g_free(ids);

	return;
}

static gboolean __callback_call(zval *callback_array, zval **rvalue, guint n_params, zval ***params)
{
	if(Z_TYPE_P(callback_array) != IS_ARRAY) {
		g_warning("Callback is not an array");
		return FALSE;
	}

	gchar *callback_name;
	gchar *lcname = NULL;
	gboolean retval = FALSE;
	zval **fmethod;
	zval **obj;
	TSRMLS_FETCH();

	guint elements = zend_hash_num_elements(HASH_OF(callback_array));
	if(elements < 1 || elements > 2) {
		g_warning("Expected callback array with 1 or 2 elements. %d given", elements);
		return FALSE;
	}
	
	/* We have only one element so it should be function */
	if(elements == 1) {

		zend_hash_index_find(Z_ARRVAL_P(callback_array), 0, (void **) &fmethod);

		if(Z_TYPE_PP(fmethod) != IS_STRING) {
			g_warning("Callback argument is not a function name.");
			return FALSE;
		}

		lcname = estrndup(Z_STRVAL_PP(fmethod), Z_STRLEN_PP(fmethod));
		zend_str_tolower(lcname, Z_STRLEN_PP(fmethod));
		if (zend_hash_exists(EG(function_table), lcname, Z_STRLEN_PP(fmethod)+1))
			retval = TRUE;
		efree(lcname);

		/* Check if zend is able to call it */
		if((!MGD_IS_CALLABLE(*fmethod, 0, &callback_name))) {
			g_warning("Can not invoke %s as callback", callback_name);
			efree(callback_name);
			retval = FALSE;
		}

		efree(callback_name);

		/* Invoke callback */
		call_user_function_ex(EG(function_table), NULL, *fmethod,
				rvalue, n_params, params, 0, NULL TSRMLS_CC);

		return retval;
	}

	gboolean valid_f_arg = FALSE;
	zend_class_entry *ce = NULL, **lookup_ce;

	/* Two arguments so we should have object or classname and method name */
	if(elements == 2) {

		zend_hash_index_find(Z_ARRVAL_P(callback_array), 0, (void **) &obj);
		zend_hash_index_find(Z_ARRVAL_P(callback_array), 1, (void **) &fmethod);
		
		if(Z_TYPE_PP(obj) == IS_STRING || Z_TYPE_PP(obj) == IS_OBJECT) 
			valid_f_arg = TRUE;
		
		if(!valid_f_arg) {
			g_warning("First callback argument must be object or classname");
			return FALSE;
		}

		if(Z_TYPE_PP(fmethod) != IS_STRING) {
			g_warning("Second callback argument must be method name");
			return FALSE;
		}

		if(Z_TYPE_PP(obj) == IS_STRING) {
			lcname = estrndup(Z_STRVAL_PP(obj), Z_STRLEN_PP(obj));
			zend_str_tolower(lcname, Z_STRLEN_PP(obj));
			if(zend_lookup_class(Z_STRVAL_PP(obj), Z_STRLEN_PP(obj), &lookup_ce TSRMLS_CC) == FAILURE) {
				g_warning("Can not find %s classname", Z_STRVAL_PP(obj));
				efree(lcname);
				return FALSE;
			}
		
			/* We must have pointer and pointer's pointer. 
			 * One is needed for lookup_class and another for hash_exists.
			 * Feel free to change it, and look at ZE method_exists implementation */
			ce = *lookup_ce;
		}
		
		if(Z_TYPE_PP(obj) == IS_OBJECT) {
			ce = Z_OBJCE_PP(obj);
			lcname = estrndup(Z_STRVAL_PP(fmethod), Z_STRLEN_PP(fmethod));
			zend_str_tolower(lcname, Z_STRLEN_PP(fmethod));
		}
			
		if (zend_hash_exists(&ce->function_table, Z_STRVAL_PP(fmethod), Z_STRLEN_PP(fmethod) + 1))
			retval = TRUE;
		else 
			g_warning("%s is not a method of %s class", Z_STRVAL_PP(fmethod), ce->name);

		call_user_function_ex(&ce->function_table, NULL, callback_array, rvalue, n_params, params, 0, NULL TSRMLS_CC);

		efree(lcname);
		return retval;
	}

	return retval;
}

GParameter *php_midgard_array_to_gparameter(zval *params, guint *n_params)
{
	if(params == NULL)
		return NULL;

	HashTable *zht = Z_ARRVAL_P(params);

	/* count hash elements to set number of parameters */
	*n_params = (guint)zend_hash_num_elements(zht);

	if(*n_params == 0)
		return NULL;

	/* Initialize parameters vector */
	GParameter *parameters = g_new0(GParameter, *n_params);

	HashPosition pos;
	zval **value;
	char *key;
	uint key_len, i = 0, k;
	ulong num_index;

	/* reset array and set pointer at first position */
	zend_hash_internal_pointer_reset_ex(zht, &pos);

	/* iterate over array and set parameters' names and values */
	while (zend_hash_get_current_data_ex(zht, (void **) &value, &pos) == SUCCESS) {
		if (zend_hash_get_current_key_ex(zht, &key, &key_len, &num_index, 0, &pos) == HASH_KEY_IS_STRING) {

			parameters[i].name = (const gchar *)key;
			
			GValue gval = {0, };
			if(!php_midgard_gvalue_from_zval(*value, &gval))
				goto CLEAN_AND_RETURN_NULL;
		
			parameters[i].value = gval;
			
		} else {

			g_warning("Parameter key must be valid string!");
			goto CLEAN_AND_RETURN_NULL;
		}

		i++;
		zend_hash_move_forward_ex(zht, &pos);
	}

	return parameters;

CLEAN_AND_RETURN_NULL:

	for(k = i ; k > -1; k--) {
		g_value_unset(&parameters[i].value);
	}
	g_free(parameters);
	
	return NULL;
}
