/* $Id: preparser.c 27410 2014-09-01 07:39:28Z piotras $
Copyright (C) 1999 Jukka Zitting <jukka.zitting@iki.fi>
Copyright (C) 2000 The Midgard Project ry
Copyright (C) 2000 Emile Heyns, Aurora SA <emile@iris-advies.com>

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "mgd_internal.h"
#include "zend_highlight.h"
#include "ext/standard/basic_functions.h"
#include "mgd_preparser.h"

#include "php_midgard__helpers.h"

#if HAVE_MIDGARD

#include <midgard/midgard.h>

MGD_FUNCTION(ret_type, is_element_loaded, (type param))
{
	char *test_val = NULL, *ename;
	int ename_length;
	midgard_request_config *rcfg = mgd_rcfg();
	RETVAL_FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &ename, &ename_length)  == FAILURE) {
		return;
	}
    
	if (test_val == NULL && rcfg != NULL)
		test_val = g_hash_table_lookup(rcfg->elements, ename);
	
	if (test_val != NULL)
		RETURN_TRUE;
}


char * php_midgard_template(midgard_pool * pool, char * name)
{
	gchar *value = NULL;
	midgard_request_config *rcfg = mgd_rcfg();

	/* TODO : check CHECK_MGD macro to be usefull also here */
	if (mgd_handle() == NULL) {
		php_error(E_ERROR, "Not a midgard request.");
		return NULL;
	}
 
	if (value == NULL && rcfg != NULL)
		value = (gchar *)midgard_pc_get_element(name, rcfg->elements);  
    
	if (value == NULL) {
		return NULL;
	}

	return value;
}

MGD_FUNCTION(ret_type, template, (type param))
{
	char *arg, *tmp;
	int arg_length;
	midgard_pool * pool;
	CHECK_MGD;
	RETVAL_NULL();
  
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_length) == FAILURE) {
		return;
	}

	pool = mgd_alloc_pool();
	tmp = php_midgard_template(pool, arg);

	if( tmp != NULL)
		RETVAL_STRING(tmp,1);

	mgd_free_pool(pool);
}

PHP_FUNCTION(mgd_snippet)
{
	midgard *mgd = mgd_handle(); 
	const gchar *path;
	guint pathl;
	MgdObject *object;	
	CHECK_MGD;
	RETVAL_FALSE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &pathl)  == FAILURE) {
		return;
	}

	if ((object = midgard_object_get_by_path(mgd, "midgard_snippet", path)) != NULL) {
		gchar *code = NULL;
		g_object_get(G_OBJECT(object), "code", &code, NULL);
		if(!code) code = "";
		RETVAL_STRING(code,1);
		g_object_unref(object);
	} else {
		/* Keep it commented right now.
		 * zend_error(E_WARNING, "Could not load requested snippet://%s.", path);
		 */
		RETVAL_STRING("", 1);
	}
}

MGD_FUNCTION(ret_type, snippet_required, (type param))
{
	midgard *mgd = mgd_handle(); 
	const gchar *path;
	guint pathl;
	MgdObject *object;
	CHECK_MGD;
	RETVAL_FALSE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &pathl)  == FAILURE) {
		return;
	}
	   
	if ((object = midgard_object_get_by_path(mgd, "midgard_snippet", path)) != NULL) {
		gchar *code;
		g_object_get(G_OBJECT(object), "code", &code, NULL);
		RETVAL_STRING(code,1);
		g_object_unref(object);
	} else {
		zend_error(E_ERROR, "Could not load requested snippet://%s.", path);
		RETVAL_STRING("", 1);
	}
}

#endif /* HAVE_MIDGARD */
