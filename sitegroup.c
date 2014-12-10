/* $Id: sitegroup.c 27410 2014-09-01 07:39:28Z piotras $
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
#include "mgd_oop.h"

//#ifdef PHP_MIDGARD_LEGACY_API

MGD_FUNCTION(ret_type, has_sitegroups, (type param))
{
	RETURN_TRUE;
}

MGD_FUNCTION(ret_type, list_sitegroups, (type param))
{
  php_midgard_select(&midgardsitegroup, return_value, "*", "sitegroup", NULL, "name");
}

MGD_FUNCTION(ret_type, create_sitegroup, (type param))
{
	zval *zv_name, *self;

	RETVAL_FALSE;
	CHECK_MGD;

	if (!mgd_isroot(mgd_handle()))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "name", zv_name)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if ((ZEND_NUM_ARGS() != 1) || (zend_parse_parameters(1 TSRMLS_CC, "z", &zv_name) != SUCCESS)) 
			WRONG_PARAM_COUNT;
	}

	convert_to_string_ex(&zv_name);

	/* no 'magic' chars in sitegroup name */
	if (strpbrk(Z_STRVAL_P(zv_name), MIDGARD_LOGIN_RESERVED_CHARS))
		RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_NAME);

	if (mgd_exists_bool(mgd_handle(), "sitegroup", "name=$q", Z_STRVAL_P(zv_name)))
		RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

	php_midgard_create(return_value, self, "sitegroup",
			   "name,admingroup", "$q,$d", Z_STRVAL_P(zv_name),
			   0);

	PHP_CREATE_REPLIGARD("sitegroup", Z_LVAL_P(return_value));
}

MGD_FUNCTION(ret_type, get_sitegroup, (type param))
{
	zval *id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	case 0:
		php_midgard_bless(return_value, &midgardsitegroup);
		mgd_object_init(return_value, "name", "admingroup", "realm", NULL);
		return;

	case 1:
		if (zend_parse_parameters(1 TSRMLS_CC, "z", &id) == FAILURE) {
		   WRONG_PARAM_COUNT;
		}

		convert_to_long_ex(&id);
		break;

	default:
		WRONG_PARAM_COUNT;
	}

   	php_midgard_get_object(return_value, MIDGARD_OBJECT_SITEGROUP, Z_LVAL_P(id));
}

MGD_FUNCTION(ret_type, update_sitegroup, (type param))
{
	zval *zv_id, *zv_name, *zv_admingroup, *zv_realm, *self;
	int group;
	int id, admingroup;

	RETVAL_FALSE;
	CHECK_MGD;

	if (!mgd_isroot(mgd_handle()))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "id", zv_id)
		    || !MGD_PROPFIND(self, "name", zv_name)
		    || !MGD_PROPFIND(self, "admingroup", zv_admingroup)
		    || !MGD_PROPFIND(self, "realm", zv_realm)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if ((ZEND_NUM_ARGS() != 4)
		    ||
		    (zend_parse_parameters
		     (4 TSRMLS_CC, "zzzz", &zv_id, &zv_name, &zv_admingroup,
		      &zv_realm) != SUCCESS)) WRONG_PARAM_COUNT;
	}

	convert_to_string_ex(&zv_name);
	convert_to_string_ex(&zv_realm);
	convert_to_long_ex(&zv_admingroup);
	convert_to_long_ex(&zv_id);

	id = Z_LVAL_P(zv_id);
	admingroup = Z_LVAL_P(zv_admingroup);

	if (id == 0)
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	group = mgd_exists_id(mgd_handle(), "sitegroup", "name=$q", Z_STRVAL_P(zv_name));
	if (group != 0 && group != id)
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	/* check admingroup == sitegroup || admingroup == 0 */
	if (admingroup != 0
	    && !mgd_exists_bool(mgd_handle(), "grp", "id=$d and sitegroup=$d",
				admingroup, id)
	   )
		RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);

	php_midgard_update(return_value,
			   "sitegroup",
			   "name=$q, admingroup=$d, realm=$q",
			   id,
			   Z_STRVAL_P(zv_name),
			   admingroup, Z_STRVAL_P(zv_realm));
	PHP_UPDATE_REPLIGARD("sitegroup", id);
}

MGD_FUNCTION(ret_type, delete_sitegroup, (type param))
{
  IDINIT;
  CHECK_MGD;

  if (!mgd_isroot(mgd_handle())) RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

  if (mgd_global_exists(mgd_handle(), "sitegroup=$d", id)) RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

  php_midgard_delete(return_value, "sitegroup", id);
  PHP_DELETE_REPLIGARD("sitegroup", id);
}

MGD_FUNCTION(ret_type, oop_sitegroup_get, (type param))
{
zval *self = NULL;
zval **pv_table, **pv_id;
int sitegroup;

   CHECK_MGD;

   self=getThis();
   if (self == NULL) {
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
   }

   if (!MGD_PROPFIND(self, "__table__", pv_table)) {
       RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
   }

   if (!MGD_PROPFIND(self, "id", pv_id)) {
       RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
   }
     
   convert_to_string_ex(pv_table);
   convert_to_long_ex(pv_id);

   if (strcmp((*pv_table)->value.str.val, "sitegroup") == 0) {
      RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }

   if (ZEND_NUM_ARGS() != 0) { WRONG_PARAM_COUNT; }

   sitegroup = mgd_idfield(mgd_handle(), "sitegroup",
      (*pv_table)->value.str.val,(*pv_id)->value.lval);

   php_midgard_sitegroup_get(&midgardsitegroup, return_value,
      0, "*", "sitegroup", sitegroup);
}

MGD_FUNCTION(ret_type, oop_sitegroup_set, (type param))
{
   zval *self;
   zval *zv_sitegroup;
   zval **zv_table, **zv_id;
   char table_i[256];
   midgard_res *res;
   int i_id;

   CHECK_MGD;

	if (!mgd_isroot(mgd_handle())) {
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }

	self = getThis();
	if (self == NULL) {
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
   }

   if (ZEND_NUM_ARGS() != 1) { WRONG_PARAM_COUNT; }

    if (!MGD_PROPFIND(self, "__table__", zv_table)) {
        RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
    }
  
    if (!MGD_PROPFIND(self, "id", zv_id)) {
        RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
    }

   convert_to_string_ex(zv_table);
   convert_to_long_ex(zv_id);

   if (strcmp((*zv_table)->value.str.val, "sitegroup") == 0) {
      RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }

   if (zend_parse_parameters(1 TSRMLS_CC, "z", &zv_sitegroup) == FAILURE) {
      WRONG_PARAM_COUNT;
   }

   convert_to_long_ex(&zv_sitegroup);

   if (Z_LVAL_P(zv_sitegroup) != 0
         && !mgd_exists_bool(mgd_handle(), "sitegroup", "id=$d", Z_LVAL_P(zv_sitegroup))) {
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
   }

   add_property_long(getThis(), "sitegroup", Z_LVAL_P(zv_sitegroup));
   
   php_midgard_update(return_value,
      (*zv_table)->value.str.val,
      "sitegroup=$d", (*zv_id)->value.lval,
      Z_LVAL_P(zv_sitegroup));
   mgd_update_repligard(mgd_handle(), (*zv_table)->value.str.val, (*zv_id)->value.lval,
      "sitegroup=$d", Z_LVAL_P(zv_sitegroup));

   TOUCH_CACHE;
   if (strcmp((*zv_table)->value.str.val, "article") == 0 || strcmp((*zv_table)->value.str.val, "element") == 0 || strcmp((*zv_table)->value.str.val, "page") == 0 || 
   strcmp((*zv_table)->value.str.val, "pageelement") == 0 || strcmp((*zv_table)->value.str.val, "snippet") == 0) {
     if(strlen((*zv_table)->value.str.val) < 253) {
       strcpy(table_i, (*zv_table)->value.str.val);
       strcat(table_i, "_i");
       res = mgd_ungrouped_select(mgd_handle(), "id", table_i, "sid=$d", NULL, (*zv_id)->value.lval);
	while (res && mgd_fetch(res)) {
	  i_id = atol(mgd_colvalue(res, 0));
	  php_midgard_update(return_value,
			     table_i,
			     "sitegroup=$d", i_id,
			     Z_LVAL_P(zv_sitegroup));
	  mgd_update_repligard(mgd_handle(), table_i, i_id,
			       "sitegroup=$d", Z_LVAL_P(zv_sitegroup));
}
     }
}
   TOUCH_CACHE;

}

static zend_function_entry midgardsitegroupMethods[] = {
   PHP_FALIAS(midgardsitegroup,  mgd_get_sitegroup,	   NULL)
   PHP_FALIAS(create,	         mgd_create_sitegroup,	NULL)
   PHP_FALIAS(update,	         mgd_update_sitegroup,	NULL)
   PHP_FALIAS(delete,	         mgd_delete_sitegroup,	NULL)
#ifdef PHP_MIDGARD_LEGACY_API
   PHP_FALIAS(fetch,	            mgd_oop_list_fetch,		NULL)
   PHP_FALIAS(guid,		         mgd_oop_guid_get,		   NULL)
   MIDGARD_OOP_PARAMETER_METHODS
   MIDGARD_OOP_ATTACHMENT_METHODS
#endif
   {  NULL,             NULL,                         NULL}
};

MidgardClass midgardsitegroup = {
   "midgardsitegroup",
   "sitegroup",
   midgardsitegroupMethods,
   {},
   NULL
};

/*
MIDGARD_CLASS(midgardsitegroup, sitegroup, midgardsitegroup, sitegroup)
   * MidgardClass midgardsitegroup = {
   "midgardsitegroup",
   "sitegroup",
   midgardsitegroupMethods,
   {},
   NULL
   };
*/
//#endif
