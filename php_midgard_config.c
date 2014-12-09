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

static zend_class_entry *php_midgard_config_class;

/* Object constructor */
static PHP_METHOD(midgard_config, __construct)
{
	RETVAL_FALSE;

	zval *object = getThis();

	if (ZEND_NUM_ARGS() > 0) 
		WRONG_PARAM_COUNT;

	MidgardConfig *config = 
		midgard_config_new();

	if(!config)
		RETURN_FALSE;

	php_midgard_gobject *php_gobject = 
		(php_midgard_gobject *)zend_object_store_get_object(object TSRMLS_CC);
	php_gobject->gobject = G_OBJECT(config);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_config___construct, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_config, save_file)
{
	RETVAL_FALSE;
	gboolean rv;
	zend_bool zbool = FALSE;
	zval *zval_object = getThis();

	gchar *name;
	guint name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b",
				&name, &name_length, &zbool) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	php_midgard_gobject *php_gobject =
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);

	MidgardConfig *config =
		(MidgardConfig *) php_gobject->gobject;

	rv = midgard_config_save_file(config ,name, zbool);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_config_save_file, 0, 0, 1)
        ZEND_ARG_INFO(0, name)
        ZEND_ARG_INFO(0, user)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_config, read_file)
{
	RETVAL_FALSE;
	gboolean rv;
	zend_bool zbool = FALSE;
	zval *zval_object = getThis();

	gchar *name;
	guint name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b",
				&name, &name_length, &zbool) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	php_midgard_gobject *php_gobject = 
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	
	MidgardConfig *config =
		(MidgardConfig *) php_gobject->gobject;

	rv = midgard_config_read_file(config, name, zbool);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_config_read_file, 0, 0, 1)
        ZEND_ARG_INFO(0, name)
        ZEND_ARG_INFO(0, user)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_config, create_midgard_tables)
{
	RETVAL_FALSE;
	gboolean rv;
	zval *zval_object = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, ""
				) == FAILURE) {
		return;
	}
	
	MidgardConfig *config = NULL;

	if(zval_object) {
		php_midgard_gobject *php_gobject = 
			(php_midgard_gobject *)zend_object_store_get_object(
					zval_object TSRMLS_CC);
		
		config =
			(MidgardConfig *) php_gobject->gobject;
	}

	rv = midgard_config_create_midgard_tables(config, mgd_handle()->_mgd);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_config_create_midgard_tables, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_config, list_files)
{
	RETVAL_FALSE;
	zend_bool user = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,  
				"|b", &user) == FAILURE) {
		return;
	}
	
	array_init(return_value);

	gchar **files = midgard_config_list_files(user);

	if(!files)
		return;

	guint i = 0;
	while(files[i] != NULL) {	
		add_index_string(return_value, i,
				(gchar *)files[i], 1);
		i++;
	}
	
	g_strfreev(files);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_config_list_files, 0, 0, 0)
        ZEND_ARG_INFO(0, user)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_config, create_class_table)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gboolean rv;
	zval *zval_object = getThis();

	gchar *name;
	guint name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&name, &name_length) == FAILURE) {
		return;
	}
	
	MidgardConfig *config = NULL;

	if(zval_object) {
		
		php_midgard_gobject *php_gobject = 
			(php_midgard_gobject *)zend_object_store_get_object(
					zval_object TSRMLS_CC);
		
		config =
			(MidgardConfig *) php_gobject->gobject;
	}

	MidgardObjectClass *klass = 
		MIDGARD_OBJECT_GET_CLASS_BY_NAME(name);

	if(!klass)
		return;

	rv = midgard_config_create_class_table(config, klass, mgd_handle()->_mgd);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_config_create_class_table, 0, 0, 1)
        ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_config, class_table_exists)
{
	RETVAL_FALSE;
	gboolean rv;
	zval *zval_object = getThis();

	gchar *name;
	guint name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&name, &name_length) == FAILURE) {
		return;
	}
	
	MidgardConfig *config = NULL;

	if(zval_object) {
		
		php_midgard_gobject *php_gobject = 
			(php_midgard_gobject *)zend_object_store_get_object(
					zval_object TSRMLS_CC);
		
		config =
			(MidgardConfig *) php_gobject->gobject;
	}

	MidgardObjectClass *klass = 
		MIDGARD_OBJECT_GET_CLASS_BY_NAME(name);

	if(!klass)
		return;

	rv = midgard_config_class_table_exists(config, klass, mgd_handle()->_mgd);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_config_class_table_exists, 0, 0, 1)
        ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_config, update_class_table)
{
	RETVAL_FALSE;
	gboolean rv;
	zval *zval_object = getThis();

	gchar *name;
	guint name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&name, &name_length) == FAILURE) {
		return;
	}
	
	MidgardConfig *config = NULL;

	if(zval_object) {
		
		php_midgard_gobject *php_gobject = 
			(php_midgard_gobject *)zend_object_store_get_object(
					zval_object TSRMLS_CC);
		config =
			(MidgardConfig *) php_gobject->gobject;
	}

	MidgardObjectClass *klass = 
		MIDGARD_OBJECT_GET_CLASS_BY_NAME(name);

	if(!klass)
		RETURN_BOOL(FALSE);

	rv = midgard_config_update_class_table(config, klass, mgd_handle()->_mgd);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_config_update_class_table, 0, 0, 1)
        ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

/* Initialize ZEND&PHP class */
void php_midgard_config_init(int module_numer)
{

	static php_midgard_function_entry midgard_config_methods[] = {
		PHP_ME(midgard_config,	__construct,	
			arginfo_midgard_config___construct,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_config,	save_file,	
			arginfo_midgard_config_save_file,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_config,	read_file,	
			arginfo_midgard_config_read_file,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_config,  list_files,      
			arginfo_midgard_config_list_files,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_config,  create_midgard_tables, 
			arginfo_midgard_config_create_midgard_tables,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_config,  create_class_table,	
			arginfo_midgard_config_create_class_table,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_config,  class_table_exists,     
			arginfo_midgard_config_class_table_exists,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_config,  update_class_table,     
			arginfo_midgard_config_update_class_table,	ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_config_class_entry;
	TSRMLS_FETCH();

	INIT_CLASS_ENTRY(
			php_midgard_config_class_entry,
			"midgard_config", midgard_config_methods);

	php_midgard_config_class =
		zend_register_internal_class(
				&php_midgard_config_class_entry TSRMLS_CC);
	
	/* Set function to initialize underlying data */
	php_midgard_config_class->create_object = php_midgard_gobject_new;	
}
