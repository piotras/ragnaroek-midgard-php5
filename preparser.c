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
    gchar *test_val = NULL, *ename;
    gint enamel;
    midgard_request_config *rcfg = mgd_rcfg();
    

    if (zend_parse_parameters(1 TSRMLS_CC,
                "s", &ename, &enamel)  == FAILURE) {
        RETVAL_FALSE;
        WRONG_PARAM_COUNT;
    }
    
    if (test_val == NULL && rcfg != NULL)
        test_val = g_hash_table_lookup(rcfg->elements, ename);

        if (test_val == NULL){
            RETURN_FALSE; 
        } else {
            RETURN_TRUE; 
        }
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
 
  /* TODO: check this when enabling FTs
	if (MGD_ENV.ft.elements) {
		value = g_hash_table_lookup(MGD_ENV.ft.elements, name);
	}
*/

	if (value == NULL && rcfg != NULL)
    value = (gchar *)midgard_pc_get_element(name, rcfg->elements);  
    
	if (value == NULL) {
		return NULL;
	}

	return value;
}

MGD_FUNCTION(ret_type, template, (type param))
{
	zval **arg;
	char *tmp;
	midgard_pool * pool;

	CHECK_MGD;
  
	if (ZEND_NUM_ARGS() != 1 || zend_parse_parameters(1 TSRMLS_CC, "z", &arg)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string_ex(arg);
	pool = mgd_alloc_pool();
	tmp = php_midgard_template(pool, (*arg)->value.str.val);

	if( tmp== NULL ) {
		RETVAL_NULL();
	} else {
		RETVAL_STRING(tmp,1);
	}

	mgd_free_pool(pool);
}


#if MIDGARD_142MOD

char * php_midgard_include(midgard_pool * pool, char * name, char * value, char * type)
{
	midgard *mgd;

	mgd = mgd_handle();
	if (mgd == NULL) {
		php_error(E_NOTICE, "Not a midgard request");
		return NULL;
	}

	if(!pool && type[0]=='i') { /* inline */
		return value;
	}

	if (!type) {
		return mgd_format(mgd, pool, "$p", value);
	}

	if (strcmp(type, "u") == 0
			|| strcmp(type, "url") == 0)
		return mgd_format(mgd, pool, "$u", value);
	else if (strcmp(type, "f") == 0
		 || strcmp(type, "format") == 0)
		return mgd_format(mgd, pool, "$f", value);
	else if (strcmp(type, "F") == 0
		 || strcmp(type, "format-more") == 0)
		return mgd_format(mgd, pool, "$F", value);
	else if (strcmp(type, "t") == 0
		 || strcmp(type, "text") == 0)
		return mgd_format(mgd, pool, "$h", value);
	else if (strcmp(type, "T") == 0
		 || strcmp(type, "quote-all") == 0)
		return mgd_format(mgd, pool, "$p", value);
	else if (strcmp(type, "h") == 0
		 || strcmp(type, "html") == 0)
		return mgd_format(mgd, pool, "$H", value);
	else
		php_error(E_WARNING, "Unknown format type %s", type);

	return NULL;
}

#endif /* MIDGARD_142MOD */

#if MIDGARD_142MOD

char * php_midgard_variable(midgard_pool * pool, char * name, char * member, char * type)
{
	zval **var;

	if (mgd_handle() == NULL) {
		php_error(E_ERROR, "Not a midgard request.");
		return NULL;
	}

	if (zend_hash_find (EG(active_symbol_table), name,
			 strlen(name)+1, (void **) &var) == FAILURE) {
		g_warning("Uninitialized variable $%s", name);
		return NULL;
	}

	if (member) {

		if (!(Z_TYPE_PP(var) == IS_ARRAY || Z_TYPE_PP(var) == IS_OBJECT)) {
			g_warning("Not an object or array $%s", name);
			return NULL;
		}

		if (!(Z_TYPE_PP(var) == IS_OBJECT)) {
			if(zend_hash_find((*var)->value.ht, member, strlen(member)+1,
						(void **)&var)!= SUCCESS) {
				g_warning("Uninitialized hash property $%s", member);
				return NULL;
			}
		} else {
			if(!MGD_PROPFIND((*var), member, var)) {
				g_warning("Uninitialized member variable $%s",
						member);
				return NULL;
			}
		}
	}


	convert_to_string_ex(var);
	return php_midgard_include(pool, name, (*var)->value.str.val, type);
}

#endif /* MIDGARD_142MOD */

#if MIDGARD_142MOD

MGD_FUNCTION(ret_type, variable, (type param))
{
	zval **var, **arg, **rec;
	char *tmp;
	midgard_pool *pool=NULL;

	if (ZEND_NUM_ARGS() == 1) {
		if( zend_parse_parameters(1 TSRMLS_CC, "z", &var) == FAILURE ) {
			WRONG_PARAM_COUNT;
		} else {
			pool = mgd_alloc_pool();
			convert_to_string_ex(var);
			tmp = php_midgard_variable(pool, (*var)->value.str.val, NULL, NULL);
			if( tmp== NULL ) {
				RETVAL_STRING("",1);
			} else {
				RETVAL_STRING(tmp,1);
			}
		}
	}

	if (ZEND_NUM_ARGS() == 2) {
		if( zend_parse_parameters(2 TSRMLS_CC, "zz", &var,&arg) == FAILURE ) {
			WRONG_PARAM_COUNT;
		} else {
			pool = mgd_alloc_pool();
			convert_to_string_ex(var);
			convert_to_string_ex(arg);
			tmp = php_midgard_variable(pool, (*var)->value.str.val, NULL,
											 (*arg)->value.str.val);
			if( tmp== NULL ) {
				RETVAL_STRING("",1);
			} else {
				RETVAL_STRING(tmp,1);
			}
		}
	}

	if (ZEND_NUM_ARGS() == 3) {
		if( zend_parse_parameters(3 TSRMLS_CC, "zzz", &var,&rec,&arg) == FAILURE ) {
			WRONG_PARAM_COUNT;
		} else {
			pool = mgd_alloc_pool();
			convert_to_string_ex(var);
			convert_to_string_ex(arg);
			convert_to_string_ex(rec);
			if( (*arg)->value.str.val[0] == '\0' ) {
				tmp = php_midgard_variable(pool, (*var)->value.str.val,
												 (*rec)->value.str.val,
												 NULL);
			} else {
				tmp = php_midgard_variable(pool, (*var)->value.str.val,
												 (*rec)->value.str.val,
												 (*arg)->value.str.val);
			}
			if( tmp== NULL ) {
				RETVAL_STRING("",1);
			} else {
				RETVAL_STRING(tmp,1);
			}
		}
	}
	mgd_free_pool(pool);
}

#endif /* MIDGARD_142MOD */

/* DG {HACK ALERT}: Since the function zend_eval_string does not behave like
 * the statement eval(), thie following function is the exact copy of the
 * function zend_eval_string with some lines commented out.
 * This is necessary to keep the compatibility between eval() and mgd_eval()
 */
static int mgd_eval_string(char *str, zval *retval_ptr, char *string_name CLS_DC ELS_DC)
{
	zval pv;
	zend_op_array *new_op_array;

  TSRMLS_FETCH();
  
	zend_op_array *original_active_op_array = EG(active_op_array);
	php_mgd_function_state original_function_state_ptr = php_mgd_function_state_ptr();
	int retval;

//	if (retval_ptr) {
//		pv.value.str.len = strlen(str)+sizeof("return  ;")-1;
//		pv.value.str.val = emalloc(pv.value.str.len+1);
//		strcpy(pv.value.str.val, "return ");
//		strcat(pv.value.str.val, str);
//		strcat(pv.value.str.val, " ;");
//	} else {
		pv.value.str.len = strlen(str);
		pv.value.str.val = estrndup(str, pv.value.str.len);
//	}
	pv.type = IS_STRING;

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 3
	int original_handle_op_arrays = CG(handle_op_arrays);
	CG(handle_op_arrays) = 0;
#else
# warning "What should we do here?"
#endif
	new_op_array = compile_string(&pv, string_name TSRMLS_CC);
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 3
	CG(handle_op_arrays) = original_handle_op_arrays;
#endif

	if (new_op_array) {
		zval *local_retval_ptr=NULL;
		zval **original_return_value_ptr_ptr = EG(return_value_ptr_ptr);
		zend_op **original_opline_ptr = EG(opline_ptr);

		EG(return_value_ptr_ptr) = &local_retval_ptr;
		EG(active_op_array) = new_op_array;
		EG(no_extensions)=1;

		zend_execute(new_op_array TSRMLS_CC);

		if (local_retval_ptr) {
			if (retval_ptr) {
				COPY_PZVAL_TO_ZVAL(*retval_ptr, local_retval_ptr);
			} else {
				zval_ptr_dtor(&local_retval_ptr);
			}
		} else {
			if (retval_ptr) {
				INIT_ZVAL(*retval_ptr);
			}
		}

		EG(no_extensions)=0;
		EG(opline_ptr) = original_opline_ptr;
		EG(active_op_array) = original_active_op_array;
		php_mgd_function_state_ptr() = original_function_state_ptr;
		destroy_op_array(new_op_array TSRMLS_CC);
		efree(new_op_array);
		EG(return_value_ptr_ptr) = original_return_value_ptr_ptr;
		retval = SUCCESS;
	} else {
		retval = FAILURE;
	}
	zval_dtor(&pv);
	return retval;
}

#if MIDGARD_142MOD

MGD_FUNCTION(ret_type, eval, (type param))
{
 	zval **string, **name;
	char *tmp;
  GByteArray *buffer;
  midgard_request_config *rcfg = mgd_rcfg(); 
  
   
	RETVAL_FALSE;
	switch (ZEND_NUM_ARGS()) {
		case 2:
			if (zend_parse_parameters(2 TSRMLS_CC, "zz", &string, &name) == SUCCESS) {
				convert_to_string_ex(string);
				convert_to_string_ex(name);
				tmp = (*name)->value.str.val;
				break;
			}
		case 1:
			if (zend_parse_parameters(1 TSRMLS_CC, "z", &string) == SUCCESS) {
				convert_to_string_ex(string);
				tmp = "mgd_eval()";
				break;
			}
		default:
			WRONG_PARAM_COUNT;
	}

	if((*string)->value.str.len) {
      buffer = mgd_preparse_string((*string)->value.str.val);

/* OUTPUT IS SENT TO BROWSER WARNING! , uncomment if You really want this 
  
		if(mgd_eval_string(buffer->data, return_value, tmp CLS_CC ELS_CC) != SUCCESS) {

			zend_syntax_highlighter_ini syntax_highlighter_ini;
			php_get_highlight_struct(&syntax_highlighter_ini);
			highlight_string(*string, &syntax_highlighter_ini, tmp);
         g_byte_array_free(buffer, TRUE);
			zend_bailout(); //exit(0);
		}
*/
    
      g_byte_array_free(buffer, TRUE);
	}
}

#endif

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, snippet, (type param))
#else 
PHP_FUNCTION(mgd_snippet)
#endif
{
	midgard *mgd = mgd_handle(); 
	const gchar *path;
	guint pathl;
	MgdObject *object;
	
	CHECK_MGD;
	
	if (zend_parse_parameters(1 TSRMLS_CC,
				"s", &path, &pathl)  == FAILURE) {
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if((object = midgard_object_get_by_path(
					mgd, "midgard_snippet", path)) != NULL) {
		
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
	
	if (zend_parse_parameters(1 TSRMLS_CC,
				"s", &path, &pathl)  == FAILURE) {
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
	   
	if((object = midgard_object_get_by_path(
					mgd, "midgard_snippet", path)) != NULL) {
		
		gchar *code;
		g_object_get(G_OBJECT(object), "code", &code, NULL);
		RETVAL_STRING(code,1);
		g_object_unref(object);
	
	} else {
		zend_error(E_ERROR, "Could not load requested snippet://%s.", path);
		RETVAL_STRING("", 1);
	}
}

MGD_FUNCTION(ret_type, ref, (type param))
{
  zval **mode, **guid, **attrx, **attry, **label;
  midgard_res *res;
  midgard_pool *pool;
  int id, params;
  char *path, *guidstr, *namestr, *table, *prefix, *ah_prefix;
  midgard_request_config *rcfg;

  switch (ZEND_NUM_ARGS()) {
  case 4:
    if (zend_parse_parameters(4 TSRMLS_CC, "zzzz", &mode, &guid, &attrx, &attry) == FAILURE ) {
      WRONG_PARAM_COUNT;
    }
    params =4;
    break;
  case 5:
    if( zend_parse_parameters(5 TSRMLS_CC, "zzzzz", &mode, &guid, &attrx, &attry, &label) == FAILURE ) {
      WRONG_PARAM_COUNT;
    }
    params = 5;
    break;
  default:
    WRONG_PARAM_COUNT;
  }

  convert_to_long_ex(mode);
  convert_to_string_ex(guid);
  convert_to_string_ex(attrx);
  convert_to_string_ex(attry);
  if (params == 5) {    convert_to_string_ex(label);
  }
  rcfg = mgd_rcfg();

  if (rcfg == NULL) {
    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_CONNECTED);
  }

  pool = mgd_alloc_pool();

  guidstr = mgd_strndup(pool, (*guid)->value.str.val, 32);
  namestr = strchr((*guid)->value.str.val, '/');

  res = mgd_sitegroup_select(mgd_handle(), "realm,id", "repligard",
			     "guid=$q", NULL, guidstr);
  if (!res || !mgd_fetch(res)) {
    if (res) mgd_release(res);
    RETURN_STRING("",1);
  }

  table = mgd_strdup(pool, mgd_colvalue(res, 0));
  id = atoi(mgd_colvalue(res, 1));

  mgd_release(res);

  prefix = mgd_strndup(pool, rcfg->uri, rcfg->prelen);
  ah_prefix = mgd_get_ah_prefix(mgd_handle());

  if ((*mode)->value.lval) {
    if (params == 4 && !strcmp(table, "blobs") && mgd_exists_id(mgd_handle(), "blobs", "id=$d", id)) {
      if (namestr) {
        RETVAL_STRING(g_strconcat("<img", 
              (*attrx)->value.str.val, "\"", prefix, "/", 
              ah_prefix, "/", guidstr, namestr, "\"", 
              (*attry)->value.str.val, " />", NULL), 1);  
      } else {
        RETVAL_STRING(g_strconcat("<img", 
              (*attrx)->value.str.val, "\"", prefix, "/", 
              ah_prefix, "/", guidstr, "\"", 
              (*attry)->value.str.val, " />", NULL), 1);
      }
    } else {
      RETVAL_STRING("",1);
    }
  } else {
    if (params == 5) {
      if (!strcmp(table, "blobs") && mgd_exists_id(mgd_handle(), "blobs", "id=$d", id)) {
        if (namestr) {
          RETVAL_STRING(g_strconcat("<a",
                (*attrx)->value.str.val, "\"", 
                prefix, "/", ah_prefix, "/", guidstr, 
                namestr, "\"", (*attry)->value.str.val, ">", 
                (*label)->value.str.val, "</a>", NULL), 1);
        } else {
          RETVAL_STRING(g_strconcat("<a",  
                (*attrx)->value.str.val, "\"",prefix, "/", 
                ah_prefix, "/", guidstr, "\"", (*attry)->value.str.val, ">", 
                (*label)->value.str.val, "</a>", NULL), 1);
        }
      } else if (!strcmp(table, "page") && mgd_exists_id(mgd_handle(), "page", "id=$d", id)) {
        path = NULL;
        while ((res = mgd_sitegroup_record(mgd_handle(), "up, name", "page", id))) {
          if (!res || !mgd_fetch(res)) {
            if (res) mgd_release(res);     
	    break;
          
          } else {
            
            id = atoi(mgd_colvalue(res, 0));
            if (id > 0) {
              if (path) {
                path = g_strconcat(mgd_colvalue(res, 1), "/", path, NULL);
              } else {
                path = g_strconcat(mgd_colvalue(res, 1), "/", NULL);
              }
            } else {
              path = path?path:"";
            }
          }
          mgd_release(res);
        }
        if (path) {
          RETVAL_STRING(g_strconcat("<a",
                (*attrx)->value.str.val, "\"",prefix, "/", 
                path, "\"", (*attry)->value.str.val, ">", 
                (*label)->value.str.val, "</a>", NULL), 1);
        } else {
          RETVAL_STRING("",1);
        }
      } else {
        RETVAL_STRING("",1);
      }
    } else {
      RETVAL_STRING("",1);
    }
  }
  mgd_free_pool(pool);
}


#endif /* HAVE_MIDGARD */
