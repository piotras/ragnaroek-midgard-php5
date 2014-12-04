/* Copyright (C) 2006 Piotr Pokora <piotrek.pokora@gmail.com>
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

static zend_class_entry *php_midgard_replicator_class;

static MidgardReplicator *_get_replicator(zval *object)
{
	TSRMLS_FETCH();
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(object TSRMLS_CC);
	
	return MIDGARD_REPLICATOR(php_gobject->gobject);
}

static MgdObject *_get_object(zval *object)
{
	TSRMLS_FETCH();
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(object TSRMLS_CC);
	
	return MIDGARD_OBJECT(php_gobject->gobject);
}

/* Object constructor */
static PHP_METHOD(midgard_replicator, __construct)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zobject, *zval_object = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobject) == FAILURE) 
		return;
	
	php_midgard_gobject *a_object =
		(php_midgard_gobject *)zend_object_store_get_object(zobject TSRMLS_CC);

	MidgardReplicator *replicator = 
		midgard_replicator_new(MIDGARD_OBJECT(a_object->gobject));

	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	php_gobject->gobject = G_OBJECT(replicator);

	if(!replicator)
		RETURN_FALSE;
}

static PHP_METHOD(midgard_replicator, serialize)
{
	RETVAL_FALSE;
	CHECK_MGD;
	
	gchar *xml;
	zval *zval_object = getThis();
	zval *zobject;

	/* We have PHP object instance */
	if(zval_object) {
		
		if(ZEND_NUM_ARGS() > 0) {
			WRONG_PARAM_COUNT;
			RETURN_FALSE;
		}

		php_midgard_gobject *php_gobject =
			(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
		MidgardReplicator *rep = MIDGARD_REPLICATOR(php_gobject->gobject);

		if(!rep)
			RETURN_NULL();

		xml = midgard_replicator_serialize(rep, NULL);
	
	} else {

		/* No PHP object , static method call midgard_replicator::serialize($object) */
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobject)
				== FAILURE) {
			WRONG_PARAM_COUNT;
			RETURN_FALSE;
		}
		
		zend_class_entry *ce_base;
		PHP_MIDGARD_PARSE_CLASS_ARGUMENT(Z_OBJCE_P(zobject)->name, MIDGARD_TYPE_DBOBJECT, TRUE, &ce_base);

		php_midgard_gobject *a_object =
			(php_midgard_gobject *)zend_object_store_get_object(zobject TSRMLS_CC);
		if (!a_object 
				|| (a_object && !a_object->gobject)
				|| (a_object->gobject && !G_IS_OBJECT (a_object->gobject))) {
			g_warning ("Missed underlying GObject for given %s instance", Z_OBJCE_P(zobject)->name);
			RETURN_NULL(); 	
		}
		GObject *object = G_OBJECT(a_object->gobject);
		
		xml = midgard_replicator_serialize(NULL, object);
	}

	if(xml == NULL)
		RETURN_NULL();
	
	/* We have to duplicate xml string because Zend seems to ... */

	RETVAL_TRUE;
	RETVAL_STRING(xml, 1);
	
	g_free(xml);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_serialize, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, export)
{
	RETVAL_FALSE;
	CHECK_MGD;
	
	gboolean exported;
	zval *zobject, *zval_object = getThis(); 

	/* We have PHP object instance */
	if(getThis()) {
		
		if(ZEND_NUM_ARGS() > 0) {
			WRONG_PARAM_COUNT;
			RETURN_FALSE;
		}

		MidgardReplicator *rep = _get_replicator(zval_object);

		if(!rep)
			RETURN_FALSE;

		exported = midgard_replicator_export(rep, NULL);
	
	} else {

		/* No PHP object , static method call midgard_replicator::export($object) */
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobject)
				== FAILURE) {
			WRONG_PARAM_COUNT;
			RETURN_FALSE;
		}
		
		MgdObject *object = _get_object(zobject);
		
		exported = midgard_replicator_export(NULL, MIDGARD_DBOBJECT(object));
	}
	
	RETURN_BOOL(exported);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_export, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, export_by_guid)
{
	RETVAL_FALSE;
	CHECK_MGD;

	gchar *guid;
	guint guid_length;
	gboolean rv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&guid, &guid_length) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	rv = midgard_replicator_export_by_guid(
			mgd_handle()->_mgd, (const gchar *) guid);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_export_by_guid, 0, 0, 1)
        ZEND_ARG_INFO(0, guid)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, export_purged)
{
	RETVAL_FALSE;
	CHECK_MGD;

	gchar *startdate = NULL , *enddate = NULL , *xml = NULL;
	guint start_length, end_length;
	zval *ook; /* Object Or Klass name zval */
	MidgardObjectClass *klass = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|ss",
				&ook, &startdate, &start_length,
				&enddate, &end_length) == FAILURE) {
		WRONG_PARAM_COUNT;
		RETURN_FALSE;
	}

	if((Z_TYPE_P(ook) != IS_STRING) &&
			Z_TYPE_P(ook) != IS_OBJECT) {
		php_error(E_WARNING, 
				"%s() accepts object or string as first argument",
			              get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	if(Z_TYPE_P(ook) == IS_STRING) {
		klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME(
				(const gchar *)Z_STRVAL_P(ook));
	} else if(Z_TYPE_P(ook) == IS_OBJECT) {
		klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME(
				(const gchar *)Z_OBJCE_P(ook)->name);
	}

	if(getThis()) {

		MidgardReplicator *rep = _get_replicator(getThis());
		xml = midgard_replicator_export_purged(rep, NULL, NULL,
				startdate, enddate);
	} else {
		xml = midgard_replicator_export_purged(NULL, klass, mgd_handle()->_mgd,
				startdate, enddate);
	}

	if(xml == NULL)
		RETURN_NULL();
	 
	RETVAL_TRUE;
	RETVAL_STRING(xml, 1);
	g_free(xml);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_export_purged, 0, 0, 1)
        ZEND_ARG_INFO(0, class)
        ZEND_ARG_INFO(0, startdate)
        ZEND_ARG_INFO(0, enddate)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, serialize_blob)
{
	RETVAL_FALSE;
	CHECK_MGD;
	
	gchar *xml;
	zval *zobject; 
	MgdObject *object;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobject)
			== FAILURE) {
		WRONG_PARAM_COUNT;
		RETURN_FALSE;
	}

	/* We have PHP object instance */
	if(getThis()) {

		MidgardReplicator *rep = _get_replicator(getThis());

		if(!rep)
			RETURN_FALSE;

		object = _get_object(zobject);
		
		if(!object)
			RETURN_FALSE;

		xml = midgard_replicator_export_blob(rep, object);
	
	} else {

		/* No PHP replicator object , 
		 * static method call midgard_replicator::export($object) */
		MgdObject *object = _get_object(zobject);
		
		xml = midgard_replicator_serialize_blob(NULL, object);
	}
	
	if(xml == NULL)
		RETURN_NULL();
	
	RETVAL_TRUE;
	RETVAL_STRING(xml, 1);
	g_free(xml);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_serialize_blob, 0, 0, 1)
        ZEND_ARG_OBJ_INFO(0, object, midgard_attachment, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, unserialize)
{
	RETVAL_FALSE;
	CHECK_MGD;

	GObject **objects;
	gchar *xml;
	guint xml_length, i = 0;
	zend_bool zbool = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b",
				&xml, &xml_length, &zbool) == FAILURE) {
		WRONG_PARAM_COUNT;
		RETURN_FALSE;
	}

	zval *ret_arr;
	MAKE_STD_ZVAL(ret_arr);
	array_init(ret_arr);

	if(getThis()) {
		
		MidgardReplicator *rep = _get_replicator(getThis());
		
		if(!rep)
			RETURN_FALSE;
		
		objects = midgard_replicator_unserialize(rep, NULL, 
				(const gchar*)xml, (gboolean) zbool);
		
	} else {

		objects = midgard_replicator_unserialize(NULL, 
				mgd_handle()->_mgd, (const gchar *)xml , (gboolean) zbool);
	}

	if(!objects)
		RETURN_ZVAL(ret_arr, 1, 0);

	zend_class_entry **ce;

	while(objects[i] != NULL) {		
		
		zval *zobject;
		MAKE_STD_ZVAL(zobject);
		
		gchar *class_name = 
			g_ascii_strdown(G_OBJECT_TYPE_NAME(G_OBJECT(objects[i])), -1);
		zend_hash_find(CG(class_table),
				class_name,
				strlen(class_name)+1,
				(void **) &ce);

		php_midgard_gobject_new_with_gobject(zobject, *ce, objects[i], TRUE);

		zend_hash_next_index_insert(
				HASH_OF(ret_arr), &zobject, sizeof(zval *), NULL);
		
		i++;
	};
	
	g_free(objects);

	RETURN_ZVAL(ret_arr, 1, 0);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_unserialize, 0, 0, 1)
        ZEND_ARG_INFO(0, xml)
        ZEND_ARG_INFO(0, force)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, import_object)
{
	RETVAL_FALSE;
	CHECK_MGD;
	
	gboolean imported;
	zval *zobject; 
	zend_bool zbool = FALSE;

	/* We have PHP object instance */
	if(getThis()) {
		
		if(ZEND_NUM_ARGS() > 0) {
			WRONG_PARAM_COUNT;
			RETURN_FALSE;
		}

		MidgardReplicator *rep = _get_replicator(getThis());

		if(!rep)
			RETURN_FALSE;

		imported = midgard_replicator_import_object(rep, NULL, zbool);
	
	} else {

		/* No PHP object , static method call midgard_replicator::export($object) */
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o|b", 
					&zobject, &zbool) == FAILURE) {
			WRONG_PARAM_COUNT;
			RETURN_FALSE;
		}
		
		MgdObject *object = _get_object(zobject);
		
		imported = midgard_replicator_import_object(NULL, MIDGARD_DBOBJECT(object), zbool);
	}
	
	RETURN_BOOL(imported);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_import_object, 0, 0, 1)
        ZEND_ARG_INFO(0, object)
        ZEND_ARG_INFO(0, force)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, import_from_xml)
{
	CHECK_MGD;

	gchar *xml;
	guint xml_length;
	zend_bool zbool = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &xml, &xml_length, &zbool) == FAILURE) 
		return;

	if(getThis()){

		MidgardReplicator *rep = _get_replicator(getThis());
		
		if(rep)
			midgard_replicator_import_from_xml(
					rep, NULL, 
					(const gchar *)xml, zbool);
	} else {
		
		midgard_replicator_import_from_xml(
				NULL, mgd_handle()->_mgd, 
				(const gchar *)xml, zbool);
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_import_from_xml, 0, 0, 1)
        ZEND_ARG_INFO(0, xml)
        ZEND_ARG_INFO(0, force)
ZEND_END_ARG_INFO()

void php_midgard_replicator_init(int module_number)
{
	static php_midgard_function_entry replicator_methods[] = {
		PHP_ME(midgard_replicator, __construct, 	NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_replicator, serialize, 
				arginfo_midgard_replicator_serialize,		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, export, 
				arginfo_midgard_replicator_export,		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, export_by_guid, 
				arginfo_midgard_replicator_export_by_guid,	ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, export_purged, 
				arginfo_midgard_replicator_export_purged,	ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, serialize_blob, 
				arginfo_midgard_replicator_serialize_blob,	ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, unserialize, 
				arginfo_midgard_replicator_unserialize,		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, import_object, 
				arginfo_midgard_replicator_import_object,	ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, import_from_xml, 
				arginfo_midgard_replicator_import_from_xml,	ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_replicator_class_entry;
	TSRMLS_FETCH();
	
	INIT_CLASS_ENTRY(
			php_midgard_replicator_class_entry,
			"midgard_replicator", replicator_methods);
	
	php_midgard_replicator_class =
		zend_register_internal_class(&php_midgard_replicator_class_entry TSRMLS_CC);
}
