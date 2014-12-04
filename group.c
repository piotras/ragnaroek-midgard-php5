/* $Id: group.c 10324 2006-11-27 12:47:04Z piotras $
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

#ifdef PHP_MIDGARD_LEGACY_API

MGD_FUNCTION(ret_type, is_group_owner, (type param))
{
    IDINIT;
	CHECK_MGD;
    RETVAL_LONG(isgroupowner(id));
}

MGD_FUNCTION(ret_type, list_groups, (type param))
{
	zval **id;

	RETVAL_FALSE;
	CHECK_MGD;
	switch (ZEND_NUM_ARGS()) {
	case 0:
		php_midgard_select(&MidgardGroup, return_value,
         "id,name,official" SITEGROUP_SELECT, "grp", NULL, "name"); 
		return;

	case 1:
		if (zend_get_parameters_ex(1, &id) == FAILURE) {
		   WRONG_PARAM_COUNT;
		}

		convert_to_long_ex(id);
		break;

	default:
		WRONG_PARAM_COUNT;
	}
	if((*id)->value.lval && !mgd_exists_id(mgd_handle(), "grp", "id=$d", (*id)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	php_midgard_select(&MidgardGroup, return_value, "id,name,official" SITEGROUP_SELECT, "grp", "owner=$d", "name",(*id)->value.lval); 
}

MGD_FUNCTION(ret_type, get_group, (type param))
{
	zval **id;
	CHECK_MGD;

	RETVAL_FALSE;
	switch (ZEND_NUM_ARGS()) {
	case 0:
      php_midgard_bless(return_value, &MidgardGroup);
      mgd_object_init(return_value, "name", "official", "street", "postcode", "city", "homepage", "email", "extra", "owner", NULL);
		return;

	case 1:
		if (zend_get_parameters_ex(1, &id) == FAILURE) {
		   WRONG_PARAM_COUNT;
		}

		convert_to_long_ex(id);
		break;

	default:
		WRONG_PARAM_COUNT;
	}

   php_midgard_get_object(return_value, MIDGARD_OBJECT_GRP, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, get_group_by_name, (type param))
{
	zval **id, **name;
	int gid=0;
	CHECK_MGD;

	RETVAL_FALSE;
	switch (ZEND_NUM_ARGS()) {
	case 0:
		php_midgard_bless(return_value, &MidgardGroup);
		mgd_object_init(return_value, "name", "official", "street",
		    "postcode", "city", "homepage", "email",
		    "extra", "owner", NULL);
		return;

	case 1:
		if (zend_get_parameters_ex(1, &name) == FAILURE) {
         WRONG_PARAM_COUNT;
      }

		convert_to_string_ex(name);
		gid=mgd_exists_id(mgd_handle(),"grp","name=$q",(*name)->value.str.val);
		break;

	case 2:
		if (zend_get_parameters_ex(2, &id, &name) == FAILURE) {
		   WRONG_PARAM_COUNT;
		}

		convert_to_long_ex(id);
		convert_to_string_ex(name);
		gid=mgd_exists_id(mgd_handle(),"grp","owner=$d AND name=$q",
		   (*id)->value.lval,
         (*name)->value.str.val);
		break;

	default:
		WRONG_PARAM_COUNT;
	}

   if (gid == 0) RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
   php_midgard_get_object(return_value, MIDGARD_OBJECT_GRP, gid);
}

MGD_FUNCTION(ret_type, create_group, (type param))
{
	zval **name, **official, **street, **postcode, **city;
	zval **homepage, **email, **extra, **owner;
	zval *self = NULL;

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "official", official)
		    || !MGD_PROPFIND(self, "street", street)
		    || !MGD_PROPFIND(self, "postcode", postcode)
		    || !MGD_PROPFIND(self, "city", city)
		    || !MGD_PROPFIND(self, "homepage", homepage)
		    || !MGD_PROPFIND(self, "email", email)
		    || !MGD_PROPFIND(self, "extra", extra)
		    || !MGD_PROPFIND(self, "owner", owner)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 9
		    || zend_get_parameters_ex(9, &name, &official, &street,
				     &postcode, &city, &homepage, &email,
				     &extra, &owner) != SUCCESS)
			WRONG_PARAM_COUNT;
	}

	convert_to_string_ex(name);
	convert_to_string_ex(official);
	convert_to_string_ex(street);
	convert_to_string_ex(postcode);
	convert_to_string_ex(city);
	convert_to_string_ex(homepage);
	convert_to_string_ex(email);
	convert_to_string_ex(extra);
	convert_to_long_ex(owner);


	if (!isgroupowner((*owner)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

   if ((*owner)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "grp", "id=$d", (*owner)->value.str.val))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

   if (mgd_exists_id(mgd_handle(), "grp", "name=$q", (*name)->value.str.val))
      RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

	php_midgard_create(return_value, self, "grp",
			   "name,official,street,postcode,city,"
			   "homepage,email,extra,owner",
			   "$q,$q,$q,$q,$q,$q,$q,$q,$d", (*name)->value.str.val,
			   (*official)->value.str.val, (*street)->value.str.val,
			   (*postcode)->value.str.val, (*city)->value.str.val,
			   (*homepage)->value.str.val, (*email)->value.str.val,
			   (*extra)->value.str.val, (*owner)->value.lval);

	PHP_CREATE_REPLIGARD("grp", return_value->value.lval);
}

MGD_FUNCTION(ret_type, update_group, (type param))
{
	zval **gid, **name, **official, **street, **postcode, **city;
	zval **homepage, **email, **extra, **owner;
	zval *self = NULL;

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "id", gid)
		    || !MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "official", official)
		    || !MGD_PROPFIND(self, "street", street)
		    || !MGD_PROPFIND(self, "postcode", postcode)
		    || !MGD_PROPFIND(self, "city", city)
		    || !MGD_PROPFIND(self, "homepage", homepage)
		    || !MGD_PROPFIND(self, "email", email)
		    || !MGD_PROPFIND(self, "extra", extra)
		    || !MGD_PROPFIND(self, "owner", owner)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 10
		    || zend_get_parameters_ex(10, &gid, &name, &official, &street,
				     &postcode, &city, &homepage, &email,
				     &extra, &owner) != SUCCESS)
			WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(gid);
	convert_to_string_ex(name);
	convert_to_string_ex(official);
	convert_to_string_ex(street);
	convert_to_string_ex(postcode);
	convert_to_string_ex(city);
	convert_to_string_ex(homepage);
	convert_to_string_ex(email);
	convert_to_string_ex(extra);
	convert_to_long_ex(owner);

	if (!isgroupowner((*gid)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

   if ((*owner)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "grp", "id=$d", (*owner)->value.str.val))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	php_midgard_update(return_value, "grp",
			   "name=$q,official=$q,street=$q,postcode=$q,city=$q,"
			   "homepage=$q,email=$q,extra=$q,owner=$d",
			   (*gid)->value.lval,
			   (*name)->value.str.val, (*official)->value.str.val,
			   (*street)->value.str.val, (*postcode)->value.str.val,
			   (*city)->value.str.val, (*homepage)->value.str.val,
			   (*email)->value.str.val, (*extra)->value.str.val,
			   (*owner)->value.lval);
	PHP_UPDATE_REPLIGARD("grp", (*gid)->value.lval);
}

MGD_FUNCTION(ret_type, delete_group, (type param))
{
    IDINIT;
	CHECK_MGD;
    if(mgd_has_dependants(mgd_handle(),id,"group"))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

    if(mgd_global_exists(mgd_handle(), "owner=$d", id)
	    || mgd_exists_id(mgd_handle(), "member", "gid=$d", id))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

    if (!isgroupowner(id))
	 RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    php_midgard_delete(return_value, "grp", id);
    PHP_DELETE_REPLIGARD("grp", id);
}

MGD_WALK_FUNCTION_EX(group, grp, owner)

static zend_function_entry MidgardGroupMethods[] =
   {
      PHP_FALIAS(MidgardGroup,   mgd_get_group,       NULL)
      PHP_FALIAS(create,         mgd_create_group,    NULL)
      PHP_FALIAS(update,         mgd_update_group,    NULL)
      PHP_FALIAS(delete,         mgd_delete_group,    NULL)
      PHP_FALIAS(fetch,          mgd_oop_list_fetch,  NULL)
      MIDGARD_OOP_ATTACHMENT_METHODS
      MIDGARD_OOP_SITEGROUP_METHODS
      MIDGARD_OOP_PARAMETER_METHODS
      {NULL, NULL, NULL}
   };
MidgardClass MidgardGroup = {
   "MidgardGroup",
   "grp",
   MidgardGroupMethods,
   {},
   NULL
};

#endif

