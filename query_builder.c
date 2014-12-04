/* Copyright (C) 2005 Jukka Zitting <jz@yukatan.fi>
 * Copyright (C) 2008, 2009 Piotr Pokora <piotrek.pokora@gmail.com>
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
#include <zend_interfaces.h>

#define _GET_BUILDER_OBJECT \
	zval *zval_object = getThis(); \
	php_midgard_gobject *php_gobject = \
                (php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC); \
	MidgardQueryBuilder *builder = MIDGARD_QUERY_BUILDER(php_gobject->gobject); \
	if(!builder) php_error(E_ERROR, "Can not find underlying builder instance");

static zend_class_entry *php_midgard_query_builder_class;

/* Object constructor */
static PHP_METHOD(midgard_query_builder, __construct) 
{
	CHECK_MGD;
	char *classname;
        int classname_length;
	zend_class_entry *ce_base;
	zval *zval_object = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&classname, &classname_length) == FAILURE) {
		return;
	}

	PHP_MIDGARD_PARSE_CLASS_ARGUMENT(classname, MIDGARD_TYPE_DBOBJECT, TRUE, &ce_base);

	MidgardQueryBuilder *builder =
		midgard_query_builder_new(mgd_handle(), ce_base->name);
        
	if (!builder) {

		php_midgard_error_exception_throw(mgd_handle());
		return;
	}
       
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	php_gobject->gobject = G_OBJECT(builder);

	/* Set user defined class. We might need it when execute is invoked */
	zend_class_entry **ce;
	zend_lookup_class(classname, classname_length, &ce TSRMLS_CC);
	php_gobject->user_ce = *ce;
	php_gobject->user_class_name = (*ce)->name;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mqb___construct, 0, 0, 1)
        ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_builder, add_constraint) 
{
	RETVAL_FALSE;
	CHECK_MGD;	        
        char *name, *op;
        int name_length, op_length;
        zval *value;
	zval *zval_object = getThis();
	gboolean rv;

        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                "ssz", &name, &name_length, &op, &op_length, &value) != SUCCESS) {
                return;
        }
	
	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
        MidgardQueryBuilder *builder = MIDGARD_QUERY_BUILDER(php_gobject->gobject);

        GValue *gvalue = php_midgard_zval2gvalue(value);

	if(gvalue == NULL)
		RETURN_BOOL(FALSE);

	rv = midgard_query_builder_add_constraint(builder, name, op, gvalue);
	
	g_value_unset(gvalue);
        g_free(gvalue);
        
	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mqb_add_constraint, 0, 0, 3)
        ZEND_ARG_INFO(0, property)
        ZEND_ARG_INFO(0, operator)
        ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_builder, add_constraint_with_property)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *name_a, *name_b, *op;
	guint name_a_length, name_b_length, op_length;
	gboolean rv;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"sss", &name_a, &name_a_length, &op, &op_length, 
				&name_b, &name_b_length) != SUCCESS) {
		return;
	}

	_GET_BUILDER_OBJECT;

	rv = midgard_query_builder_add_constraint_with_property(builder, 
			(const gchar *)name_a, (const gchar *)op, (const gchar *)name_b);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mqb_add_constraint_with_property, 0, 0, 3)
        ZEND_ARG_INFO(0, property)
        ZEND_ARG_INFO(0, operator)
        ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_builder, begin_group) 
{
	RETVAL_FALSE;
	CHECK_MGD;
        char *type = NULL;
        int type_length;
	gboolean rv;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", 
				&type, &type_length) != SUCCESS) {
                return;
        }
        
	_GET_BUILDER_OBJECT;
	
	rv = midgard_query_builder_begin_group(builder, type);
       
	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_mqb_begin_group, 0)
        ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_builder, end_group) 
{
	RETVAL_FALSE;
	CHECK_MGD;
	gboolean rv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") != SUCCESS) {
                return;
        }

	_GET_BUILDER_OBJECT;

        rv = midgard_query_builder_end_group(builder);
	
	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_mqb_end_group, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_builder, execute) 
{
	RETVAL_FALSE;
	CHECK_MGD;	
        zval *argv = NULL;

        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &argv) != SUCCESS) {
                RETURN_FALSE;
        }

	_GET_BUILDER_OBJECT;

	guint i;
	MidgardTypeHolder *holder = g_new(MidgardTypeHolder, 1);
        GObject **objects =
                midgard_query_builder_execute(builder, holder);

        array_init(return_value);

	if(!objects) {
		g_free(holder);
		return;
	}

	/* Initialize zend objects for the same class which was used to initialize Query Builder */
	if (php_gobject->user_ce == NULL) {
		
		g_warning("Query Builder instance not associated with any class");
		g_free(holder);
		return;
	}

	zend_class_entry *ce = php_gobject->user_ce;
	
        for (i = 0; i < holder->elements; i++) {

		zval *zobject;
		MAKE_STD_ZVAL(zobject);
		
		/* Simplify code below. If possible. */

		/* Initialize new object for which QB has been created for */
		object_init_ex(zobject, ce);
		zval *tmp_obj;
		
		/* Call class constructor on given instance */
		zend_call_method_with_0_params(&zobject, ce, NULL, "__construct", &tmp_obj);
		zval_ptr_dtor(&tmp_obj);

		/* Get underlying structure with GObject */
		php_midgard_gobject *php_gobject =
			(php_midgard_gobject *)zend_object_store_get_object(zobject TSRMLS_CC);

		/* Destroy underlying GObject */
		if(php_gobject->gobject != NULL)
			g_object_unref(php_gobject->gobject);

		/* Associate new one */
		php_gobject->gobject = objects[i];

		php_midgard_object_connect_class_closures(G_OBJECT(php_gobject->gobject), zobject);

		zend_hash_next_index_insert(
                        HASH_OF(return_value), &zobject, sizeof(zval *), NULL);
		g_signal_emit(objects[i], MIDGARD_OBJECT_GET_CLASS(objects[i])->signal_action_loaded_hook, 0);
        }

	g_free(holder);

	if(objects) g_free(objects);
}

ZEND_BEGIN_ARG_INFO(arginfo_mqb_execute, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_builder, add_order) 
{
	RETVAL_FALSE;
	CHECK_MGD;
        char *field, *order = "ASC";
        int field_length, order_length;
	gboolean rv;
        
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s",
                &field, &field_length, &order, &order_length) != SUCCESS) {
                return;
        }

	_GET_BUILDER_OBJECT;

	rv = midgard_query_builder_add_order(builder, field, order);

	RETURN_BOOL(rv); 
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mqb_add_order, 0, 0, 1)
        ZEND_ARG_INFO(0, property)
        ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_builder, set_limit) 
{
	RETVAL_FALSE;
	CHECK_MGD;
        long limit;
        
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", 
				&limit) != SUCCESS) {
                return;
        }

	_GET_BUILDER_OBJECT;

	/* TODO, check if limit check can be ignored */
        if (limit < 0) {
                php_error(E_WARNING, "Ignoring a negative query limit");
                RETURN_FALSE;
        } else {
                midgard_query_builder_set_limit(builder, limit);
                RETURN_TRUE;
        }
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mqb_set_limit, 0, 0, 1)
        ZEND_ARG_INFO(0, limit)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_builder, set_offset) 
{
	RETVAL_FALSE;
	CHECK_MGD;
        long offset;

        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", 
				&offset) != SUCCESS) {
                RETURN_FALSE;
        }

	_GET_BUILDER_OBJECT;

        if (offset < 0) {
                php_error(E_WARNING, "Ingoring a negative query offset");
                RETURN_FALSE;
        } else {
                midgard_query_builder_set_offset(builder, offset);
                RETURN_TRUE;
        }
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mqb_set_offset, 0, 0, 1)
        ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_builder, set_lang) 
{
	RETVAL_FALSE;
	CHECK_MGD;
        long lang;
        
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", 
				&lang) != SUCCESS) {
                return;
        }

	_GET_BUILDER_OBJECT;

        if (lang < 0) {
                php_error(E_WARNING, "Ignoring a negative language setting");
                RETURN_FALSE;
        } else {
                midgard_query_builder_set_lang(builder, lang);
                RETURN_TRUE;
        }
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mqb_set_lang, 0, 0, 1)
        ZEND_ARG_INFO(0, langid)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_builder, include_deleted) 
{	
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") != SUCCESS) 
		return;
	
	_GET_BUILDER_OBJECT;

	midgard_query_builder_include_deleted(builder);

	RETURN_TRUE;
}

ZEND_BEGIN_ARG_INFO(arginfo_mqb_include_deleted, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_builder, count) 
{
	RETVAL_FALSE;
	CHECK_MGD;
        zval *argv = NULL;
        
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", 
				&argv) != SUCCESS) {
                return;
        }

	_GET_BUILDER_OBJECT;

        RETURN_LONG(midgard_query_builder_count(builder));
}

ZEND_BEGIN_ARG_INFO(arginfo_mqb_count, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_builder, toggle_read_only) 
{
	RETVAL_FALSE;
	CHECK_MGD;
        zend_bool read_only;
        
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", 
				&read_only) != SUCCESS) {
                return;
        }

	_GET_BUILDER_OBJECT;

	midgard_query_builder_toggle_read_only (builder, read_only);
	RETVAL_TRUE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mqb_toggle_read_only, 0, 0, 1)
        ZEND_ARG_INFO(0, toggle)
ZEND_END_ARG_INFO()

void php_midgard_query_builder_init(int module_number) 
{
	static php_midgard_function_entry query_builder_methods[] = {
		PHP_ME(midgard_query_builder,	__construct,	
				arginfo_mqb___construct,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_builder,	add_constraint,	
				arginfo_mqb_add_constraint,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_builder,	add_constraint_with_property,
			 	arginfo_mqb_add_constraint_with_property,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_builder,	begin_group,	
				arginfo_mqb_begin_group,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_builder,  	end_group,
			     	arginfo_mqb_end_group,			ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_builder,  	execute,
				arginfo_mqb_execute,			ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_builder,  	add_order,
			     	arginfo_mqb_add_order,			ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_builder,  	set_limit,
			     	arginfo_mqb_set_limit,			ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_builder,  	set_offset,
			    	arginfo_mqb_set_offset,			ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_builder,  	set_lang,
			      	arginfo_mqb_set_lang,			ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_builder,  	toggle_read_only,
			      	arginfo_mqb_toggle_read_only,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_builder,   include_deleted,
				arginfo_mqb_include_deleted,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_builder,  	count,
		         	arginfo_mqb_count,			ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};
	
	static zend_class_entry query_builder_class_entry;
	TSRMLS_FETCH();
	
	INIT_CLASS_ENTRY(
			query_builder_class_entry, 
			"midgard_query_builder", query_builder_methods);
	
	php_midgard_query_builder_class =
		zend_register_internal_class(&query_builder_class_entry TSRMLS_CC);

	/* Set function to initialize underlying data */
	php_midgard_query_builder_class->create_object = php_midgard_gobject_new;
}
