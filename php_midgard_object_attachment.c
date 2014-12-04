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

PHP_FUNCTION(_php_midgard_object_list_attachments)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();	
	MgdObject **objects = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") != SUCCESS) 
		return;

	MgdObject *mobj = php_midgard_get_midgard_object(zval_object);
	objects = midgard_object_list_attachments(mobj);	

	zval *ret_arr;
	MAKE_STD_ZVAL(ret_arr);
	array_init(ret_arr);
	
	if(objects) {	
		php_midgard_array_from_objects((GObject **)objects, "midgard_attachment", ret_arr);
		g_free(objects);
	}

	RETURN_ZVAL(ret_arr, 1, 0);
}

PHP_FUNCTION(_php_midgard_object_delete_attachments)
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
	rv = midgard_object_delete_attachments(mobj, n_params, parameters);

	_FREE_PARAMETERS;

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_purge_attachments)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	zval *params = NULL;
	guint n_params = 0;
	gboolean rv = FALSE;
	zend_bool zbool = TRUE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"|zz", &zbool, &params) != SUCCESS) {
		return;
	}	

	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params);
	MgdObject *mobj = php_midgard_get_midgard_object(zval_object);
	rv = midgard_object_purge_attachments(mobj, (gboolean) zbool, n_params, parameters);

	_FREE_PARAMETERS;

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_find_attachments)
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
	objects = midgard_object_find_attachments(mobj, n_params, parameters);

	_FREE_PARAMETERS;

	zval *ret_arr;
	MAKE_STD_ZVAL(ret_arr);
	array_init(ret_arr);
	
	if(objects) {	
		php_midgard_array_from_objects((GObject **)objects, "midgard_attachment", ret_arr);
		g_free(objects);
	}

	RETURN_ZVAL(ret_arr, 1, 0);
}

PHP_FUNCTION(_php_midgard_object_create_attachment)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();	
	gchar *class_name;
	const gchar *type_name;
	zend_class_entry **ce;
	const gchar *name = NULL, *title = NULL, *mimetype = NULL;
	guint name_length, title_length, mimetype_length;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"|sss", &name, &name_length, 
				&title, &title_length, 
				&mimetype, &mimetype_length) != SUCCESS) {
		return;
	}	

	MgdObject *mobj = php_midgard_get_midgard_object(zval_object);
	MgdObject *att = midgard_object_create_attachment(mobj, name, title, mimetype);

	if(att) {
		
		type_name = G_OBJECT_TYPE_NAME((GObject*)att);
		class_name = g_ascii_strdown(type_name, strlen(type_name));
		zend_hash_find(CG(class_table),
				class_name, strlen(class_name)+1, (void **) &ce);
		g_free(class_name);

		php_midgard_gobject_new_with_gobject(return_value, *ce,	G_OBJECT(att), TRUE);
		g_signal_emit(att, MIDGARD_OBJECT_GET_CLASS(att)->signal_action_loaded_hook, 0);
	
	} else {
		
		RETURN_NULL();
	}
}

/* It's not binded from core. This is PHP specific */
PHP_FUNCTION(_php_midgard_object_serve_attachment)
{
	RETVAL_FALSE;
	CHECK_MGD;	
	MgdObject *att = NULL;
	const gchar *guid;
	guint guid_length;


	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"s", &guid, &guid_length) != SUCCESS) {
		return;
	}	

	if(!midgard_is_guid(guid)) {
		g_warning("Given parameter is not a guid");
		return;
	}

	GValue gval = {0, };
	g_value_init(&gval, G_TYPE_STRING);
	g_value_set_string(&gval, guid);
	att = midgard_object_new(mgd_handle(), "midgard_attachment", &gval);

	/* error is set by core */
	if(!att)
		return;

	MidgardBlob *blob = midgard_blob_new(att, NULL);

	if(!blob)
		return ;
	gchar *mimetype; 
	g_object_get(G_OBJECT(att), "mimetype", &mimetype, NULL);
	gchar *content_type = g_strconcat("Content-type: ", mimetype, NULL);
	sapi_add_header(content_type, strlen(content_type), 1);
	g_free(content_type);

	if (sapi_send_headers(TSRMLS_C) != SUCCESS) 
		return;

	const gchar *path = midgard_blob_get_path(blob);
	FILE *fp;
	int b;
	char buf[1024];

	if (!(fp = fopen(path, "r"))) {
		g_warning("File doesn't exist");
		MIDGARD_ERRNO_SET(mgd_handle(), MGD_ERR_INTERNAL);
		return;
	}

	while ((b = fread(buf, 1, sizeof(buf), fp)) > 0) {
		PHPWRITE(buf, b);
	}
	
	fclose(fp);

	RETVAL_TRUE;;
}
