/* $Id: pagelink.c 7173 2004-04-15 11:55:14Z indeyets $
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

MGD_FUNCTION(ret_type, has_pagelinks, (type param))
{
#if HAVE_MIDGARD_PAGELINKS
	RETURN_TRUE;
#else
	RETURN_FALSE;
#endif
}

#if HAVE_MIDGARD_PAGELINKS
/* pagelink */

MGD_FUNCTION(ret_type, is_pagelink_owner, (type param))
{
    IDINIT;
    CHECK_MGD;
    RETVAL_LONG(ispagelinkowner(id));
}


MGD_FUNCTION(ret_type, list_pagelinks, (type param))
{
    IDINIT;
	CHECK_MGD;
    php_midgard_select(&MidgardPagelink, return_value,
		"pagelink.id AS id,"
		"pagelink.name AS name,"
		"pagelink.grp,"
		"pagelink.up,"
		"pagelink.target,"
		"page.name AS pname,"
		"page.style,"
		"page.title,"
		"page.changed,"
		"page.author,"
	    NAME_FIELD " AS authorname"
      ",pagelink.sitegroup"
		, "pagelink,page,person",
	    "pagelink.up=$d AND pagelink.target=page.id AND person.id=page.author",
		"pagelink.name", id);
}

MGD_FUNCTION(ret_type, list_pagelinks_targeted_at, (type param))
{
    IDINIT;
	CHECK_MGD;
    php_midgard_select(&MidgardPagelink, return_value,
		"pagelink.id AS id,"
		"pagelink.name AS name,"
		"pagelink.grp,"
		"pagelink.up,"
		"pagelink.target,"
		"page.name AS pname,"
		"page.style,"
		"page.title,"
		"page.changed,"
		"page.author,"
	    NAME_FIELD " AS authorname"
	        ",pagelink.sitegroup"
		, "pagelink,page,person",
	    "pagelink.target=$d AND pagelink.up=page.id AND person.id=page.author",
		"pagelink.name", id);
}

MGD_FUNCTION(ret_type, get_pagelink, (type param))
{
	zval **id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	case 0:
		php_midgard_bless(return_value, &MidgardPagelink);
		mgd_object_init(return_value, "up", "name", "target", "grp", "owner", NULL);
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

   php_midgard_get_object(return_value, MIDGARD_OBJECT_PAGELINK, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, get_pagelink_by_name, (type param))
{
    zval **root, **pagelink;
    int objid;

	CHECK_MGD;
    if (ZEND_NUM_ARGS() != 2
	|| zend_get_parameters_ex(2, &root, &pagelink) != SUCCESS)
	WRONG_PARAM_COUNT;
    convert_to_long_ex(root);
    convert_to_string_ex(pagelink);

   objid = mgd_exists_id(mgd_handle(), "element", "up=$d AND name=$q",
      (*root)->value.lval, (*pagelink)->value.str.val);
   php_midgard_get_object(return_value, MIDGARD_OBJECT_PAGELINK, objid);
}

MGD_FUNCTION(ret_type, create_pagelink, (type param))
{
    zval **up, **name, **target, **grp, **owner, *self;

    RETVAL_FALSE;
	CHECK_MGD;

   if ((self = getThis()) != NULL) {
      if (ZEND_NUM_ARGS() != 0) { WRONG_PARAM_COUNT; }

      if (!MGD_PROPFIND(self, "up", up)
            || !MGD_PROPFIND(self, "name", name)
            || !MGD_PROPFIND(self, "target", target)
            || !MGD_PROPFIND(self, "grp", grp)
            || !MGD_PROPFIND(self, "owner", owner)
            ) {
         RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
      }
   } else {
    if (ZEND_NUM_ARGS() != 5
		|| zend_get_parameters_ex(5, &up, &name, &target, &grp, &owner) == FAILURE)
	WRONG_PARAM_COUNT;
   }

    convert_to_long_ex(up);
    convert_to_string_ex(name);
    convert_to_long_ex(target);
    convert_to_long_ex(grp);
    convert_to_long_ex(owner);

    if (!(*up)->value.lval || !ispageowner((*up)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    if (!(*target)->value.lval || !ispageowner((*target)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	if (mgd_exists_id(mgd_handle(), "pagelink", "up=$d AND name=$q and grp=$d",
				  (*up)->value.lval, (*name)->value.str.val, (*grp)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);
	
	/* up must be in same SG or be 0 */
	if ((*up)->value.lval != 0 && !mgd_exists_id(mgd_handle(), "page",
			"id=$d AND sitegroup IN (0,$d)",
	(*up)->value.lval, mgd_sitegroup(mgd_handle())))
		RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);
	/* target must be in same SG or be 0 */
	if ((*target)->value.lval != 0 && !mgd_exists_id(mgd_handle(), "page",
			"id=$d AND sitegroup IN (0,$d)",
	(*target)->value.lval, mgd_sitegroup(mgd_handle())))
		RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);
	/* grp must be in same SG or be 0 */
	if ((*grp)->value.lval != 0 && !mgd_exists_id(mgd_handle(), "grp",
			"id=$d AND sitegroup IN (0,$d)",
			(*grp)->value.lval, mgd_sitegroup(mgd_handle())))
	       	RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);
	/* owner must be in same SG or be 0 */
	if ((*owner)->value.lval != 0 && !mgd_exists_id(mgd_handle(), "grp",
			"id=$d AND sitegroup IN (0,$d)",
			(*owner)->value.lval, mgd_sitegroup(mgd_handle())))
		RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);

    php_midgard_create(return_value, self, "pagelink",
				   "up,name,target,grp,owner",
				   "$d,$q,$d,$d,$d",
				   (*up)->value.lval, (*name)->value.str.val,
				   (*target)->value.lval,
				   (*grp)->value.lval, (*owner)->value.lval);
    PHP_CREATE_REPLIGARD("pagelink",return_value->value.lval);
}

MGD_FUNCTION(ret_type, update_pagelink, (type param))
{
    zval **id, **name, **target, **grp, **owner, *self;

    RETVAL_FALSE;
	CHECK_MGD;
   if ((self = getThis()) != NULL) {
      if (ZEND_NUM_ARGS() != 0) { WRONG_PARAM_COUNT; }

      if (!MGD_PROPFIND(self, "id", id)
            || !MGD_PROPFIND(self, "name", name)
            || !MGD_PROPFIND(self, "target", target)
            || !MGD_PROPFIND(self, "grp", grp)
            || !MGD_PROPFIND(self, "owner", owner)
            ) {
         RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
      }
   } else {
    if (ZEND_NUM_ARGS() != 5
	|| zend_get_parameters_ex(5, &id, &name, &target, &grp, &owner) == FAILURE)
	WRONG_PARAM_COUNT;
   }

    convert_to_long_ex(id);
    convert_to_string_ex(name);
    convert_to_long_ex(target);
    convert_to_long_ex(grp);
    convert_to_long_ex(owner);

    if (!ispagelinkowner((*id)->value.lval)
			|| !ispageowner((*target)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	/* target must be in same SG or be 0 */
	if ((*target)->value.lval == 0 || !mgd_exists_bool(mgd_handle(), "page,pagelink",
									"pagelink.id=$d AND page.id=$d"
									" AND (pagelink.sitegroup=page.sitegroup"
											" OR pagelink.sitegroup=0"
											" OR page.sitegroup=0)",
										(*id)->value.lval, (*target)->value.lval)) RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);
	/* grp must be in same SG or be 0 */
	if ((*grp)->value.lval != 0 && !mgd_exists_bool(mgd_handle(), "pagelink,grp",
										"pagelink.id=$d AND grp.id=$d"
										" AND (pagelink.sitegroup=grp.sitegroup"
											" OR pagelink.sitegroup=0"
											" OR grp.sitegroup=0)",
										(*id)->value.lval, (*grp)->value.lval)) RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);
	/* owner must be in same SG or be 0 */
	if ((*owner)->value.lval != 0 && !mgd_exists_bool(mgd_handle(), "pagelink,grp",
										"pagelink.id=$d AND grp.id=$d"
										" AND (pagelink.sitegroup=grp.sitegroup"
											" OR pagelink.sitegroup=0"
											" OR grp.sitegroup=0)",
										(*id)->value.lval, (*owner)->value.lval)) RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);

    php_midgard_update(return_value, "pagelink",
	       "name=$q,target=$d,grp=$d,owner=$d",
	       (*id)->value.lval,
	       (*name)->value.str.val, (*target)->value.lval, (*grp)->value.lval,
	       (*owner)->value.lval);
    PHP_UPDATE_REPLIGARD("pagelink",(*id)->value.lval);
}

MGD_FUNCTION(ret_type, delete_pagelink, (type param))
{
    IDINIT;
	CHECK_MGD;
    if(mgd_has_dependants(mgd_handle(),id,"pagelink"))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

    if (!ispagelinkowner(id))
	 RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

    php_midgard_delete(return_value, "pagelink", id);
    PHP_DELETE_REPLIGARD("pagelink", id);
}
/* /pagelink */

MIDGARD_CLASS(MidgardPagelink, pagelink, midgardpagelink, pagelink)

#endif


