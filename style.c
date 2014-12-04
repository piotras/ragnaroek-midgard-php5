/* $Id: style.c 10324 2006-11-27 12:47:04Z piotras $
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
#include <midgard/pageresolve.h>

#ifdef PHP_MIDGARD_LEGACY_API

MGD_FUNCTION(ret_type, is_style_owner, (type param))
{
    IDINIT;
    CHECK_MGD;
    RETVAL_LONG(isstyleowner(id));
}

MGD_FUNCTION(ret_type, list_styles, (type param))
{
	zval **id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	case 0:
        php_midgard_select(&MidgardStyle, return_value, "id,name" SITEGROUP_SELECT, "style", NULL, "name");
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

    php_midgard_select(&MidgardStyle, return_value, "id,name" SITEGROUP_SELECT, "style", "up=$d", "name",
	                                                     (*id)->value.lval);
}

MGD_FUNCTION(ret_type, get_style, (type param))
{
	zval **id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	case 0:
		php_midgard_bless(return_value, &MidgardStyle);
		mgd_object_init(return_value, "up", "name", "owner", NULL);
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

   php_midgard_get_object(return_value, MIDGARD_OBJECT_STYLE, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, get_style_by_name, (type param))
{
   zval **style;
   int objid;

   CHECK_MGD;

   if (ZEND_NUM_ARGS() != 1
         || zend_get_parameters_ex(1, &style) != SUCCESS) {
      WRONG_PARAM_COUNT;
   }

   convert_to_string_ex(style);
   objid = mgd_exists_id(mgd_handle(), "style", "name=$q",
      (*style)->value.str.val);
   php_midgard_get_object(return_value, MIDGARD_OBJECT_STYLE, objid);
}

MGD_FUNCTION(ret_type, create_style, (type param))
{
	zval **up, **name, **owner, *self;

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "up", up)
		    || !MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "owner", owner)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 3
		    || zend_get_parameters_ex(3, &up, &name, &owner) == FAILURE)
			WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(up);
	convert_to_string_ex(name);
	convert_to_long_ex(owner);

	if (!isstyleowner((*up)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if ((*up)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "style", "id=$d", (*up)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if ((*owner)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "grp", "id=$d", (*owner)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	php_midgard_create(return_value, self, "style", "up,name,owner", "$d,$q,$d",
			   (*up)->value.lval, (*name)->value.str.val,
			   (*owner)->value.lval);

	PHP_CREATE_REPLIGARD("style", return_value->value.lval);

   TOUCH_CACHE;
}

MGD_FUNCTION(ret_type, update_style, (type param))
{
	zval **id, **name, **owner, *self;

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "id", id)
		    || !MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "owner", owner)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 3
		    || zend_get_parameters_ex(3, &id, &name, &owner) == FAILURE)
			WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(id);
	convert_to_string_ex(name);
	convert_to_long_ex(owner);

	if (!isstyleowner((*id)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if ((*owner)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "grp", "id=$d", (*owner)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	php_midgard_update(return_value, "style", "name=$q,owner=$d",
			   (*id)->value.lval, (*name)->value.str.val,
			   (*owner)->value.lval);
	PHP_UPDATE_REPLIGARD("style", (*id)->value.lval);

   TOUCH_CACHE;
}

MGD_FUNCTION(ret_type, delete_style, (type param))
{
    IDINIT;
	CHECK_MGD;
    if(mgd_has_dependants(mgd_handle(), id, "style"))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
    if(mgd_exists_id(mgd_handle(), "host", "style=$d", id))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
    if(mgd_exists_id(mgd_handle(), "page", "style=$d", id))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
    if(mgd_exists_id(mgd_handle(), "style", "up=$d", id))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
    if(mgd_exists_id(mgd_handle(), "element", "style=$d", id))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

    if (!isstyleowner(id)) RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    php_midgard_delete(return_value, "style", id);
    PHP_DELETE_REPLIGARD("style", id);

   TOUCH_CACHE;
}

MGD_FUNCTION(ret_type, copy_style, (int id, int root))
{
   zval **id, **root; int id_r;

   RETVAL_FALSE;
   CHECK_MGD;

   switch (ZEND_NUM_ARGS()) {
   case 1:
     if (zend_get_parameters_ex(1, &id) == FAILURE) {
      WRONG_PARAM_COUNT;
     } else {
       root = NULL;
   }
     break;
   case 2:
     if (zend_get_parameters_ex(2, &id, &root) == FAILURE) {
       WRONG_PARAM_COUNT;
     }
     break;
   default:

      WRONG_PARAM_COUNT;
   }

   convert_to_long_ex(id);
   if (root) {
   convert_to_long_ex(root);
   }

	/* root must be in same SG or be 0 */
   if (root) {
     if (root && (*root)->value.lval != 0 && !mgd_exists_bool(mgd_handle(), "style src, style tgt",
									"src.id=$d AND tgt.id=$d"
									" AND (src.sitegroup=tgt.sitegroup"
									" OR src.sitegroup=0"
									" OR tgt.sitegroup=0)",
							      (*id)->value.lval,(root)?(*root)->value.lval:0))
	RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);
   }

    id_r = mgd_copy_style(mgd_handle(),  (*id)->value.lval);
#if HAVE_MIDGARD_QUOTA
	if (mgd_get_quota_error(mgd_handle())) {
	  mgd_set_quota_error(mgd_handle(), 0);
	  RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
	}
#endif /* HAVE_MIDGARD_QUOTA */
    if(root && id_r) {
      php_midgard_update(return_value, "style", "up=$i", id_r, (*root)->value.lval);
      PHP_UPDATE_REPLIGARD("style",id_r);
    }
	RETVAL_LONG(id_r);

   TOUCH_CACHE;
}

MGD_FUNCTION(ret_type, delete_style_tree, (type param))
{
    IDINIT;
	CHECK_MGD;
    if (!isstyleowner(mgd_idfield(mgd_handle(), "up", "style", id)))
	RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    if(mgd_exists_id(mgd_handle(), "host", "style=$d", id))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
    if(mgd_exists_id(mgd_handle(), "page", "style=$d", id))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
    if(mgd_delete_style(mgd_handle(),  id))
	RETVAL_TRUE;

   TOUCH_CACHE;
}

MGD_MOVE_AND_TOUCH(style,style,style,up,1)

MGD_WALK_FUNCTION(style)

MIDGARD_CLASS(MidgardStyle, style, midgardstyle, style)

#endif
