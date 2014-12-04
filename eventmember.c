/* $Id: eventmember.c 10324 2006-11-27 12:47:04Z piotras $
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

int event_collides(int first, int second)
{
    midgard_res *res;
    int s1,s2,e1,e2;
    const char query[]="SELECT start,end FROM event WHERE id=$d AND busy=1";

    if((res = mgd_query(mgd_handle(),query,first)) && mgd_fetch(res))
	{
        s1=mgd_sql2int(res, 0);
        e1=mgd_sql2int(res, 1);
	}
	else
		return 0;
    mgd_release(res);

    if((res = mgd_query(mgd_handle(),query,second)) && mgd_fetch(res))
	{
        s2=mgd_sql2int(res, 0);
        e2=mgd_sql2int(res, 1);
	}
	else
		return 0;
    mgd_release(res);

	if(s1>=e2 || e1<=s2)
	    return 0;
	else
		return 1;
}

MGD_FUNCTION(ret_type, create_event_member, (type param))
{
	midgard_res *res;
	const char query[] = "SELECT eid FROM eventmember WHERE uid=$d";
	zval **eid, **uid, **extra, *self;

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "eid", eid)
		    || !MGD_PROPFIND(self, "uid", uid)
		    || !MGD_PROPFIND(self, "extra", extra)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 3
		    || zend_get_parameters_ex(3, &eid, &uid, &extra) != SUCCESS)
			WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(eid);
	convert_to_long_ex(uid);
	convert_to_string_ex(extra);

	if (!mgd_exists_id(mgd_handle(), "event", "id=$d", (*eid)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if (!mgd_exists_id(mgd_handle(), "person", "id=$d", (*uid)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if (!iseventowner((*eid)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	/* check collision if event is marked as busy */
	if (mgd_exists_id
	    (mgd_handle(), "event", "id=$d AND busy=1", (*eid)->value.lval)) {
		if ((res = mgd_query(mgd_handle(), query, (*uid)->value.lval))) {
			while (mgd_fetch(res)) {
				if (event_collides
				    ((*eid)->value.lval, mgd_sql2int(res, 0))) {
					mgd_release(res);
					RETURN_FALSE_BECAUSE(MGD_ERR_ERROR);
				}
			}
			mgd_release(res);
		}
	}
	php_midgard_create(return_value, self, "eventmember", "eid,uid,extra",
			   "$d,$d,$q", (*eid)->value.lval,
			   (*uid)->value.lval, (*extra)->value.str.val);

	PHP_CREATE_REPLIGARD("eventmember", return_value->value.lval);
}

MGD_FUNCTION(ret_type, update_event_member, (type param))
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
		    || zend_get_parameters_ex(2, &id, &extra) == FAILURE)
			   WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(id);
	convert_to_string_ex(extra);

	/* event member must exist and be in the same sitegroup as the current sitegroup
	 */
	if (!mgd_exists_id(mgd_handle(), "eventmember", "id=$d", (*id)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if (!iseventowner
	    (mgd_idfield(mgd_handle(), "eid", "eventmember", (*id)->value.lval)))
		   RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	php_midgard_update(return_value, "eventmember",
			   "extra=$q", (*id)->value.lval, (*extra)->value.str.val);
	PHP_UPDATE_REPLIGARD("eventmember", (*id)->value.lval);
}

MGD_FUNCTION(ret_type, delete_event_member, (type param))
{
    IDINIT;
	CHECK_MGD;
    if(mgd_has_dependants(mgd_handle(),id,"eventmember"))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

	if(!iseventowner(mgd_idfield(mgd_handle(), "eid", "eventmember", id)) && mgd_user(mgd_handle())!=mgd_idfield(mgd_handle(), "uid", "eventmember", id)) RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    php_midgard_delete(return_value, "eventmember", id);
    PHP_DELETE_REPLIGARD("eventmember", id);
}

MGD_FUNCTION(ret_type, get_event_member, (type param))
{
	zval **id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	case 0:
		php_midgard_bless(return_value, &MidgardEventMember);
		mgd_object_init(return_value, "eid", "uid", "extra", NULL);
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

   php_midgard_get_object(return_value, MIDGARD_OBJECT_EVENTMEMBER, (*id)->value.lval);
}

static const char *eventmember_sort(const char *order)
{
    static struct { const char *order, *sort; } sort[] = {
		{ "username", "username ASC" },
		{ "reverse username", "username DESC" },
		{ "reverse rname", "rname DESC" },
		{ "name", "name ASC" },
		{ "reverse name", "name DESC" },
		{ NULL, "rname ASC" }
    };
    int i;

    for (i = 0; sort[i].order; i++)
	if (strcmp(order, sort[i].order) == 0)
	    return sort[i].sort;

    return sort[i].sort;
}


MGD_FUNCTION(ret_type, list_event_members, (type param))
{
    const char *sortv;
    zval **id, **sortn;
    char *query;
	midgard_pool *pool;

    RETVAL_FALSE;
	CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 2:
	if (zend_get_parameters_ex(2, &id, &sortn) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_string_ex(sortn);
	    sortv = eventmember_sort((*sortn)->value.str.val);
	    break;
	}
    case 1:
	if (zend_get_parameters_ex(1, &id) == SUCCESS) {
	    convert_to_long_ex(id);
	    sortv = "rname";
	    break;
	}
    default:
	WRONG_PARAM_COUNT;
    }

    if (mgd_isadmin(mgd_handle())) {
    	php_midgard_select(&MidgardEventMember, return_value, 
						   "eventmember.id AS id,eid,uid,eventmember.extra"
			               ", " EMAIL_FIELDS ", " NAME_FIELDS
                     ",eventmember.sitegroup"
		                   , "eventmember,person",
		                   "eid=$d AND person.id=eventmember.uid", 
						   sortv, (*id)->value.lval);
	}
	else {
		pool=mgd_alloc_pool();
		query=mgd_format(mgd_handle(),pool, 
		                 "eventmember.id AS id,eid,uid,eventmember.extra"
		                 ", " NAME_FIELDS ", " EVENT_PUBLIC_FIELD(16,email)
			             ", " EVENT_EMAIL_FIELD
                        ",eventmember.sitegroup"
                         ,mgd_user(mgd_handle()), mgd_user(mgd_handle()));
    	php_midgard_select(&MidgardEventMember, return_value, query, "eventmember,person",
		                   "eid=$d AND person.id=eventmember.uid", 
						   sortv, (*id)->value.lval);
		
	}
}

MGD_FUNCTION(ret_type, count_event_members, (type param))
{
    midgard_res *res;

    IDINIT;
	CHECK_MGD;

   RETVAL_FALSE;

	res = mgd_query(mgd_handle(),"SELECT count(*) FROM $s WHERE eid=$d"
                              EVENT_SITEGROUP, "eventmember" , id
						 ,mgd_sitegroup(mgd_handle())
						 );
	if(res && mgd_fetch(res)) {
		RETVAL_LONG(mgd_sql2int(res,0));
		mgd_release(res);
	}
}

/* end calendar functions */

MIDGARD_CLASS(MidgardEventMember, eventmember, midgardeventmember, event_member)

#endif
