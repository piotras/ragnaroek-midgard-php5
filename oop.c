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
      fprintf(stderr,
         "SEVERE: No PHP class entry pointer for Midgard class '%s'\n",
            species->name);
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
      fprintf(stderr,
         "SEVERE: No PHP class entry pointer for Midgard class '%s'\n",
            species->name);
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

   /* EEH: TODO: log_error(
      "Midgard: delete of %s %d failed",
         table ? table : "<null class>", id);
   */
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
      /* EEH: TODO: log_error(
         "Midgard: update of %s %d failed",
            table ? table : "<null class>", id);
      */
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

MGD_FUNCTION(ret_type, oop_list_fetch, (type param))
{
	zval *self;
	zval **key;
	midgard_res *res;
	char *name, *value;
   int i;

	CHECK_MGD;
    RETVAL_FALSE;

	self = getThis();
	if (self == NULL) {
        RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
    }

    if (!MGD_PROPFIND(self, "__res__", key)) {
        RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
    }

    
   /* PP: Just commented now. Remove if FETCH macro' stability is proven 
    * res = (midgard_res*) zend_fetch_resource(key, -1,
    *  "midgard list fetch", NULL, 1, le_midgard_list_fetch); 
    */ 
  
   ZEND_FETCH_RESOURCE(res, midgard_res *, key, -1, "midgard list fetch" , le_midgard_list_fetch); 
  
   if (res == NULL) {
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
   }

   if (mgd_fetch(res)) {
      for (i = 0; i < mgd_cols(res); i++) {
         name = (char*)mgd_colname(res, i);
			value = (char *)mgd_colvalue(res, i);
			if (value == NULL ) {
				value = "";
			}

         char *value_end;
         long long_value = strtol(value, &value_end, 10);
         if (value_end != value && *value_end == '\0' &&
			 (!g_str_equal((char*)mgd_colname(res, i), "name"))) {
            add_property_long(self, name, long_value);
         } else {
			   add_property_string(self, name, (char *)value, 1);
         }
		}
      RETVAL_TRUE;
   } else {
       /* EEH: this should take care of destruction automatically */
       /* PP: list_delete commented as we must follow ZEND RESOURCES */
       //zend_list_delete((*key)->value.lval);
       zend_hash_del(Z_OBJPROP_P(self), "__res__", 8);
       
       RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
   }
}

#ifdef PHP_MIDGARD_LEGACY_API
void php_midgard_get_object_all_langs(zval *return_value, int table, int id) { 

   switch (table) {
   case MIDGARD_OBJECT_ARTICLE:
     php_midgard_select(&MidgardArticle, return_value, ARTICLE_SELECT_FAST "," CALENDAR_FIELDS, "article, article_i", "article.id=article_i.sid AND article_i.sid=$d", NULL, id);
     break;
   case MIDGARD_OBJECT_ELEMENT:
     php_midgard_select(&MidgardElement, return_value, "element.guid AS guid,element.id AS id,style,name,value,lang,element.sitegroup as sitegroup", "element, element_i", "element.id=element_i.sid AND element_i.sid=$d", NULL, id);
     break;
   case MIDGARD_OBJECT_PAGE:
     php_midgard_select(&MidgardPage, return_value, "page.guid AS guid,page.id AS id,up,lang,name,style,title,content,page.changed,page.author AS author,page.owner AS owner, page_i.author AS contentauthor, page_i.owner AS contentowner, info&1=1 AS auth,info&2=2 AS active, page.sitegroup as sitegroup",
			"page, page_i", "page.id=page_i.sid AND page_i.sid=$d", NULL, id);
     break;

   case MIDGARD_OBJECT_PAGEELEMENT:
     php_midgard_select(&MidgardPageElement, return_value, "pageelement.guid AS guid,pageelement.id AS id,page,name,value,lang,info&1 AS inherit, pageelement.sitegroup as sitegroup",
			"pageelement, pageelement_i", "pageelement.id=pageelement_i.sid AND pageelement_i.sid=$d", NULL, id);
     break;

   case MIDGARD_OBJECT_SNIPPET:
     php_midgard_select(&MidgardSnippet, return_value, "snippet.guid AS guid,snippet.id AS id,up,name,code,doc,author,creator,created,revisor,revised,revision,lang,snippet.sitegroup as sitegroup",
			  "snippet, snippet_i", "snippet.id=snippet_i.sid AND snippet_i.sid=$d", NULL, id);
         break;
   default:
     RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
   }  
}
#endif /* PHP_MIDGARD_LEGACY_API */

void php_midgard_get_object(zval *return_value, int table, int id)
{
#define MGD_GET_OBJECT_SITEGROUP_FIELD ",guid,sitegroup"

   switch (table) {
#ifdef PHP_MIDGARD_LEGACY_API
      case MIDGARD_OBJECT_ARTICLE:
#if ! HAVE_MIDGARD_MULTILANG
         php_midgard_get(&MidgardArticle, return_value,
            "id,up,topic,name,title,abstract,content,author,"
               "Date_format(created,'%d.%m.%Y') AS date,"
               "Date_format(created,'%D %b. %Y') AS adate,"
               "Date_format(created,'%D %M %Y') AS aldate,"
               "extra1,extra2,extra3,article.score,type,"
               "Unix_Timestamp(created) AS created,creator,"
               "Unix_Timestamp(revised) AS revised,revisor,revision,"
               "Unix_Timestamp(approved) AS approved,approver,"
               "Unix_Timestamp(locked) AS locked,locker,"
               "url,icon,view,print,"
               CALENDAR_FIELDS
		         MGD_GET_OBJECT_SITEGROUP_FIELD,
            "article", id);
#else /* HAVE_MIDGARD_MULTILANG */
         php_midgard_get_lang(&MidgardArticle, return_value,
			      ARTICLE_SELECT_FAST "," CALENDAR_FIELDS,
			 "article", id, 1);
#endif /* HAVE_MIDGARD_MULTILANG */
         break;
#endif /* PHP_MIDGARD_LEGACY_API */
      case MIDGARD_OBJECT_BLOBS:
         php_midgard_get(&MidgardAttachment, return_value,
            "id,name,title,mimetype,score,author,created,ptable,pid,location"
		         MGD_GET_OBJECT_SITEGROUP_FIELD,
			   "blobs", id);
         break;
#ifdef PHP_MIDGARD_LEGACY_API
      case MIDGARD_OBJECT_ELEMENT:
#if ! HAVE_MIDGARD_MULTILANG
         php_midgard_get(&MidgardElement, return_value,
            "id,style,name,value" MGD_GET_OBJECT_SITEGROUP_FIELD,
            "element", id);
#else /* HAVE_MIDGARD_MULTILANG */
	php_midgard_get_lang(&MidgardElement, return_value,
			     "element.guid AS guid,element.id AS id,style,lang,name,value,element.sitegroup as sitegroup",
            "element", id, 1);
#endif /* HAVE_MIDGARD_MULTILANG */
         break;

      case MIDGARD_OBJECT_EVENT:
         php_midgard_get(&MidgardEvent, return_value,
            "id,up,start,end,title,description,"
               "type,extra,owner,creator,created,revisor,revised,revision,busy,"
               "Unix_Timestamp(locked) AS locked,locker"
		         MGD_GET_OBJECT_SITEGROUP_FIELD,
            "event", id);
         break;

      case MIDGARD_OBJECT_EVENTMEMBER:
         php_midgard_get(&MidgardEventMember, return_value, "id,eid,uid,extra"
		         MGD_GET_OBJECT_SITEGROUP_FIELD,
            "eventmember", id);
         break;


      case MIDGARD_OBJECT_GRP:
         php_midgard_get(&MidgardGroup, return_value, 
            "id,name,official," ADDRESS_FIELDS ","
               GROUP_HOMEPAGE_FIELDS "," GROUP_EMAIL_FIELDS ","
					"extra,owner"
		         MGD_GET_OBJECT_SITEGROUP_FIELD,
            "grp", id);
         break;

      case MIDGARD_OBJECT_HOST:
         php_midgard_get(&MidgardHost, return_value,
#if ! HAVE_MIDGARD_MULTILANG
				"id,name,port,online,root,style,info&1 AS auth,owner,prefix,"
#else /* HAVE_MIDGARD_MULTILANG */
				"id,name,port,online,root,style,info&1 AS auth,owner,prefix,lang,"
#endif /* HAVE_MIDGARD_MULTILANG */
               HOSTNAME_FIELD
		         MGD_GET_OBJECT_SITEGROUP_FIELD,
				"host", id);
         break;

      case MIDGARD_OBJECT_MEMBER:
         php_midgard_get(&MidgardMember, return_value,
            "id,gid,uid,extra" MGD_GET_OBJECT_SITEGROUP_FIELD,
            "member", id);
         break;

      case MIDGARD_OBJECT_PAGE:
#if ! HAVE_MIDGARD_MULTILANG
         php_midgard_get(&MidgardPage, return_value,
            "id,up,name,style,title,changed,content,author,"
               "info&1=1 AS auth,info&2=2 AS active"
		         MGD_GET_OBJECT_SITEGROUP_FIELD,
            "page", id);
#else /* HAVE_MIDGARD_MULTILANG */ 
         php_midgard_get_lang(&MidgardPage, return_value,
            "page.guid AS guid, page.id AS id,up,lang,name,style,title,content,page.changed AS changed, page.author AS author,page.owner AS owner, page_i.author AS contentauthor, page_i.owner AS contentowner, "
			      "info&1=1 AS auth,info&2=2 AS active, page.sitegroup as sitegroup",
            "page" , id, 1);
#endif /* HAVE_MIDGARD_MULTILANG */
         break;

      case MIDGARD_OBJECT_PAGEELEMENT:
#if ! HAVE_MIDGARD_MULTILANG
         php_midgard_get(&MidgardPageElement, return_value,
            "id,page,name,value,info&1 AS inherit"
		         MGD_GET_OBJECT_SITEGROUP_FIELD,
            "pageelement", id);
#else /* HAVE_MIDGARD_MULTILANG */
         php_midgard_get_lang(&MidgardPageElement, return_value,
			      "pageelement.guid AS guid,pageelement.id AS id,page,lang,name,value,info&1 AS inherit, pageelement.sitegroup as sitegroup",
            "pageelement", id, 1);
#endif /* HAVE_MIDGARD_MULTILANG */
         break;

#if HAVE_MIDGARD_PAGELINKS
      case MIDGARD_OBJECT_PAGELINK:
         php_midgard_get(&MidgardPagelink, return_value,
            "id,up,name,target,grp,owner"
		         MGD_GET_OBJECT_SITEGROUP_FIELD,
            "pagelink", id);
         break;
#endif

      case MIDGARD_OBJECT_PERSON:
         if (isuserowner(id))
            php_midgard_get(&MidgardPerson, return_value,
               "id," NAME_FIELDS ","
                  "If(Left(password,2)='**',Substring(password,3),'')"
                  " AS password,"
                  ADDRESS_FIELDS "," PHONE_FIELDS ","
                  HOMEPAGE_FIELDS "," EMAIL_FIELDS ","
                  "Date_format(birthdate,'%d.%m.%Y') AS birthdate,extra,"
                  "Date_format(created,'%d.%m.%Y') AS created,creator,"
                  "img,topic,subtopic,department,office,pgpkey," PUBLIC_FIELDS
		            MGD_GET_OBJECT_SITEGROUP_FIELD,
               "person", id);
         else
            php_midgard_get(&MidgardPerson, return_value,
               "id," NAME_FIELDS ",'' AS password,"
                  "If(info&2," ADDRESS_FIELD ",'') AS address,"
                  PUBLIC_FIELD(2,street) ","
                  PUBLIC_FIELD(2,postcode) ","
                  PUBLIC_FIELD(2,city) ","
                  "If(info&4," PHONE_FIELD ",'') AS phone,"
                  PUBLIC_FIELD(4,handphone) ","
                  PUBLIC_FIELD(4,homephone) ","
                  PUBLIC_FIELD(4,workphone) ","
                  "If(info&8," HOMEPAGE_FIELD ",'') AS homepagelink,"
                  PUBLIC_FIELD(8,homepage) ","
                  "If(info&16," EMAIL_FIELD ",'') AS emaillink,"
                  PUBLIC_FIELD(16,email) ","
                  /* [eeh] birthdate does not have a privacy setting */
                  "'' AS birthdate," PUBLIC_FIELD(32,extra) ","
                  "img,topic,subtopic,department,office,pgpkey,"
                  "Date_format(created,'%d.%m.%Y') AS created,creator,"
                  PUBLIC_FIELDS
		            MGD_GET_OBJECT_SITEGROUP_FIELD,
               "person", id);
         break;

#endif /* PHP_MIDGARD_LEGACY_API */
      case MIDGARD_OBJECT_SITEGROUP:
         php_midgard_sitegroup_get(&midgardsitegroup, return_value, 0, "*",
            "sitegroup", id);
         break;
#ifdef PHP_MIDGARD_LEGACY_API
#if HAVE_MIDGARD_QUOTA
      case MIDGARD_OBJECT_QUOTA:
         php_midgard_get(&MidgardQuota, return_value, "*",
            "quota", id);
         break;
#endif

      case MIDGARD_OBJECT_SNIPPET:
#if ! HAVE_MIDGARD_MULTILANG
         php_midgard_get(&MidgardSnippet, return_value,
            "id,up,name,code,doc,author,creator,created,revisor,"
               "revised,revision" MGD_GET_OBJECT_SITEGROUP_FIELD,
            "snippet", id);
#else /* HAVE_MIDGARD_MULTILANG */
         php_midgard_get_lang(&MidgardSnippet, return_value,
            "snippet.id AS id,up,name,lang,code,doc,author,creator,created,revisor,"
               "revised,revision, snippet.sitegroup as sitegroup",
            "snippet", id, 1);
#endif /* HAVE_MIDGARD_MULTILANG */
         break;

      case MIDGARD_OBJECT_SNIPPETDIR:
         php_midgard_get(&MidgardSnippetdir, return_value, "id,up,name,description,owner"
               MGD_GET_OBJECT_SITEGROUP_FIELD,
            "snippetdir", id);
         break;

      case MIDGARD_OBJECT_STYLE:
         php_midgard_get(&MidgardStyle, return_value, "id,up,name,owner"
               MGD_GET_OBJECT_SITEGROUP_FIELD,
            "style", id);
         break;

      case MIDGARD_OBJECT_TOPIC:
	 php_midgard_get_lang(&MidgardTopic, return_value,
			 "topic.guid AS guid, topic.id AS id, up,score,name,"
			 "description,extra,owner,code,"
			 "creator,Unix_timestamp(created) as created,"
			 "revisor,Unix_timestamp(revised) as revised,revision,"
			 "topic.sitegroup AS sitegroup",
			 "topic", id, 1);
	 break;
#if HAVE_MIDGARD_MULTILANG
      case MIDGARD_OBJECT_LANGUAGE:
	php_midgard_get(&MidgardLanguage, return_value,
            "id,code,name" MGD_GET_OBJECT_SITEGROUP_FIELD,
            "language", id);
         break;
#endif /* HAVE_MIDGARD_MULTILANG */

      /* EEH: These are not handled */
      case MIDGARD_OBJECT_REPLIGARD:
      case MIDGARD_OBJECT_RECORD_EXTENSION:
      case MIDGARD_OBJECT_HISTORY:
#endif /* PHP_MIDGARD_LEGACY_API */
      default:
         RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
   }
}

#ifdef  PHP_MIDGARD_LEGACY_API 
MGD_FUNCTION(ret_type, get_object_by_guid, (type param))
{
	zval **guid;
	midgard_res *res;
	gchar *tablename;
	guint table;
	midgard *mgd = mgd_handle();
	MgdObject *mobj;
	zend_class_entry *ce;
	
	php_error(E_NOTICE, "mgd_get_object_by_guid is deprecated. Use midgard_object_class::get_object_by_guid() instead");
	
	if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &guid) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}
	
	if ((*guid)->type != IS_STRING) 
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);  

	GString *sql = g_string_new("SELECT realm FROM repligard ");
	g_string_append_printf(sql, 
			"WHERE guid = '%s' and action NOT LIKE 'delete'",
			(*guid)->value.str.val);

	gchar *tmpstr = g_string_free(sql, FALSE);
	res = mgd_query(mgd_handle(), tmpstr);
	g_free(tmpstr);

	if (!res || !mgd_fetch(res)) {
		if (res) mgd_release(res);
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}
	
	tablename = g_strdup(mgd_colvalue(res, 0));
	mgd_release(res);

	table = mgd_lookup_table_id(tablename);
		
	if (table != 0) {
		
		res = mgd_sitegroup_select(mgd_handle(), "id" , (const gchar*)tablename, 
				"guid=$q", NULL, (*guid)->value.str.val);
		if (!res || !mgd_fetch(res)) {
			if (res) mgd_release(res);
			g_free(tablename);
			RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
		}
		guint id = atoi(mgd_colvalue(res, 0));
		mgd_release(res);
		php_midgard_get_object(return_value, table, id);
	
	} else {

		mobj = midgard_object_class_get_object_by_guid(mgd->_mgd, Z_STRVAL_PP(guid));
		if (mobj) {
			if ((zend_hash_find(CG(class_table),
							tablename,
							strlen(tablename)+1,
							(void **) &ce)) == SUCCESS ){
				php_midgard_gobject_new_with_gobject(return_value, 
						ce, G_OBJECT(mobj), TRUE);
			}
		}
	}					      

	g_free(tablename);
}

#if HAVE_MIDGARD_MULTILANG
MGD_FUNCTION(ret_type, get_object_by_guid_all_langs, (type param))
{
   zval **guid;
   midgard_res *res;
   long table, id;

   if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &guid) != SUCCESS) {
      WRONG_PARAM_COUNT;
   }

   if ((*guid)->type != IS_STRING) { RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS); }
   
   res = mgd_sitegroup_select(mgd_handle(), "realm,id", "repligard",
            "guid=$q", NULL, (*guid)->value.str.val);

   if (!res || !mgd_fetch(res)) {
      if (res) mgd_release(res);
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
   }

   table = mgd_lookup_table_id(mgd_colvalue(res, 0));
   id = atoi(mgd_colvalue(res, 1));

   mgd_release(res);

   php_midgard_get_object_all_langs(return_value, table, id);
}

MGD_FUNCTION(ret_type, is_owner_by_guid, (type param))
{

  midgard_res *res;
  char * strTable;
  long table, id;
  int content = 0;
  zval **guid;
  if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &guid) != SUCCESS) {
    WRONG_PARAM_COUNT;
  }

  if (mgd_isadmin(mgd_handle()))
    RETURN_TRUE;

  res = mgd_sitegroup_select(mgd_handle(), "realm,id", "repligard",
			     "guid=$q", NULL, (*guid)->value.str.val);
  
  if (!res || !mgd_fetch(res)) {
    if (res) mgd_release(res);
    RETURN_FALSE;
   }
  
  strTable = (char * )mgd_colvalue(res, 0);
  if (strstr(strTable, "_i") != NULL) {
    content = 2;
    strTable[strlen(strTable) -2] = 0;
  }
  table = mgd_lookup_table_id(strTable);
  id = atoi(mgd_colvalue(res, 1));
  
  mgd_release(res);
  
  if (mgd_global_is_owner_lang  (mgd_handle(), table, id, content, mgd_lang(mgd_handle()))) {
    RETURN_TRUE;
  } else {
    RETURN_FALSE;
  }
  
}

#endif /* HAVE_MIDGARD_MULTILANG */
MidgardClassPtr MidgardClasses [] = {
   &MidgardArticle,
   &MidgardAttachment,
   &MidgardElement,
   &MidgardEvent,
   &MidgardEventMember,
   &MidgardGroup,
   &MidgardHost,
   &MidgardMember,
   &MidgardPage,
   &MidgardPageElement,
#if HAVE_MIDGARD_PAGELINKS
   &MidgardPagelink,
#endif
   &MidgardParameter,
   &MidgardPerson,
   &midgardsitegroup,
   &MidgardSnippet,
   &MidgardSnippetdir,
   &MidgardStyle,
   &MidgardTopic,
#if HAVE_MIDGARD_MULTILANG
   &MidgardLanguage,
#endif /* HAVE_MIDGARD_MULTILANG */
#if HAVE_MIDGARD_QUOTA
   &MidgardQuota,
#endif /* HAVE_MIDGARD_QUOTA */
   NULL
};

#else /* LEGACY API */

MidgardClassPtr MidgardClasses [] = {
	&MidgardAttachment,
	&midgardsitegroup,
	NULL
};

#endif /* LEGACY API */
