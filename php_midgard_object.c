/* 
 * Copyright (C) 2007, 2008 Piotr Pokora <piotrek.pokora@gmail.com>
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
#include "php_midgard_object.h"
#include <Zend/zend_exceptions.h>

#define __THROW_EXCEPTION \
	if(EG(exception)) { \
		return_value = zend_throw_exception(Z_OBJCE_P(EG(exception)), "", 0 TSRMLS_CC); \
		return; \
	}

#define _GET_MIDGARD_OBJECT \
	zval *zval_object = getThis(); \
	php_midgard_gobject *php_gobject = \
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC); \
	MgdObject *object = MIDGARD_OBJECT(php_gobject->gobject); \
	if(!object) php_error(E_ERROR, "Can not find underlying midgard object instance");

/* Object constructor */    
PHP_FUNCTION(_midgard_php_object_constructor)
{
	CHECK_MGD;
	RETVAL_FALSE;
	guint arguments = ZEND_NUM_ARGS(); 
	zval *objid;	
	MgdObject *gobject = NULL;
	midgard *mgd = mgd_handle();
	zval *zval_object = getThis();
	zend_class_entry *base_class = 
		php_midgard_get_baseclass_ptr(Z_OBJCE_P(zval_object));
	const gchar *zend_classname = base_class->name;	

	if(arguments > 1) {
		WRONG_PARAM_COUNT;
	}	
	
	/* Initialize empty MgdObject */
	if(arguments == 0) {
		
		gobject = midgard_object_new(mgd, 
				(const gchar *) zend_classname,
				NULL);
		if(!gobject){

			php_midgard_error_exception_throw(mgd);
			return;	

		} else {

			php_midgard_gobject *php_gobject =
				(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
			php_gobject->gobject = G_OBJECT(gobject);
			RETVAL_TRUE;	
		}
	}
	
	/* There is one parameter , so we get object by id or guid */
	if(arguments == 1) {
		
		if(zend_parse_parameters_ex(
					2, 1 TSRMLS_CC, "z", &objid)  == FAILURE) 
			return;
		
		if((objid->type == IS_STRING) 
				|| (objid->type == IS_LONG)
				|| (objid->type == IS_NULL)) {
			
			if(objid->type != IS_NULL) {
			
				GValue *value = 
					php_midgard_zval2gvalue(objid);
				gobject = midgard_object_new(mgd, 
						(const gchar *) zend_classname,
						value);
				g_value_unset(value);
				g_free(value);
			
			} else {

				gobject = midgard_object_new(mgd,
						(const gchar *) zend_classname,
						NULL);
			}

			if(!gobject) {
				
				php_midgard_error_exception_throw(mgd);
				return; 

			}
					
			php_midgard_gobject *php_gobject =
				(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
			php_gobject->gobject = G_OBJECT(gobject);
			RETVAL_TRUE;
		
		} else {

			php_error(E_WARNING, "Wrong type for '%s' constructor",
					zend_classname);
		}		
	}

	 if(gobject) {

		php_midgard_init_properties_objects(zval_object);
		php_midgard_object_connect_class_closures(G_OBJECT(gobject), zval_object);
		g_signal_emit(gobject, MIDGARD_OBJECT_GET_CLASS(gobject)->signal_action_loaded_hook, 0);
	} 

	/* php_error(E_NOTICE, "PHP OBJ CONSTR %s (%p)", Z_OBJCE_P(zval_object)->name, (void*)getThis()); */
}

PHP_FUNCTION(_midgard_php_object_find)
{
	zval *zval_object = getThis();
	CHECK_MGD;
	RETVAL_TRUE;

	if(ZEND_NUM_ARGS() != 0)
		WRONG_PARAM_COUNT;
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj =
		MIDGARD_OBJECT(php_gobject->gobject);

	zval *ret_arr;
	MAKE_STD_ZVAL(ret_arr);
	array_init(ret_arr);

	if(mobj) {
		
		guint i;
		MidgardTypeHolder *holder = 
			g_new(MidgardTypeHolder, 1);
		GObject **objects =
			midgard_object_find(mobj, holder);
		
		for (i = 0; i < holder->elements; i++) {
			zval *zobject;
			MAKE_STD_ZVAL(zobject);
			php_midgard_gobject_new_with_gobject(zobject, Z_OBJCE_P(zval_object), 
					objects[i], TRUE);
			zend_hash_next_index_insert(
					HASH_OF(ret_arr), &zobject, sizeof(zval *), NULL);
		}
		
		g_free(holder);
		
		if(objects > 0) 
			g_free(objects);
	}        

	RETURN_ZVAL(ret_arr, 1, 0);
}
  
PHP_FUNCTION(_midgard_php_object_create)
{
	CHECK_MGD;
	RETVAL_FALSE;
	NOT_STATIC_METHOD();

	zval *zval_object = getThis();
	
	if(ZEND_NUM_ARGS() > 0)
		WRONG_PARAM_COUNT;

				
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT( php_gobject->gobject);

	if (mobj != NULL) {		

		g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_create_hook, 0);
		__THROW_EXCEPTION

		if(midgard_object_create(mobj))
			RETVAL_TRUE;
	}
}

PHP_FUNCTION(_midgard_php_object_update)
{
	RETVAL_FALSE;	
	CHECK_MGD;
	NOT_STATIC_METHOD();
	zval *zval_object = getThis();	

	if(ZEND_NUM_ARGS() != 0) 
		WRONG_PARAM_COUNT;
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);	

	if (mobj) {	

		g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_update_hook, 0);
		__THROW_EXCEPTION
		
		if(midgard_object_update(mobj))
			RETVAL_TRUE; 		
	}
}

PHP_FUNCTION(_get_type_by_id)
{
	RETVAL_FALSE;
	CHECK_MGD;
	NOT_STATIC_METHOD();
	guint id;
	zval *zval_object = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &id) == FAILURE) 
		return;
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj =
		MIDGARD_OBJECT(php_gobject->gobject);
	
	if (mobj) {

		midgard_object_get_by_id(mobj, id);
		if(!php_midgard_error_exception_throw(mgd_handle())){
			g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_loaded_hook, 0);
			RETVAL_TRUE;
		}
	}  
}

PHP_FUNCTION(_midgard_php_object_get_by_guid)
{
	RETVAL_FALSE;
	CHECK_MGD;
	NOT_STATIC_METHOD();
	zval *zval_object = getThis();
	gchar *guid;
	int glen;
	
	if (zend_parse_parameters(1 TSRMLS_CC, 
				"s", &guid, &glen)  == FAILURE) {
		return;
	}
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);

	if (mobj) {
		
		midgard_object_get_by_guid(mobj, guid);

		if(!php_midgard_error_exception_throw(mgd_handle())){
			g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_loaded_hook, 0);
			RETVAL_TRUE;
		}
	}      
}

PHP_FUNCTION(_midgard_php_object_is_in_parent_tree)
{
	RETVAL_FALSE;
	CHECK_MGD;
	NOT_STATIC_METHOD();
	zval *zval_object = getThis();
	long rootid, id;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &rootid, &id) == FAILURE) {
		return;
	}
	
	/* Return TRUE if id or rootid is 0. */
	if ((rootid == 0) && id == 0)
		RETURN_TRUE;
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);
	
	if(mobj) {
		if (midgard_object_is_in_parent_tree(mobj, rootid, id)){
			/* FIXME , throw exception */
			RETURN_TRUE;
		}
	}  
}

PHP_FUNCTION(_midgard_php_object_is_in_tree)
{
	RETVAL_FALSE;
	CHECK_MGD;
	NOT_STATIC_METHOD();
	zval *zval_object = getThis();
	long rootid, id; 	
        
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &rootid, &id) == FAILURE) {
		return;
	}

	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);

	if(mobj) {
		if (midgard_object_is_in_tree(mobj, rootid, id))
			RETURN_TRUE; 
	}
}

PHP_FUNCTION(_midgard_php_object_delete)
{
	RETVAL_FALSE;
	CHECK_MGD;
	NOT_STATIC_METHOD();
	zval *zval_object = getThis();
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);

	if(mobj) {
	
		g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_delete_hook, 0);
		__THROW_EXCEPTION
		
		if(midgard_object_delete(mobj))
			RETVAL_TRUE;
	} 
}

PHP_FUNCTION(_midgard_php_object_get_parent)
{
	RETVAL_FALSE;
	CHECK_MGD;
	NOT_STATIC_METHOD();
	zval *zval_object = getThis();
	zend_class_entry **pce; 
	MgdObject *pobj;
	gchar *class_name;
	const gchar *type_name;
	
	if (ZEND_NUM_ARGS() != 0) 
		return;

	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);

	if (mobj) {
		
		pobj = midgard_object_get_parent(mobj);
		
		if(pobj) { 
			type_name = G_OBJECT_TYPE_NAME((GObject*)pobj);
			class_name = g_ascii_strdown(type_name, strlen(type_name));
			zend_hash_find(CG(class_table), 
					class_name, strlen(class_name)+1, (void **) &pce);
			php_midgard_gobject_new_with_gobject(return_value, *pce, 
					G_OBJECT(pobj), TRUE);
			g_free(class_name);
			g_signal_emit(pobj, MIDGARD_OBJECT_GET_CLASS(pobj)->signal_action_loaded_hook, 0);
		}     
	}
}

PHP_FUNCTION(_midgard_php_object_get)
{
	RETVAL_FALSE;
	CHECK_MGD;
	NOT_STATIC_METHOD();
	zval *zval_object = getThis();
	
	if (ZEND_NUM_ARGS() != 0) 
		return;
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);
	
	
	if(mobj) {	
		
		midgard_object_get(mobj);
		if(!php_midgard_error_exception_throw(mgd_handle()))
			RETVAL_TRUE;
	}
}

PHP_FUNCTION(_midgard_php_object_list)
{
	RETVAL_FALSE;
	CHECK_MGD;
	NOT_STATIC_METHOD();
	zval *zval_object = getThis();
   	
	if (ZEND_NUM_ARGS() != 0) 
		WRONG_PARAM_COUNT;

	zval *ret_arr;
	MAKE_STD_ZVAL(ret_arr);
	array_init(ret_arr);
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);

	if(mobj) {
		
		GObject **objects;
		guint i;
		MidgardTypeHolder *holder = 
			g_new(MidgardTypeHolder, 1);
		objects = midgard_object_list(mobj, holder);
		
		if (objects) {
			
			for (i = 0; i < holder->elements; i++) {
				
				zval *zobject = NULL;
				MAKE_STD_ZVAL(zobject);
				object_init_ex(zobject, Z_OBJCE_P(zval_object));
				php_midgard_gobject *php_gobject =
					(php_midgard_gobject *)zend_object_store_get_object(zobject TSRMLS_CC);
				php_gobject->gobject = G_OBJECT(objects[i]);
			
				zend_hash_next_index_insert(
						HASH_OF(ret_arr), &zobject, sizeof(zval *), NULL);
				g_signal_emit(objects[i], MIDGARD_OBJECT_GET_CLASS(objects[i])->signal_action_loaded_hook, 0);
			}				
		}

		g_free(holder);
	}

	RETURN_ZVAL(ret_arr, 0, 0);
}

PHP_FUNCTION(_midgard_php_object_list_children)
{
	RETVAL_FALSE;
	CHECK_MGD;
	NOT_STATIC_METHOD();
	zval *zval_object = getThis();
	gchar *childcname;
	guint ccnl;
	
	if (zend_parse_parameters(1 TSRMLS_CC,
				"s", &childcname, &ccnl)  == FAILURE) {
		return;		
	}

	zval *ret_arr;
	MAKE_STD_ZVAL(ret_arr);
	array_init(ret_arr);

	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);

	if(mobj) {
		
		GObject **objects;
		guint i;

		MidgardTypeHolder *holder = 
			g_new(MidgardTypeHolder, 1);
		
		if ((objects = midgard_object_list_children(
						mobj, childcname, holder)) != NULL) {
			
			for (i = 0; i < holder->elements; i++) {

				zval *zobject = NULL;
				MAKE_STD_ZVAL(zobject);

				zend_class_entry **ce;
				zend_lookup_class(childcname, strlen(childcname), &ce TSRMLS_CC);

				object_init_ex(zobject, *ce);
				php_midgard_gobject *php_gobject =
					(php_midgard_gobject *)zend_object_store_get_object(zobject TSRMLS_CC);
				php_gobject->gobject = G_OBJECT(objects[i]);
		
				zend_hash_next_index_insert(
						HASH_OF(ret_arr), 
						&zobject, sizeof(zval *), NULL);
				g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_loaded_hook, 0);
			}						
		}
	}

	RETURN_ZVAL(ret_arr, 1, 0);
}

PHP_FUNCTION(_midgard_php_object_get_by_path)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();	
	gchar *path;
	guint pathl;	
	
	if (zend_parse_parameters(1 TSRMLS_CC,
				"s", &path, &pathl)  == FAILURE) {
		return;
	}
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);
	MgdObject *newobj;

	if(mobj) {
		
		newobj = midgard_object_get_by_path(mgd_handle(), 
				G_OBJECT_TYPE_NAME(mobj), path);
		
		if(newobj) {
			
			/* FIXME, it's a hack */
			g_object_unref(mobj);
			php_gobject->gobject = G_OBJECT(newobj);
			RETURN_TRUE;
		
		} else {
				
			RETURN_FALSE;
		}
	
	} 
}

PHP_FUNCTION(_midgard_php_object_parent)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	const gchar *parent_class_name; 	
	
	if (ZEND_NUM_ARGS() != 0) 
		return;
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);

	if(mobj) {
		
		parent_class_name = midgard_object_parent(mobj);
		if(parent_class_name)
			RETVAL_STRING((gchar *)parent_class_name, 1);		 
	}                       
}

PHP_FUNCTION(_php_midgard_object_get_languages)
{
	RETVAL_FALSE;
	CHECK_MGD;
	NOT_STATIC_METHOD();
	zval *zval_object = getThis();

	if(ZEND_NUM_ARGS() > 0)
		WRONG_PARAM_COUNT;

	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);

	zval *ret_arr;
	MAKE_STD_ZVAL(ret_arr);
	array_init(ret_arr);

	if(mobj) {

		guint i = 0;
		GObject **objects =
			midgard_object_get_languages(mobj, NULL);
			
		if(objects == NULL)
			RETURN_ZVAL(ret_arr, 1, 0);

		zend_class_entry **ce;
		zend_hash_find(CG(class_table), "midgard_language", 
				17, (void **) &ce);	
		
		while(objects[i] != NULL){
			
			zval *zobject = NULL;
			MAKE_STD_ZVAL(zobject);
			php_midgard_gobject_new_with_gobject(zobject, *ce, objects[i], TRUE);
			zend_hash_next_index_insert(HASH_OF(ret_arr), 
					&zobject, sizeof(zval *), NULL);
			g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_loaded_hook, 0);

			i++;
		}
		
		g_free(objects);
	}

	RETURN_ZVAL(ret_arr, 1, 0);
}

PHP_FUNCTION(_php_midgard_object_purge)
{
	RETVAL_FALSE;
	CHECK_MGD;
	NOT_STATIC_METHOD();
	zval *zval_object = getThis();
	
	if(ZEND_NUM_ARGS() != 0)
		return;

	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);

	if(mobj) {
	
		g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_purge_hook, 0);
		__THROW_EXCEPTION
		
		if(midgard_object_purge(mobj))
			RETVAL_TRUE;
	}
}

PHP_FUNCTION(_php_midgard_object_undelete)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *guid;
	guint guid_length;
	gboolean rv;

	if (zend_parse_parameters(1 TSRMLS_CC,
				"s", &guid, &guid_length)  == FAILURE) {
		return;
	}

	rv = midgard_object_undelete(mgd_handle()->_mgd, 
			(const gchar *)guid);
	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_export)
{
	RETVAL_FALSE;
	CHECK_MGD;
	NOT_STATIC_METHOD();
	zval *zval_object = getThis();
	gchar *xml = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") != SUCCESS) {
		return;
	}
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *mobj = MIDGARD_OBJECT(php_gobject->gobject);

	if(mobj){
		xml= midgard_object_export(mobj);	
	}
	
	if(xml){
		RETVAL_TRUE;
		RETURN_STRING(xml, 1);
	} else {
		RETURN_NULL();
	}
}

PHP_FUNCTION(_php_midgard_object_connect)
{
	CHECK_MGD;
	php_midgard_gobject_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

PHP_FUNCTION(_php_midgard_new_query_builder)
{
	CHECK_MGD;

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	const char *_cname_space = NULL; 
#else 
	char *_cname_space = NULL;
#endif
	const char *_class_name = get_active_class_name(&_cname_space TSRMLS_CC); 

	MidgardQueryBuilder *builder =
		midgard_query_builder_new(mgd_handle(), _class_name);
	
	if (!builder)
		return;

	zend_class_entry *ce = 
		php_midgard_get_baseclass_ptr_by_name("midgard_query_builder");

	php_midgard_gobject_new_with_gobject(return_value, ce, G_OBJECT(builder), TRUE);
}

PHP_FUNCTION(_php_midgard_new_collector)
{
	CHECK_MGD;

	gchar *propname;
	guint *propname_length;
	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz",
				&propname, &propname_length, &zvalue) == FAILURE) 
		return;

	GValue *gvalue = php_midgard_zval2gvalue(zvalue);
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	const char *_cname_space = NULL;
#else
	char *_cname_space = NULL;
#endif
	const char *_class_name = get_active_class_name(&_cname_space TSRMLS_CC);

	MidgardCollector *collector =
		midgard_collector_new(mgd_handle()->_mgd, _class_name, propname, gvalue);
	
	if(!collector)
		return;

	zend_class_entry **ce;
	zend_hash_find(CG(class_table), "midgard_collector", 
			strlen("midgard_collector")+1, (void **) &ce);

	php_midgard_gobject_new_with_gobject(return_value, *ce, G_OBJECT(collector), TRUE);
}
 
PHP_FUNCTION(_php_midgard_new_reflection_property) 
{
	CHECK_MGD;

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3	
	const char *_cname_space = NULL;
#else
	char *_cname_space = NULL;
#endif
	const char *_class_name = get_active_class_name(&_cname_space TSRMLS_CC);
	
	MidgardObjectClass *klass = 
		MIDGARD_OBJECT_GET_CLASS_BY_NAME((const gchar *)_class_name);

	MidgardReflectionProperty *mrp =
		midgard_reflection_property_new(klass);
	
	if (!mrp)
		return;
	
	zend_class_entry *ce =
		php_midgard_get_baseclass_ptr_by_name("midgard_reflection_property");

	php_midgard_gobject_new_with_gobject(return_value, ce, G_OBJECT(mrp), TRUE);
}

PHP_FUNCTION(_php_midgard_object_set_guid)
{
	CHECK_MGD;

	gchar *guid = NULL;
	guint guid_length;
	zval *zval_object = getThis();
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&guid, &guid_length) == FAILURE)
		return;

	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *object = MIDGARD_OBJECT(php_gobject->gobject);

	RETURN_BOOL(midgard_object_set_guid(object, (const gchar *)guid));
}

PHP_FUNCTION(_php_midgard_object_emit)
{
	CHECK_MGD;

	gchar *name = NULL;
	guint name_length;
	zval *zval_object = getThis();
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&name, &name_length) == FAILURE)
		return;

	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	MgdObject *object = MIDGARD_OBJECT(php_gobject->gobject);

	g_signal_emit_by_name(object, name);
}

PHP_FUNCTION(_php_midgard_object_approve)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;
	gboolean rv = midgard_object_approve(object);
	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_is_approved)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;
	gboolean rv = midgard_object_is_approved(object);
	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_unapprove)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;
	gboolean rv = midgard_object_unapprove(object);
	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_lock)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;
	gboolean rv = midgard_object_lock(object);
	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_is_locked)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;
	gboolean rv = midgard_object_is_locked(object);
	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_unlock)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;
	gboolean rv = midgard_object_unlock(object);
	RETURN_BOOL(rv);
}

static struct 
{
	char *fname;
	void (*handler)(INTERNAL_FUNCTION_PARAMETERS);
	zend_uint flags;
	zend_arg_info arg_info[8]; /* Keep it as reasonable high value. Or refactor so it doesn't have to be allocated statically */
	zend_uint num_args;
}
__midgard_php_type_functions[] = 
{
	{"__construct",
		ZEND_FN(_midgard_php_object_constructor),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "guid or id", sizeof("guid or id")-1, NULL, 0, 0, 1 /* Allows NULL */, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "guid or id", sizeof("guid or id")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif
		}, 1
	},

	{"get_by_id",
		ZEND_FN(_get_type_by_id),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 1 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},
	
	{"get_by_guid",
		ZEND_FN(_midgard_php_object_get_by_guid),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},

	{"update",
		ZEND_FN(_midgard_php_object_update),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },	
#endif
		}, 0
	},

	{"create",
		ZEND_FN(_midgard_php_object_create),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"is_in_parent_tree",
		ZEND_FN(_midgard_php_object_is_in_parent_tree),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "root_id", sizeof("root_id")-1, NULL, 0, 0, 0, 0 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "root_id", sizeof("root_id")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 2
	},

	{"is_in_tree",
		ZEND_FN(_midgard_php_object_is_in_tree),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "root_id", sizeof("root_id")-1, NULL, 0, 0, 0, 0 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "root_id", sizeof("root_id")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 2
	},

	{"delete",
		ZEND_FN(_midgard_php_object_delete),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"get_parent",
		ZEND_FN(_midgard_php_object_get_parent),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},
	
	{"list",
		ZEND_FN(_midgard_php_object_list),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"list_children",
		ZEND_FN(_midgard_php_object_list_children),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "node class name", sizeof("node class name")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "node class name", sizeof("node class name")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},

	{"get_by_path",
		ZEND_FN(_midgard_php_object_get_by_path),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "path", sizeof("path")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "path", sizeof("path")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},

	{"parent",
		ZEND_FN(_midgard_php_object_parent),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"list_parameters",
		ZEND_FN(_php_midgard_object_list_parameters),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif
		}, 1
	},

	{"find_parameters",
		ZEND_FN(_php_midgard_object_find_parameters),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif
		}, 1
	},

	{"delete_parameters",
		ZEND_FN(_php_midgard_object_delete_parameters),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif
		}, 1
	},

	{"purge_parameters",
		ZEND_FN(_php_midgard_object_purge_parameters),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif
		}, 1
	},

	{"get_parameter",
		ZEND_FN(_php_midgard_object_get_parameter),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 2
	},

	{"set_parameter",
		ZEND_FN(_php_midgard_object_set_parameter),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0 },
			{ "value", sizeof("value")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 3 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "value", sizeof("value")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "multilang", sizeof("multilang")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 4
	},

	{"parameter",
		ZEND_FN(_php_midgard_object_parameter),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0 },
			{ "value", sizeof("value")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "value", sizeof("value")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "multilang", sizeof("multilang")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 4
	},

	{"list_attachments",
		ZEND_FN(_php_midgard_object_list_attachments),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"find_attachments",
		ZEND_FN(_php_midgard_object_find_attachments),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif
		}, 1
	},

	{"delete_attachments",
		ZEND_FN(_php_midgard_object_delete_attachments),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif
		}, 1
	},

	{"purge_attachments",
		ZEND_FN(_php_midgard_object_purge_attachments),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
			{ "delete blob", sizeof("delete blob")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
			{ "delete blob", sizeof("delete blob")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 2
	},

	{"create_attachment",
		ZEND_FN(_php_midgard_object_create_attachment),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "name", sizeof("name")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
			{ "title", sizeof("title")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
			{ "mimetype", sizeof("mimetype")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
			{ "title", sizeof("title")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
			{ "mimetype", sizeof("mimetype")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 3
	},

	{"serve_attachment",
		ZEND_FN(_php_midgard_object_serve_attachment),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},


	{"get_languages",
		ZEND_FN(_php_midgard_object_get_languages),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"purge",
		ZEND_FN(_php_midgard_object_purge),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"undelete",
		ZEND_FN(_php_midgard_object_undelete),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},

	{"connect",	
		ZEND_FN(_php_midgard_object_connect),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "signal", sizeof("signal")-1, NULL, 0, 0, 0, 0 },
			{ "callback", sizeof("callback")-1, NULL, 0, 0, 0, 0 },
			{ "user_data", sizeof("user_data")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "signal", sizeof("signal")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "callback", sizeof("callback")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "user_data", sizeof("user_data")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 3
	},

	{"new_query_builder",
		ZEND_FN(_php_midgard_new_query_builder),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"new_collector",
		ZEND_FN(_php_midgard_new_collector),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"new_reflection_property",
		ZEND_FN(_php_midgard_new_reflection_property),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,	
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"set_guid",
		ZEND_FN(_php_midgard_object_set_guid),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},

	{"emit",
		ZEND_FN(_php_midgard_object_emit),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "signal", sizeof("signal")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "signal", sizeof("signal")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},

	{"approve",
		ZEND_FN(_php_midgard_object_approve),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"is_approved",
		ZEND_FN(_php_midgard_object_is_approved),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"unapprove",
		ZEND_FN(_php_midgard_object_unapprove),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else

			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"lock",
		ZEND_FN(_php_midgard_object_lock),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else

			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"is_locked",
		ZEND_FN(_php_midgard_object_is_locked),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else

			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"unlock",
		ZEND_FN(_php_midgard_object_unlock),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{ NULL, NULL } 
};

int __serialize_object_hook(zval *zobject, 
		unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC)
{
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *) zend_object_store_get_object(zobject TSRMLS_CC);

	if(!php_gobject)
		return FAILURE;

	if(!php_gobject->gobject)
		return FAILURE;

	GObject *gobject = G_OBJECT(php_gobject->gobject);
	GType object_type = G_OBJECT_TYPE(gobject);
	GType parent_object_type = g_type_parent(object_type);

	if(parent_object_type != MIDGARD_TYPE_DBOBJECT) {

		/* Try MIDGARD_OBJECT */
		if(parent_object_type != MIDGARD_TYPE_OBJECT)
			return FAILURE;
	}

	gchar *xml = midgard_replicator_serialize(NULL, G_OBJECT(gobject));

	if(!xml)
		return FAILURE;

	guint xml_length = strlen(xml)+1;
	*buffer = (unsigned char *)estrndup((const char*)xml, xml_length);
	*buf_len = xml_length;
	g_free(xml);

	return SUCCESS;
}

int __unserialize_object_hook(zval **zobject, zend_class_entry *ce, 
		const unsigned char *buffer, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC)
{
	if(buffer == NULL)
		return FAILURE;

	if(buf_len < 2)
		return FAILURE;

	GObject **objects = 
		midgard_replicator_unserialize(NULL,
				mgd_handle()->_mgd, (const gchar *)buffer, TRUE);

	if(!objects)
		return FAILURE;

	php_midgard_gobject_new_with_gobject(*zobject, ce, objects[0], TRUE);

	g_free(objects);

	return SUCCESS;
}

static void __declare_class_properties(const gchar *class_name)
{
	g_assert(class_name != NULL);

	zend_class_entry **ce = NULL;
	TSRMLS_FETCH();

	if (zend_lookup_class((gchar *)class_name, strlen(class_name), &ce TSRMLS_CC) == FAILURE)
		return; 

	MidgardObjectClass *klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME(class_name);

	if (klass == NULL)
		return;

	guint n_prop;
	guint i;
	GParamSpec **pspecs = g_object_class_list_properties(G_OBJECT_CLASS(klass), &n_prop);

	for (i = 0; i < n_prop; i++) {
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
		zend_declare_property_null(*ce, pspecs[i]->name, strlen(pspecs[i]->name), ZEND_ACC_PUBLIC TSRMLS_CC);
#else
		zend_declare_property_null(*ce, (char*)pspecs[i]->name, strlen(pspecs[i]->name), ZEND_ACC_PUBLIC TSRMLS_CC);
#endif
	}
}

static void __register_php_classes(const gchar *class_name)
{
	zend_class_entry *mgdclass, *mgdclass_ptr;
	gint j;
	guint _am = 0;
	TSRMLS_FETCH();
	
	for (j = 0; __midgard_php_type_functions[j].fname; j++){
		_am++;
	}

	zend_function_entry __functions[_am+1];
	
	/* lcn is freed in zend_register_internal_class */
	gchar *lcn = g_ascii_strdown(class_name, strlen(class_name));
    
	__functions[0].fname = "__construct";
	__functions[0].handler = ZEND_FN(_midgard_php_object_constructor);
	__functions[0].arg_info = __midgard_php_type_functions[0].arg_info;
	__functions[0].num_args = __midgard_php_type_functions[0].num_args;
        __functions[0].flags = ZEND_ACC_PUBLIC | ZEND_ACC_CTOR; 
       
	for (j = 1; __midgard_php_type_functions[j].fname; j++){
		
		__functions[j].fname = __midgard_php_type_functions[j].fname;
		__functions[j].handler = __midgard_php_type_functions[j].handler;
		__functions[j].arg_info = __midgard_php_type_functions[j].arg_info;
		__functions[j].num_args = __midgard_php_type_functions[j].num_args;	
		__functions[j].flags = __midgard_php_type_functions[j].flags;
	}

	__functions[_am].fname = NULL;
	__functions[_am].handler = NULL;
	__functions[_am].arg_info = NULL;
	__functions[_am].num_args = 0;
	__functions[_am].flags = 0;
    
	mgdclass = g_new0(zend_class_entry, 1);
	mgdclass->name = lcn;
	mgdclass->name_length = strlen(class_name);
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	mgdclass->info.internal.builtin_functions = __functions;
#else
	mgdclass->builtin_functions = __functions;
#endif	
	mgdclass->constructor = NULL;
	mgdclass->destructor = NULL;
	mgdclass->clone = NULL;
	mgdclass->create_object = NULL;
	mgdclass->interface_gets_implemented = NULL;
	mgdclass->__call = NULL;
	mgdclass->__get = NULL;
	mgdclass->__set = NULL;
	mgdclass->parent = NULL;	
	mgdclass->num_interfaces = 0;
	mgdclass->interfaces = NULL;
	mgdclass->get_iterator = NULL;
	mgdclass->iterator_funcs.funcs = NULL;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	mgdclass->info.internal.module = NULL;
#else
	mgdclass->module = NULL;
#endif
	mgdclass->ce_flags = 0;
	
	mgdclass_ptr = zend_register_internal_class(mgdclass TSRMLS_CC); 
	mgdclass_ptr->ce_flags = 0;
	mgdclass_ptr->serialize = __serialize_object_hook;
	mgdclass_ptr->unserialize = __unserialize_object_hook;
	mgdclass_ptr->create_object = php_midgard_gobject_new;

	/* Enable to declare all properties */
	/* __declare_class_properties(class_name); */

	g_free(mgdclass);
}

void php_midgard_object_init(int module_numer)
{
	guint n_types, i;
	const gchar *typename;
	GType *all_types = g_type_children(MIDGARD_TYPE_OBJECT, &n_types);
	
	for (i = 0; i < n_types; i++) {
		typename = g_type_name(all_types[i]);
		__register_php_classes(typename);
	}

	g_free(all_types);
}
