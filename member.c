/* $Id: member.c 10324 2006-11-27 12:47:04Z piotras $
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

MGD_FUNCTION(ret_type, list_members, (type param))
{
    IDINIT;
	CHECK_MGD;
    php_midgard_select(&MidgardMember, return_value, "member.id AS id, person.id AS uid,"
		   NAME_FIELD " AS name," RNAME_FIELD " AS rname,department,"
		   "member.extra as extra, member.sitegroup"
		   , "person,member", "member.gid=$d AND member.uid=person.id",
		   "lastname,firstname", id);
}

MGD_FUNCTION(ret_type, list_memberships, (type param))
{
    IDINIT;
	CHECK_MGD;
    if (!isuserowner(id)) RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    php_midgard_select(&MidgardMember, return_value,
					"distinct member.id AS id, gid, "
					"IF(gid!=0,name,'Midgard Administrators') AS name"
         ",member.sitegroup"
		   , "grp,member", "member.uid=$d AND member.gid IN (0,grp.id)",
		   "name", id);
}

MGD_FUNCTION(ret_type, get_member, (type param))
{
	zval **id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	case 0:
	   php_midgard_bless(return_value, &MidgardMember);
	   mgd_object_init(return_value, "uid", "gid", "extra", NULL);
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

    if (!isgroupowner(mgd_idfield(mgd_handle(), "gid", "member", (*id)->value.lval)))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

   php_midgard_get_object(return_value, MIDGARD_OBJECT_MEMBER, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, create_member, (type param))
{
	zval **uid, **gid, **extra, *self;

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "uid", uid)
		    || !MGD_PROPFIND(self, "gid", gid)
		    || !MGD_PROPFIND(self, "extra", extra)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 3
		    || zend_get_parameters_ex(3, &uid, &gid, &extra) != SUCCESS)
			WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(uid);
	convert_to_long_ex(gid);
	convert_to_string_ex(extra);

  /* Allow SG0 root user to add persons as group members.
   * Do not check if root admin is group member as this is 
   * root admin. Uncomment to make it working.
   */ 
  
  if (mgd_isroot(mgd_handle()) != 1) { 
    if (!isgroupowner((*gid)->value.lval)
	    || mgd_exists_id(mgd_handle(), "member", "uid=$d AND gid=$d",
			     (*uid)->value.lval, (*gid)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   } 

	if ((*gid)->value.lval == 0) {
		/* only root can put users in root group */
		if (!mgd_isroot(mgd_handle()))
			RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
		if (mgd_sitegroup(mgd_handle()) != 0)
			RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);

		/* person must exist and be in SG0 */
		if (!mgd_exists_bool(mgd_handle(),
				     "person",
				     "id=$d AND sitegroup=0",
				     (*uid)->value.lval))
			   RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}
	else {
		/* only root users in SG0 please */
		/* this can be safely commented , when we need to define SG0 groups with members.
      
    if (mgd_sitegroup(mgd_handle()) == 0)
			RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    */
    
		/* group must exist and be in the same sitegroup as the person and
		   the current sitegroup
		 */
		if (!mgd_exists_bool(mgd_handle(),
				     "person,grp",
				     "person.id=$d AND person.sitegroup=$d"
				     " AND grp.id=$d AND grp.sitegroup=$d",
				     (*uid)->value.lval,
				     mgd_sitegroup(mgd_handle()),
				     (*gid)->value.lval,
				     mgd_sitegroup(mgd_handle())))
			   RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}

	php_midgard_create(return_value, self, "member", "uid,gid,extra", "$d,$d,$q",
			   (*uid)->value.lval, (*gid)->value.lval,
			   (*extra)->value.str.val);

	PHP_CREATE_REPLIGARD("member", return_value->value.lval);
}

MGD_FUNCTION(ret_type, update_member, (type param))
{
	zval **id, **extra, *self;

	RETVAL_FALSE;
	CHECK_MGD;
	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "id", id)
		    || !MGD_PROPFIND(self, "extra", extra)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 2
		    || zend_get_parameters_ex(2, &id,
				     &extra) != SUCCESS) WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(id);
	convert_to_string_ex(extra);

	if (!isgroupowner
	    (mgd_idfield(mgd_handle(), "gid", "member", (*id)->value.lval)))
		   RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	php_midgard_update(return_value, "member", "extra=$q", (*id)->value.lval,
			   (*extra)->value.str.val);
	PHP_UPDATE_REPLIGARD("member", (*id)->value.lval);
}

MGD_FUNCTION(ret_type, delete_member, (type param))
{
    IDINIT;
	CHECK_MGD;
    if(mgd_has_dependants(mgd_handle(),id,"member"))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

    if (!isgroupowner(mgd_idfield(mgd_handle(), "gid", "member", id))) RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    php_midgard_delete(return_value, "member", id);
    PHP_DELETE_REPLIGARD("member", id);
}

MIDGARD_CLASS(MidgardMember, member, midgardmember, member)

#endif
