/* $Id: parameter.c 15758 2008-03-18 12:52:39Z piotras $
Copyright (C) 1999 Jukka Zitting <jukka.zitting@iki.fi>
Copyright (C) 2000 The Midgard Project ry
Copyright (C) 2000 Emile Heyns, Aurora SA <emile@iris-advies.com>
Copyright (C) 2003 David Schmitter, Dataflow Solutions GmbH <schmitt@dataflow.ch>

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
#include "php_midgard.h"
#include "mgd_oop.h"

#ifdef PHP_MIDGARD_LEGACY_API

MGD_FUNCTION(ret_type, oop_parameter, (type param))
{
	/* sitegroup_property and current_sitegroup are used in _MGD_SITEGROUP_FORCE macros */
	zval *self;
	zval **zv_domain, **zv_name, **zv_value, **zv_nofallback, **sitegroup_property;
	/* zval **zv_lang; */
	int lang = mgd_get_default_lang(mgd_handle());
	int current_sitegroup, id = 0, exists;
	zval **zv_table, **zv_id, **zv_parent_guid;
	midgard_res *res;
	zend_class_entry *ce;
	CHECK_MGD;

	MGD_PHP_PCLASS_NAME;

	self = getThis();
	if (self == NULL) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
	}
	
	if (!MGD_PROPFIND(self, "guid", zv_parent_guid)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
	}
	if (!MGD_PROPFIND(self, "__table__", zv_table)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
	}
	if (!MGD_PROPFIND(self, "id", zv_id)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
	}

	convert_to_string_ex(zv_parent_guid);
	convert_to_string_ex(zv_table);
	convert_to_long_ex(zv_id);

    
  switch (ZEND_NUM_ARGS()) {
    
	case 2:
	  if (zend_get_parameters_ex(2, &zv_domain, &zv_name) ==
	      FAILURE) {
	    WRONG_PARAM_COUNT;
	  }
	  convert_to_string_ex(zv_domain);
	  convert_to_string_ex(zv_name);
	  zv_value = NULL;
	  zv_nofallback = NULL;
	  break;

  case 3:
	  if (zend_get_parameters_ex
	      (3, &zv_domain, &zv_name, &zv_value) == FAILURE) {
	    WRONG_PARAM_COUNT;
	  }
	  convert_to_string_ex(zv_domain);
	  convert_to_string_ex(zv_name);
	  convert_to_string_ex(zv_value);
	  zv_nofallback = NULL;
	  break;

	case 4:
	  if (zend_get_parameters_ex(4, &zv_domain, &zv_name, &zv_value, &zv_nofallback) ==
	      FAILURE) {
	    WRONG_PARAM_COUNT;
	  }
	  convert_to_string_ex(zv_domain);
	  convert_to_string_ex(zv_name);
	  convert_to_string_ex(zv_value);
	  convert_to_boolean_ex(zv_nofallback);
    
    if ((*zv_nofallback)->value.lval == TRUE)
      lang = mgd_lang(mgd_handle());
    
			break;

		default:
			WRONG_PARAM_COUNT;
	}

  /* Uncomment if you need to set parameter by object's lang */
  /*
	if (is_table_multilang((*zv_table)->value.str.val)) {
	  if (zend_hash_find(self->value.obj.properties, "lang", 5,
			     (void **) &zv_lang) !=
	      SUCCESS) {
	    lang = mgd_lang(mgd_handle());
	  } else {
	    convert_to_long_ex(zv_lang);
	    lang = (*zv_lang)->value.lval;
	  }
	}
  */
  
	res = mgd_ungrouped_select(mgd_handle(), "id,value,sitegroup",
				   "record_extension",
				   "parent_guid=$q AND domain=$q AND name=$q"
				   " AND (lang=$d OR lang=$d)",
				   "lang DESC",
				   (*zv_parent_guid)->value.str.val,
				   (*zv_domain)->value.str.val,
				   (*zv_name)->value.str.val,
				   mgd_lang(mgd_handle()),
				   mgd_get_default_lang(mgd_handle())
				   );

	exists = res ? mgd_fetch(res) : 0;
	if (exists)
		id = atoi(mgd_colvalue(res, 0));

	/* delete parameter */
	if (zv_value && (*zv_value)->value.str.len == 0) {

		if (res) { mgd_release(res); }

		/* Use legacy ACL only if this is legacy object instance */
		if(!g_type_from_name(ce->name)){
			if (!isglobalowner(
						mgd_lookup_table_id((*zv_table)->value.str.val),
						(*zv_id)->value.lval)) {
				RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
			}
		}

		if (!exists) {
			RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
		}

		_MGD_SITEGROUP_FORCE();
		if (mgd_delete(mgd_handle(), "record_extension", id)) {
			PHP_DELETE_REPLIGARD("record_extension", id);
			_MGD_SITEGROUP_FORCE_REVERT();
			RETURN_TRUE;
		}
		else {
			_MGD_SITEGROUP_FORCE_REVERT();
			RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
		}
	}

	/* return parameter */
	if (!zv_value) {

		if (!res) {
			RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
		}
		if (!exists) {
			mgd_release(res);
			RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
		}

		RETVAL_STRING((char *) mgd_colvalue(res, 1), 1);
	}
	else {
		/* update parameter */

        lang = mgd_parameters_defaultlang(mgd_handle())? mgd_get_default_lang(mgd_handle()) :lang;

		if (exists) {
			/* Use legacy ACL only if this is legacy object instance */
			if(!g_type_from_name(ce->name)){
				if (!isglobalowner(
							mgd_lookup_table_id(
								(*zv_table)->value.str.val),
							(*zv_id)->value.lval)) {
					if (res) mgd_release(res);
					RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
				}
			}

			_MGD_SITEGROUP_FORCE();
			php_midgard_update(return_value, "record_extension",
					   "value=$q, lang=$d", id,
					   (*zv_value)->value.str.val, 
                       lang);
			PHP_UPDATE_REPLIGARD("record_extension", id);
			_MGD_SITEGROUP_FORCE_REVERT();
		} else {
			/* Use legacy ACL only if this is legacy object instance */
			if(!g_type_from_name(ce->name)){
				if (!isglobalowner(
							mgd_lookup_table_id(
								(*zv_table)->value.str.val),
							(*zv_id)->value.lval)) {
					RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
				}
			}

      _MGD_SITEGROUP_FORCE();
      if (!MGD_PROPFIND(self, "guid", zv_parent_guid)) {
	      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
      }
      convert_to_string_ex(zv_parent_guid);
      php_midgard_create(return_value, NULL, "record_extension",
					   "parent_guid, tablename,oid,domain,name,value,lang",
					   "$q,$q,$d,$q,$q,$q,$d",
					   (*zv_parent_guid)->value.str.val,
					   (*zv_table)->value.str.val,
					   (*zv_id)->value.lval,
					   (*zv_domain)->value.str.val,
					   (*zv_name)->value.str.val,
					   (*zv_value)->value.str.val
					   ,lang
					   );
	PHP_CREATE_REPLIGARD("record_extension",
			return_value->value.lval);
	_MGD_SITEGROUP_FORCE_REVERT();	
		}
	}

	if (res)
		mgd_release(res);
}

MGD_FUNCTION(ret_type, oop_parameter_list, (type param))
{
	zval *self;
	zval **zv_domain;
#if HAVE_MIDGARD_MULTILANG
	zval **zv_lang;
	int lang = mgd_get_default_lang(mgd_handle());
#endif /* HAVE_MIDGARD_MULTILANG */
	zval **zv_table, **zv_id, **zv_parent_guid;;
	CHECK_MGD;

	self = getThis();
	if (self == NULL) 
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
	
	if(!MGD_PROPFIND(self, "guid", zv_parent_guid)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
	}

	if (!MGD_PROPFIND(self, "id", zv_id)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
	}
	if (!MGD_PROPFIND(self, "__table__", zv_table)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
	}
  
	convert_to_string_ex(zv_parent_guid);
	convert_to_string_ex(zv_table);
	convert_to_long_ex(zv_id);

	switch (ZEND_NUM_ARGS()) {
		case 1:
			if (zend_get_parameters_ex(1, &zv_domain) == FAILURE) {
				WRONG_PARAM_COUNT;
			}
			convert_to_string_ex(zv_domain);
			break;

		case 0:
			zv_domain = NULL;
			break;

		default:
			WRONG_PARAM_COUNT;
	}

	if (is_table_multilang((*zv_table)->value.str.val)) {
        if (!MGD_PROPFIND(self, "lang", zv_lang)) {
            lang = mgd_lang(mgd_handle());
        } else {
            convert_to_long_ex(zv_lang);
            lang = (*zv_lang)->value.lval;
        }
    }

	if (zv_domain)
		php_midgard_select(&MidgardParameter, return_value,
               "id,domain,name,value",
				   "record_extension",
				   "parent_guid=$q AND domain=$q"
#if HAVE_MIDGARD_MULTILANG   
				   " AND lang=$d"
#endif /* HAVE_MIDGARD_MULTILANG */
				   ,
				   "name",
				   (*zv_parent_guid)->value.str.val,
				   (*zv_domain)->value.str.val
#if HAVE_MIDGARD_MULTILANG
				   ,lang
#endif /* HAVE_MIDGARD_MULTILANG */
				   );
	else
		php_midgard_select(&MidgardParameter, return_value,
               "DISTINCT domain",
				   "record_extension",
				   "parent_guid=$q"
#if HAVE_MIDGARD_MULTILANG
				   " AND lang=$d"
#endif /* HAVE_MIDGARD_MULTILANG */
				   , "domain",
				   (*zv_parent_guid)->value.str.val				   
#if HAVE_MIDGARD_MULTILANG
				   ,lang
#endif /* HAVE_MIDGARD_MULTILANG */
				   );
}

MGD_FUNCTION(ret_type, oop_parameter_search, (type param))
{
	zval *self, **table;
	zval **where, **all;
	char *cond = NULL;
	midgard_pool *pool;

	RETVAL_FALSE;

	CHECK_MGD;

	self = getThis();
	if (self == NULL) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
   }

    if (!MGD_PROPFIND(self, "__table__", table)) {
        RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
    }

	convert_to_string_ex(table);

	switch (ZEND_NUM_ARGS()) {
		case 1:
			if (zend_get_parameters_ex(1, &where) != SUCCESS) {
				WRONG_PARAM_COUNT;
			}
			convert_to_string_ex(where);
			all = NULL;
			break;
		case 2:
			if (zend_get_parameters_ex(2, &where, &all) != SUCCESS) {
				WRONG_PARAM_COUNT;
			}
			convert_to_string_ex(where);
			convert_to_boolean_ex(all);
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	if (*((*table)->value.str.val) == '\0') {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}

	pool = mgd_alloc_pool();
	if (pool == NULL) {
		RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
	}

	cond = mgd_format(mgd_handle(), pool, "tablename = $q AND ($s)",
			  (*table)->value.str.val,
			  ((*((*where)->value.str.val) != '\0') ? (*where)->
			   value.str.val : "1=1")
	   );

	if (all != NULL && (*all)->value.lval) {
		php_midgard_select(&MidgardParameter, return_value,
				   "oid AS id,id AS pid,domain,name,value",
				   "record_extension", cond, NULL);
	}
	else {
		php_midgard_select(&MidgardParameter, return_value,
				   "DISTINCT oid AS id",
				   "record_extension", cond, NULL);
	}

	mgd_free_pool(pool);
}

static zend_function_entry MidgardParameterMethods[] = {
   PHP_FALIAS(fetch,    mgd_oop_list_fetch,        NULL)
   MIDGARD_OOP_SITEGROUP_METHODS
   {  NULL,             NULL,                      NULL}
};
MidgardClass MidgardParameter = {
   "MidgardParameter",
   "record_extension",
   MidgardParameterMethods,
   {},
   NULL
};

#endif 
