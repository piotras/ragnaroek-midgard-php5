/* $Id: oop.c 27410 2014-09-01 07:39:28Z piotras $
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
#include "mgd_oop.h"
#include "php_midgard.h"

#include "php_midgard_gobject.h"

typedef char *charp;
void mgd_object_init(zval *obj, ...)
{
	va_list args;
	char *propname;
	TSRMLS_FETCH();

   	va_start(args, obj);
   	while ((propname = va_arg(args, charp)) != NULL) {
      		add_property_unset(obj, propname);
   	}
   	va_end(args);
}

void php_midgard_bless(zval *object, MidgardClass *species)
{
	TSRMLS_FETCH();

	if (species->entry_ptr == NULL) {
		fprintf(stderr, "SEVERE: No PHP class entry pointer for Midgard class '%s'\n", species->name);
		return;
	}

	object_init_ex(object, species->entry_ptr);
	php_midgard_ctor(object, species);
}

void php_midgard_ctor(zval *object, MidgardClass *species)
{
	MidgardProperty *midgard_props;
	TSRMLS_FETCH();

	/* add default properties */
	add_property_string(object, "__table__", (char*)species->table, 1);
	add_property_long(object, "id", 0);
	add_property_long(object, "sitegroup", 0);
	add_property_string(object, "guid", "", 1);
	/* add other properties */
	for (midgard_props = species->properties; midgard_props && 
							midgard_props->name; midgard_props++) {
	  
		switch(midgard_props->type) {
			case IS_LONG:
				add_property_long(object, midgard_props->name, 0);
				break;
			case IS_STRING:
				add_property_string(object, midgard_props->name, "", 1);
				break;
			default: /* should not happen, but just in case */
				add_property_unset(object, midgard_props->name);
				break;
		}
	}
}

void php_midgard_bless_oop(zval *object, MidgardClass *species)
{
	TSRMLS_FETCH();
	
	if (species->entry_ptr == NULL) {
		fprintf(stderr, "SEVERE: No PHP class entry pointer for Midgard class '%s'\n", species->name);
		return;
	}
	
	object_init_ex(object, species->entry_ptr);

	add_property_string(object, "__table__", (char*)species->table, 1);
	add_property_string(object, "guid", "", 1);
	add_property_long(object, "id", 0);
	add_property_long(object, "sitegroup", 0);
}

void php_midgard_delete(zval * return_value, const char *table, int id)
{
	TSRMLS_FETCH();
	CHECK_MGD;

	if (mgd_delete(mgd_handle(), table, id)) {
		RETURN_TRUE;
	}
	
	RETVAL_FALSE_BECAUSE(MGD_ERR_INTERNAL);
}

void php_midgard_delete_repligard(const char *table, int id)
{
	if (!mgd_rcfg())
		return;

	(void) mgd_delete_repligard(mgd_handle(), table, id);
}

void php_midgard_update(zval * return_value, const char *table,
			const char *fields, int id, ...)
{
	int ret;
	va_list args;
	TSRMLS_FETCH();
	CHECK_MGD;

   	va_start(args, id);
	if ((ret = mgd_vupdate(mgd_handle(), table, id, fields, args)) > 0) {
		RETVAL_TRUE;
	}
	else {
#if HAVE_MIDGARD_QUOTA
	     if (ret == -64) RETVAL_FALSE_BECAUSE(MGD_ERR_QUOTA)
	     else	  RETVAL_FALSE_BECAUSE(MGD_ERR_INTERNAL);
#else
	     RETVAL_FALSE_BECAUSE(MGD_ERR_INTERNAL);
#endif
	}
	va_end(args);
}

void php_midgard_create(zval * return_value, zval * self, const char *table,
			const char *fields, const char *values, ...)
{
	va_list args;
	int id;
	TSRMLS_FETCH();
	CHECK_MGD;

	va_start(args, values);
	id = mgd_vcreate(mgd_handle(), table, fields, values, args);
	va_end(args);
	
	if (!id) {
		RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
		
		/* EEH: TODO: log_error(
		 * * "Midgard: create of %s failed",
		 * table ? table : "<null class>"); */
	}
#if HAVE_MIDGARD_QUOTA
	else if (id == -64) {
		RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
	}
#endif 

	if (self) { add_property_long(self, "id", id); }
	RETVAL_LONG(id);
}

void php_midgard_select(MidgardClass *species, zval * return_value,
			const char *fields, const char *tables,
			const char *where, const char *order, ...)
{
	va_list args;
	midgard_res *res;
   	int res_id;
	TSRMLS_FETCH();

	va_start(args, order);
	res =
	   mgd_sitegroup_vselect(mgd_handle(), fields, tables, where, order,
				 args);
	va_end(args);
	if (res) {
		/* 
		 * ZEND_REGISTER_RESOURCE(return_value,res,le_midgard_list_fetch);
		 * PP: I do not know if return_value may be used as resource property.
		 * Anyway It is good to register resource before inserting it to Zend resources list */
		res_id = zend_list_insert(res, le_midgard_list_fetch);
		php_midgard_bless_oop(return_value, species);
		add_property_long(return_value, "N", mgd_rows(res));
		add_property_resource(return_value, "__res__", res_id);
	}
}

void php_midgard_sitegroup_get(MidgardClass *species, zval * return_value,
               int grouped, const char *fields, const char *table, int id);
void php_midgard_get(MidgardClass *species, zval * return_value,
		     const char *fields, const char *table, int id)
{
	php_midgard_sitegroup_get(species, return_value, 1, fields, table, id);
}

#if HAVE_MIDGARD_MULTILANG
void php_midgard_sitegroup_get_lang(MidgardClass *species, zval * return_value,
               int grouped, const char *fields, const char *table, int id, int lang);

void php_midgard_get_lang(MidgardClass *species, zval * return_value,
		     const char *fields, const char *table, int id, int lang)
{
  php_midgard_sitegroup_get_lang(species, return_value, 1, fields, table, id, lang);
}
#endif /* HAVE_MIDGARD_MULTILANG */

void php_midgard_sitegroup_get(MidgardClass *species, zval * return_value,
               int grouped, const char *fields, const char *table, int id)
{
#if ! HAVE_MIDGARD_MULTILANG
	midgard_res *res;
#else /* HAVE_MIDGARD_MULTILANG */
  php_midgard_sitegroup_get_lang(species, return_value, grouped, fields, table, id, 0);
}

void php_midgard_sitegroup_get_lang(MidgardClass *species, zval * return_value,
				    int grouped, const char *fields, const char *table, int id, int lang) {
#endif /* HAVE_MIDGARD_MULTILANG */
	int i = 0;
#if HAVE_MIDGARD_MULTILANG
	midgard_res *res = NULL;
#endif /* HAVE_MIDGARD_MULTILANG */
	midgard_res *params = NULL;
	midgard_pool *pool = NULL;
	char *propname, *value=NULL;
	TSRMLS_FETCH();

	if (grouped) {
		
		if (lang) {
			res = mgd_sitegroup_record_lang(mgd_handle(), fields, table, id);
		} else {
			res = mgd_sitegroup_record(mgd_handle(), fields, table, id);
		}
		
	} else {
		res = mgd_ungrouped_record(mgd_handle(), fields, table, id);
	}
	
	if (!res || !mgd_fetch(res)) {
		if(res) mgd_release(res);
		RETVAL_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	} else {
	
		php_midgard_bless_oop(return_value, species);

		//mgd_fetch(res);
		for (i = 0; i < mgd_cols(res); i++) {
			
			value = (char *)mgd_colvalue(res, i);
			if (value == NULL ) {
				value = "";
			}

			char *value_end;
			long long_value = strtol(value, &value_end, 10);
			if (value_end != value && *value_end == '\0' &&
					(!g_str_equal((char*)mgd_colname(res, i), "name"))) {
				add_property_long(return_value, 
						(char*)mgd_colname(res, i), long_value);
			} else {
				add_property_string(return_value, 
						(char*)mgd_colname(res, i), (char *)value, 1);
			}
		}		

		if (res) mgd_release(res);

		int paramlang = mgd_lang(mgd_handle());
		gchar *order = NULL;
		if(paramlang > 0)  order = "lang ASC";
		
		params =
			mgd_ungrouped_select(mgd_handle(), "domain,name,value",
					"record_extension",
					"tablename=$q AND oid=$d and lang in ($d, $d)", order, table,
					id, 
					mgd_get_default_lang(mgd_handle()),
					paramlang);
		if (params) {
			pool = mgd_alloc_pool();
			while (mgd_fetch(params)) {
				propname =
					mgd_format(mgd_handle(), pool, "$s_$s",
							mgd_colvalue(params, 0),
							mgd_colvalue(params, 1));
				add_property_string(return_value, propname,
						    (char*)mgd_colvalue (params, 2), 1);	
			}
			mgd_release(params);
		}
	}
	if (pool) mgd_free_pool(pool);
}

MGD_FUNCTION(ret_type, oop_guid_get, (type param))
{
	zval *self;
	zval **zv_table, **zv_id;
	midgard_pool *pool;
	char *guid;

	CHECK_MGD;

	self = getThis();
	if (self == NULL) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
	}

    if (!MGD_PROPFIND(self, "__table__", zv_table)) {
        RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
    }

    if (!MGD_PROPFIND(self, "id", zv_id)) {
        RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
    }
    
	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(zv_id);
	convert_to_string_ex(zv_table);

	pool = mgd_pool(mgd_handle());
	guid =
	   mgd_repligard_guid(mgd_handle(), pool, (*zv_table)->value.str.val,
			      (*zv_id)->value.lval);
	if (guid) {
		RETVAL_STRING(guid, 1);
		mgd_free_from_pool(pool, guid);
	}
	else {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}
}

MGD_FUNCTION(ret_type, oop_parent_get, (type param))
{
  zval *self;
  zval **zv_table, **zv_id;
  long table, id;
  char *upfield;
  
  CHECK_MGD;

  self = getThis();
  if (NULL == self) {
    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
  }

  if (!MGD_PROPFIND(self, "__table__", zv_table)) {
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);              
  }  
  
  if (ZEND_NUM_ARGS() != 0) {
    WRONG_PARAM_COUNT;
  }

  convert_to_string_ex(zv_table);

  table = mgd_lookup_table_id((*zv_table)->value.str.val);

  switch(table) {
    case MIDGARD_OBJECT_ARTICLE:
    case MIDGARD_OBJECT_PAGE:
    case MIDGARD_OBJECT_SNIPPET:
    case MIDGARD_OBJECT_SNIPPETDIR:
    case MIDGARD_OBJECT_STYLE:
    case MIDGARD_OBJECT_TOPIC:
      upfield = "up";
      break;
    case MIDGARD_OBJECT_ELEMENT:
      upfield = "style";
      break;
    case MIDGARD_OBJECT_PAGEELEMENT:
      upfield = "page";
    default:
      RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
      break;
  }
 
  if (!MGD_PROPFIND(self, upfield, zv_id)) {
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
  }
  
  convert_to_long_ex(zv_id);

  id = (*zv_id)->value.lval;
  php_midgard_get_object(return_value, table, id);
}


void php_midgard_get_object(zval *return_value, int table, int id)
{
#define MGD_GET_OBJECT_SITEGROUP_FIELD ",guid,sitegroup"

   switch (table) {

      case MIDGARD_OBJECT_BLOBS:
         php_midgard_get(&MidgardAttachment, return_value,
            "id,name,title,mimetype,score,author,created,ptable,pid,location"
		         MGD_GET_OBJECT_SITEGROUP_FIELD,
			   "blobs", id);
         break;

      case MIDGARD_OBJECT_SITEGROUP:
         php_midgard_sitegroup_get(&midgardsitegroup, return_value, 0, "*",
            "sitegroup", id);
         break;

      default:
         RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
   }
}

MidgardClassPtr MidgardClasses [] = {
	&MidgardAttachment,
	&midgardsitegroup,
	NULL
};
