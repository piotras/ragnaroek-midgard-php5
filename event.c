/* $Id: event.c 10324 2006-11-27 12:47:04Z piotras $
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

/* begin calendar functions */

MGD_FUNCTION(ret_type, is_event_owner, (type param))
{
    IDINIT;
    CHECK_MGD;
    RETVAL_LONG(iseventowner(id));
}

MGD_FUNCTION(ret_type, create_event, (type param))
{
	zval **up, **start, **end, **title, **description;
	zval **type, **extra, **owner, **busy, *self;

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "up", up)
		    || !MGD_PROPFIND(self, "start", start)
		    || !MGD_PROPFIND(self, "end", end)
		    || !MGD_PROPFIND(self, "title", title)
		    || !MGD_PROPFIND(self, "description", description)
		    || !MGD_PROPFIND(self, "type", type)
		    || !MGD_PROPFIND(self, "extra", extra)
		    || !MGD_PROPFIND(self, "owner", owner)
		    || !MGD_PROPFIND(self, "busy", busy)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 9
		    || zend_get_parameters_ex(9, &up, &start, &end, &title,
				     &description, &type, &extra, &owner,
				     &busy) != SUCCESS)
			WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(up);
	convert_to_long_ex(start);
	convert_to_long_ex(end);
	convert_to_string_ex(title);
	convert_to_string_ex(description);
	convert_to_long_ex(type);
	convert_to_string_ex(extra);
	convert_to_long_ex(owner);
	convert_to_boolean_ex(busy);

	if ((*start)->value.lval > (*end)->value.lval)
		RETURN_FALSE_BECAUSE(MGD_ERR_RANGE);

	if ((*up)->value.lval && !iseventowner((*up)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if ((*up)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "event", "id=$d", (*up)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	/* TODO: should we in fact allow owner == 0 for non-root? */

	if ((*owner)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "grp", "id=$d", (*owner)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	php_midgard_create(return_value, self, "event", "up,start,end,title,"
			   "description,type,extra,owner,creator,created,revisor,revised,revision,busy",
			   "$d,$d,$d,$q,$q,$d,$q,$d,$d,Now(),$d,Now(),0,$d",
			   (*up)->value.lval, (*start)->value.lval, (*end)->value.lval,
			   (*title)->value.str.val, (*description)->value.str.val,
			   (*type)->value.lval, (*extra)->value.str.val,
			   (*owner)->value.lval, mgd_user(mgd_handle()),
			   mgd_user(mgd_handle()), (*busy)->value.lval);

	PHP_CREATE_REPLIGARD("event", return_value->value.lval);
}

MGD_FUNCTION(ret_type, update_event, (type param))
{
	zval **id, **start, **end, **title, **description;
	zval **type, **extra, **owner, **busy, *self;

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "id", id)
		    || !MGD_PROPFIND(self, "start", start)
		    || !MGD_PROPFIND(self, "end", end)
		    || !MGD_PROPFIND(self, "title", title)
		    || !MGD_PROPFIND(self, "description", description)
		    || !MGD_PROPFIND(self, "type", type)
		    || !MGD_PROPFIND(self, "extra", extra)
		    || !MGD_PROPFIND(self, "owner", owner)
		    || !MGD_PROPFIND(self, "busy", busy)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 9
		    || zend_get_parameters_ex(9, &id, &start, &end,
				     &title, &description, &type, &extra,
				     &owner, &busy) == FAILURE)

			WRONG_PARAM_COUNT;
	}
	convert_to_long_ex(id);
	convert_to_long_ex(start);
	convert_to_long_ex(end);
	convert_to_string_ex(title);
	convert_to_string_ex(description);
	convert_to_long_ex(type);
	convert_to_string_ex(extra);
	convert_to_long_ex(owner);
	convert_to_boolean_ex(busy);

	if ((*start)->value.lval > (*end)->value.lval)
		RETURN_FALSE_BECAUSE(MGD_ERR_RANGE);

	if (!(*id)->value.lval || !iseventowner((*id)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if ((*owner)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "grp", "id=$d", (*owner)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	php_midgard_update(return_value, "event",
			   "start=$d,end=$d,title=$q,"
			   "description=$q,type=$d,extra=$q,owner=$d,revisor=$d,revised=Now(),revision=revision+1,busy=$d",
			   (*id)->value.lval,
			   (*start)->value.lval, (*end)->value.lval,
			   (*title)->value.str.val, (*description)->value.lval,
			   (*type)->value.lval, (*extra)->value.str.val,
			   (*owner)->value.lval, mgd_user(mgd_handle()),
			   (*busy)->value.lval);
	PHP_UPDATE_REPLIGARD("event", (*id)->value.lval);
}

MGD_FUNCTION(ret_type, delete_event, (type param))
{
    IDINIT;
	CHECK_MGD;
    if(mgd_has_dependants(mgd_handle(),id,"event"))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

    if (mgd_exists_id(mgd_handle(), "event", "up=$d", id)
	 || mgd_exists_id(mgd_handle(), "eventmember", "eid=$d", id))
		RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

	if (!id || !iseventowner(id))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

    php_midgard_delete(return_value, "event", id);
    PHP_DELETE_REPLIGARD("event", id);
}

MGD_FUNCTION(ret_type, delete_event_tree, (type param))
{
    IDINIT;
    CHECK_MGD;
    if (!iseventowner(mgd_idfield(mgd_handle(), "up", "event", id))) RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    if(mgd_delete_event(mgd_handle(),  id)) { RETURN_TRUE; }
    RETURN_FALSE;
}

MGD_FUNCTION(ret_type, get_event, (type param))
{
	zval **id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	case 0:
		php_midgard_bless(return_value, &MidgardEvent);
		mgd_object_init(return_value, "up", "start", "end",
						"title", "description", "type", "extra",
						"owner", "busy", NULL);
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

   php_midgard_get_object(return_value, MIDGARD_OBJECT_EVENT, (*id)->value.lval);
}

static const char *event_sort(const char *order)
{
    static struct { const char *order, *sort; } sort[] = {
		{ "start", "event.start ASC" },
		{ "reverse start", "event.start DESC" },
		{ "alpha", "event.title ASC" },
		{ "reverse alpha", "event.title DESC" },
		{ "end", "event.end ASC" },
		{ "reverse end", "event.end DESC" },
		{ "up", "event.up ASC,event.id ASC" },
		{ "reverse up", "event.up DESC,event.id DESC" },
		{ "created", "event.created ASC" },
		{ NULL, "event.created DESC" }
    };
    int i;

    for (i = 0; sort[i].order; i++)
	if (strcmp(order, sort[i].order) == 0)
	    return sort[i].sort;

    return sort[i].sort;
}

MGD_FUNCTION(ret_type, list_events, (type param))
{
    const char *sortv;
    int typev;
    zval **id, **sortn, **typen;

    RETVAL_FALSE;
	CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 3:
	if (zend_get_parameters_ex(3, &id, &sortn, &typen) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_string_ex(sortn);
	    convert_to_long_ex(typen);
	    sortv = event_sort((*sortn)->value.str.val);
	    typev = (*typen)->value.lval;
	    break;
	}
    case 2:
	if (zend_get_parameters_ex(2, &id, &sortn) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_string_ex(sortn);
	    sortv = event_sort((*sortn)->value.str.val);
	    typev = -1;
	    break;
	}
    case 1:
	if (zend_get_parameters_ex(1, &id) == SUCCESS) {
	    convert_to_long_ex(id);
	    sortv = "event.id DESC";
	    typev = -1;
	    break;
	}
    default:
	WRONG_PARAM_COUNT;
    }

    if (typev == -1)
	    php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
		                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
						   "event", "event.up=$d",
			               sortv, (*id)->value.lval);
    else
	    php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
		                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
						   "event",
			               "event.type=$d AND event.up=$d",
			               sortv, typev, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, list_events_between, (type param))
{
    const char *sortv;
    int typev;
    zval **id, **sortn, **typen;
    zval **start, **end;

    RETVAL_FALSE;
	CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 5:
	if (zend_get_parameters_ex(5, &id, &start, &end, &sortn, &typen) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_long_ex(start);
	    convert_to_long_ex(end);
	    convert_to_string_ex(sortn);
	    convert_to_long_ex(typen);
	    sortv = event_sort((*sortn)->value.str.val);
	    typev = (*typen)->value.lval;
	    break;
	}
    case 4:
	if (zend_get_parameters_ex(4, &id, &start, &end, &sortn) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_long_ex(start);
	    convert_to_long_ex(end);
	    convert_to_string_ex(sortn);
	    sortv = event_sort((*sortn)->value.str.val);
	    typev = -1;
	    break;
	}
    case 3:
	if (zend_get_parameters_ex(3, &id, &start, &end) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_long_ex(start);
	    convert_to_long_ex(end);
	    sortv = "event.id DESC";
	    typev = -1;
	    break;
	}
    default:
	WRONG_PARAM_COUNT;
    }

    if (typev == -1)
	    php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
		                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
						   "event", "event.up=$d AND "
						   "((event.start>=$d AND event.start<=$d) OR "
						   "(event.start<=$d AND event.end>=$d) OR "
						   "(event.end>=$d AND event.end<=$d))",
			               sortv, (*id)->value.lval,
						   (*start)->value.lval,(*end)->value.lval,
						   (*start)->value.lval,(*end)->value.lval,
						   (*start)->value.lval,(*end)->value.lval);
    else
	    php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
		                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
						   "event",
			               "event.type=$d AND event.up=$d AND "
						   "((event.start>=$d AND event.start<=$d) OR "
						   "(event.start<=$d AND event.end>=$d) OR "
						   "(event.end>=$d AND event.end<=$d))",
			               sortv, typev, (*id)->value.lval,
						   (*start)->value.lval,(*end)->value.lval,
						   (*start)->value.lval,(*end)->value.lval,
						   (*start)->value.lval,(*end)->value.lval);
}

MGD_FUNCTION(ret_type, list_events_by_group, (type param))
{
    const char *sortv;
    int typev;
    zval **id, **sortn, **typen;

    RETVAL_FALSE;
        CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 3:
        if (zend_get_parameters_ex(3, &id, &sortn, &typen) == SUCCESS) {
            convert_to_long_ex(id);
            convert_to_string_ex(sortn);
            convert_to_long_ex(typen);
            sortv = event_sort((*sortn)->value.str.val);
            typev = (*typen)->value.lval;
            break;
        }
    case 2:
        if (zend_get_parameters_ex(2, &id, &sortn) == SUCCESS) {
            convert_to_long_ex(id);
            convert_to_string_ex(sortn);
            sortv = event_sort((*sortn)->value.str.val);
            typev = -1;
            break;
        }
    case 1:
        if (zend_get_parameters_ex(1, &id) == SUCCESS) {
            convert_to_long_ex(id);
            sortv = "event.id DESC";
            typev = -1;
            break;
        }
    default:
        WRONG_PARAM_COUNT;
    }

    if (typev == -1)
            php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
                                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
                                                   "event", "event.owner=$d",
                                       sortv, (*id)->value.lval);
    else
            php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
                                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
                                                   "event",
                                       "event.type=$d AND event.owner=$d",
                                       sortv, typev, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, list_events_between_by_group, (type param))
{
    const char *sortv;
    int typev;
    zval **id, **sortn, **typen;
    zval **start, **end;

    RETVAL_FALSE;
        CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 5:
        if (zend_get_parameters_ex(5, &id, &start, &end, &sortn, &typen) == SUCCESS) {
            convert_to_long_ex(id);
            convert_to_long_ex(start);
            convert_to_long_ex(end);
            convert_to_string_ex(sortn);
            convert_to_long_ex(typen);
            sortv = event_sort((*sortn)->value.str.val);
            typev = (*typen)->value.lval;
            break;
        }
    case 4:
        if (zend_get_parameters_ex(4, &id, &start, &end, &sortn) == SUCCESS) {
            convert_to_long_ex(id);
            convert_to_long_ex(start);
            convert_to_long_ex(end);
            convert_to_string_ex(sortn);
            sortv = event_sort((*sortn)->value.str.val);
            typev = -1;
            break;
        }
    case 3:
        if (zend_get_parameters_ex(3, &id, &start, &end) == SUCCESS) {
            convert_to_long_ex(id);
            convert_to_long_ex(start);
            convert_to_long_ex(end);
            sortv = "event.id DESC";
            typev = -1;
            break;
        }
    default:
        WRONG_PARAM_COUNT;
    }
    if (typev == -1)
            php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
                                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
                                                   "event", "event.owner=$d AND "
                                                   "((event.start>=$d AND event.start<=$d) OR "
                                                   "(event.start<=$d AND event.end>=$d) OR "
                                                   "(event.end>=$d AND event.end<=$d))",
                                       sortv, (*id)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval);
    else
            php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
                                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
                                                   "event",
                                       "event.type=$d AND event.owner=$d AND "
                                                   "((event.start>=$d AND event.start<=$d) OR "
                                                   "(event.start<=$d AND event.end>=$d) OR "
                                                   "(event.end>=$d AND event.end<=$d))",
                                       sortv, typev, (*id)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval);
}

MGD_FUNCTION(ret_type, list_events_by_person, (type param))
{
    const char *sortv;
    int typev;
    zval **id, **sortn, **typen;

    RETVAL_FALSE;
        CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 3:
        if (zend_get_parameters_ex(3, &id, &sortn, &typen) == SUCCESS) {
            convert_to_long_ex(id);
            convert_to_string_ex(sortn);
            convert_to_long_ex(typen);
            sortv = event_sort((*sortn)->value.str.val);
            typev = (*typen)->value.lval;
            break;
        }
    case 2:
        if (zend_get_parameters_ex(2, &id, &sortn) == SUCCESS) {
            convert_to_long_ex(id);
            convert_to_string_ex(sortn);
            sortv = event_sort((*sortn)->value.str.val);
            typev = -1;
            break;
        }
    case 1:
        if (zend_get_parameters_ex(1, &id) == SUCCESS) {
            convert_to_long_ex(id);
            sortv = "event.id DESC";
            typev = -1;
            break;
        }
    default:
        WRONG_PARAM_COUNT;
    }

    if (typev == -1)
            php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
                                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
                                                   "event", "event.creator=$d",
                                       sortv, (*id)->value.lval);
    else
            php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
                                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
                                                   "event",
                                       "event.type=$d AND event.creator=$d",
                                       sortv, typev, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, list_events_between_by_person, (type param))
{
    const char *sortv;
    int typev;
    zval **id, **sortn, **typen;
    zval **start, **end;

    RETVAL_FALSE;
        CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 5:
        if (zend_get_parameters_ex(5, &id, &start, &end, &sortn, &typen) == SUCCESS) {
            convert_to_long_ex(id);
            convert_to_long_ex(start);
            convert_to_long_ex(end);
            convert_to_string_ex(sortn);
            convert_to_long_ex(typen);
            sortv = event_sort((*sortn)->value.str.val);
            typev = (*typen)->value.lval;
            break;
        }
    case 4:
        if (zend_get_parameters_ex(4, &id, &start, &end, &sortn) == SUCCESS) {
            convert_to_long_ex(id);
            convert_to_long_ex(start);
            convert_to_long_ex(end);
            convert_to_string_ex(sortn);
            sortv = event_sort((*sortn)->value.str.val);
            typev = -1;
            break;
        }
    case 3:
        if (zend_get_parameters_ex(3, &id, &start, &end) == SUCCESS) {
            convert_to_long_ex(id);
            convert_to_long_ex(start);
            convert_to_long_ex(end);
            sortv = "event.id DESC";
            typev = -1;
            break;
        }
    default:
        WRONG_PARAM_COUNT;
    }
    if (typev == -1)
            php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
                                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
                                                   "event", "event.creator=$d AND "
                                                   "((event.start>=$d AND event.start<=$d) OR "
                                                   "(event.start<=$d AND event.end>=$d) OR "
                                                   "(event.end>=$d AND event.end<=$d))",
                                       sortv, (*id)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval);
    else
            php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
                                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
                                                   "event",
                                       "event.type=$d AND event.creator=$d AND "
                                                   "((event.start>=$d AND event.start<=$d) OR "
                                                   "(event.start<=$d AND event.end>=$d) OR "
                                                   "(event.end>=$d AND event.end<=$d))",
                                       sortv, typev, (*id)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval);
}

MGD_FUNCTION(ret_type, list_events_between_by_member, (type param))
{
    const char *sortv;
    int typev;
    zval **id, **sortn, **typen;
    zval **start, **end;

    RETVAL_FALSE;
        CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 5:
        if (zend_get_parameters_ex(5, &id, &start, &end, &sortn, &typen) == SUCCESS) {
            convert_to_long_ex(id);
            convert_to_long_ex(start);
            convert_to_long_ex(end);
            convert_to_string_ex(sortn);
            convert_to_long_ex(typen);
            sortv = event_sort((*sortn)->value.str.val);
            typev = (*typen)->value.lval;
            break;
        }
    case 4:
        if (zend_get_parameters_ex(4, &id, &start, &end, &sortn) == SUCCESS) {
            convert_to_long_ex(id);
            convert_to_long_ex(start);
            convert_to_long_ex(end);
            convert_to_string_ex(sortn);
            sortv = event_sort((*sortn)->value.str.val);
            typev = -1;
            break;
        }
    case 3:
        if (zend_get_parameters_ex(3, &id, &start, &end) == SUCCESS) {
            convert_to_long_ex(id);
            convert_to_long_ex(start);
            convert_to_long_ex(end);
            sortv = "event.id DESC";
            typev = -1;
            break;
        }
    default:
        WRONG_PARAM_COUNT;
    }
    if (typev == -1)
            php_midgard_select(&MidgardEvent, return_value, "event.id AS id,up,start,end,title,"
                                   "description,type,event.extra AS extra,owner,busy",
                                                   "event,eventmember", "eventmember.uid=$d AND event.id=eventmember.eid AND "
                                                   "((event.start>=$d AND event.start<=$d) OR "
                                                   "(event.start<=$d AND event.end>=$d) OR "
                                                   "(event.end>=$d AND event.end<=$d))",
                                       sortv, (*id)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval);
    else
            php_midgard_select(&MidgardEvent, return_value, "event.id AS id,up,start,end,title,"
                                   "description,type,event.extra AS extra,owner,busy",
                                                   "event,eventmember",
                                       "event.type=$d AND eventmember.uid=$d AND event.id=eventmember.eid AND "
                                                   "((event.start>=$d AND event.start<=$d) OR "
                                                   "(event.start<=$d AND event.end>=$d) OR "
                                                   "(event.end>=$d AND event.end<=$d))",
                                       sortv, typev, (*id)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval,
                                                   (*start)->value.lval,(*end)->value.lval);
}


MGD_FUNCTION(ret_type, list_events_all, (type param))
{
    int *events;
    const char *sortv;
    int typev;
    zval **id, **sortn, **typen;

    RETVAL_FALSE;
	CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 3:
	if (zend_get_parameters_ex(3, &id, &sortn, &typen) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_string_ex(sortn);
	    convert_to_long_ex(typen);
	    sortv = event_sort((*sortn)->value.str.val);
	    typev = (*typen)->value.lval;
	    break;
	}
    case 2:
	if (zend_get_parameters_ex(2, &id, &sortn) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_string_ex(sortn);
	    sortv = event_sort((*sortn)->value.str.val);
	    typev = -1;
	    break;
	}
    case 1:
	if (zend_get_parameters_ex(1, &id) == SUCCESS) {
	    convert_to_long_ex(id);
	    sortv = "event.id DESC";
	    typev = -1;
	    break;
	}
    default:
	WRONG_PARAM_COUNT;
    }

	if((*id)->value.lval) {
		if(!(events = mgd_tree(mgd_handle(), "event", "up",
										(*id)->value.lval, 0, NULL)))
			RETURN_FALSE_BECAUSE(MGD_ERR_ERROR);
    	if (typev == -1)
	    	php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
			                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
							   "event", "event.id IN $D",
				               sortv, events);
    	else
	    	php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
			                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
							   "event",
				               "event.type=$d AND event.id IN $D",
				               sortv, typev, events);

		free(events);
	}
	else {
    	if (typev == -1)
	    	php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
			                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
							   "event", NULL, sortv);
    	else
	    	php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
			                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
							   "event", "event.type=$d",
				               sortv, typev);
	}
}

MGD_FUNCTION(ret_type, list_events_all_between, (type param))
{
    int *events;
    const char *sortv;
    int typev;
    zval **id, **sortn, **typen;
    zval **start, **end;

    RETVAL_FALSE;
	CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 5:
	if (zend_get_parameters_ex(5, &id, &start, &end, &sortn, &typen) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_long_ex(start);
	    convert_to_long_ex(end);
	    convert_to_string_ex(sortn);
	    convert_to_long_ex(typen);
	    sortv = event_sort((*sortn)->value.str.val);
	    typev = (*typen)->value.lval;
	    break;
	}
    case 4:
	if (zend_get_parameters_ex(4, &id, &start, &end, &sortn) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_long_ex(start);
	    convert_to_long_ex(end);
	    convert_to_string_ex(sortn);
	    sortv = event_sort((*sortn)->value.str.val);
	    typev = -1;
	    break;
	}
    case 3:
	if (zend_get_parameters_ex(3, &id, &start, &end) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_long_ex(start);
	    convert_to_long_ex(end);
	    sortv = "event.id DESC";
	    typev = -1;
	    break;
	}
    default:
	WRONG_PARAM_COUNT;
    }

	if((*id)->value.lval) {
		if(!(events = mgd_tree(mgd_handle(), "event", "up",
										(*id)->value.lval, 0, NULL)))
			RETURN_FALSE_BECAUSE(MGD_ERR_ERROR);
	    if (typev == -1)
		    php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
			                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
							   "event", "event.id in $D AND "
							   "((event.start>=$d AND event.start<=$d) OR "
							   "(event.start<=$d AND event.end>=$d) OR "
							   "(event.end>=$d AND event.end<=$d))",
				               sortv, events,
							   (*start)->value.lval,(*end)->value.lval,
							   (*start)->value.lval,(*end)->value.lval,
							   (*start)->value.lval,(*end)->value.lval);
	    else
		    php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
			                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
							   "event",
				               "event.type=$d AND event.id IN $D AND "
							   "((event.start>=$d AND event.start<=$d) OR "
							   "(event.start<=$d AND event.end>=$d) OR "
							   "(event.end>=$d AND event.end<=$d))",
				               sortv, typev, events,
							   (*start)->value.lval,(*end)->value.lval,
							   (*start)->value.lval,(*end)->value.lval,
							   (*start)->value.lval,(*end)->value.lval);
		free(events);
	} else {
	    if (typev == -1)
		    php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
			                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
							   "event",
							   "((event.start>=$d AND event.start<=$d) OR "
							   "(event.start<=$d AND event.end>=$d) OR "
							   "(event.end>=$d AND event.end<=$d))",
				               sortv,
							   (*start)->value.lval,(*end)->value.lval,
							   (*start)->value.lval,(*end)->value.lval,
							   (*start)->value.lval,(*end)->value.lval);
		else
		    php_midgard_select(&MidgardEvent, return_value, "id,up,start,end,title,"
			                   "description,type,extra,owner,busy" SITEGROUP_SELECT,
							   "event",
				               "event.type=$d AND "
							   "((event.start>=$d AND event.start<=$d) OR "
							   "(event.start<=$d AND event.end>=$d) OR "
							   "(event.end>=$d AND event.end<=$d))",
				               sortv, typev,
							   (*start)->value.lval,(*end)->value.lval,
							   (*start)->value.lval,(*end)->value.lval,
							   (*start)->value.lval,(*end)->value.lval);
	}
}

void php_midgard_select_count_event_in_month(zval *return_value,
   const char *tables, const char *where,
   struct tm *start, struct tm *end)
{
   midgard_res *res=NULL;
	zval day;
	struct { int total; int both; int start; int end; int none; } days[31];
	struct tm s,e;
	time_t tss,tse,tsstart,tsend;
	int i;

    RETVAL_FALSE;

	res = mgd_query(mgd_handle(),"SELECT start, end FROM $s WHERE $s",
                                tables, where);
	if(res) {
		array_init(return_value);
		add_assoc_long(return_value,"total",mgd_rows(res));
		for(i=0;i<31;i++)
		{
			days[i].total=0;
			days[i].both=0;
			days[i].start=0;
			days[i].end=0;
			days[i].none=0;
		}
		while(mgd_fetch(res))
		{
			tss=mgd_sql2int(res,EVENT_START);
			tse=mgd_sql2int(res,EVENT_END);
			tsstart=mktime(start);
			tsend=mktime(end);
			memcpy((void *)&s,(void *)localtime(&tss),sizeof(s));
			memcpy((void *)&e,(void *)localtime(&tse),sizeof(e));
			if(tss<tsstart && tse>tsend)
			{
				for(i=start->tm_mday;i<=end->tm_mday;i++)
				{
					days[i].none++;
					days[i].total++;
				}
				continue;
			}
			if(tss<tsend && tse>tsend)
			{
				days[s.tm_mday].start++;
				days[s.tm_mday].total++;
				for(i=s.tm_mday+1;i<=end->tm_mday;i++)
				{
					days[i].none++;
					days[i].total++;
				}
				continue;
			}
			if(tss<tsstart && tse>tsstart)
			{
				days[e.tm_mday].end++;
				days[e.tm_mday].total++;
				for(i=start->tm_mday;i<e.tm_mday;i++)
				{
					days[i].none++;
					days[i].total++;
				}
				continue;
			}
			if(tss>=tsstart && tss<=tsend && tse<=tsend)
			{
				if ( s.tm_mday == e.tm_mday
				  && s.tm_mon  == e.tm_mon
				  && s.tm_year == e.tm_year)
				{
					days[s.tm_mday].both++;
					days[s.tm_mday].start++;
					days[e.tm_mday].end++;
					days[e.tm_mday].total++;
				}
				else
				{
					days[s.tm_mday].start++;
					days[s.tm_mday].total++;
					days[e.tm_mday].end++;
					days[e.tm_mday].total++;
					for(i=s.tm_mday+1;i<e.tm_mday;i++)
					{
						days[i].none++;
						days[i].total++;
					}
				}
			}
		}
		for(i=1;i<=end->tm_mday;i++)
		{
			array_init(&day);
			add_assoc_long(&day,"total",days[i].total);
			add_assoc_long(&day,"both",days[i].both);
			add_assoc_long(&day,"start",days[i].start);
			add_assoc_long(&day,"end",days[i].end);
			add_assoc_long(&day,"none",days[i].none);
			zend_hash_index_update(return_value->value.ht,i,
			                                 &day,sizeof(pval),NULL);
		}
		mgd_release(res);
    }
	else {
		array_init(return_value);
		add_assoc_long(return_value,"total",0);
	}
}

MGD_FUNCTION(ret_type, count_events_in_period, (type param))
{
    midgard_res *res;
    zval **startn, **endn, **uidn, **typen;
    const char *table=EVENT_COUNT_TABLE;
    char *where;
    midgard_pool *pool=NULL;

    RETVAL_FALSE;
	CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 4:
	if (zend_get_parameters_ex(4, &startn, &endn, &uidn, &typen) == SUCCESS) {
		convert_to_long_ex(startn);
		convert_to_long_ex(endn);
	    convert_to_long_ex(uidn);
	    convert_to_long_ex(typen);
		pool=mgd_alloc_pool();
		if((*uidn)->value.lval<=0)
			where=mgd_format(mgd_handle(),pool,EVENT_COUNT_WHERE_43,
			                 (*startn)->value.lval,(*endn)->value.lval,
					 (*typen)->value.lval,
					 mgd_sitegroup(mgd_handle()));
		else {
			table=EVENT_COUNT_TABLE_2;
			where=mgd_format(mgd_handle(),pool,EVENT_COUNT_WHERE_4,
			                 (*startn)->value.lval,(*endn)->value.lval,
					 (*uidn)->value.lval,(*typen)->value.lval,
					 mgd_sitegroup(mgd_handle()),
					 mgd_sitegroup(mgd_handle()));
		}
	    break;
	}
    case 3:
	if (zend_get_parameters_ex(3, &startn, &endn, &uidn) == SUCCESS) {
		convert_to_long_ex(startn);
		convert_to_long_ex(endn);
	    convert_to_long_ex(uidn);
		pool=mgd_alloc_pool();
		where=mgd_format(mgd_handle(),pool,EVENT_COUNT_WHERE_3,
		                 (*startn)->value.lval,(*endn)->value.lval,
				 (*uidn)->value.lval,
				 mgd_sitegroup(mgd_handle()));
	    break;
	}
    case 2:
	if (zend_get_parameters_ex(2, &startn, &endn) == SUCCESS) {
		convert_to_long_ex(startn);
		convert_to_long_ex(endn);
		pool=mgd_alloc_pool();
		where=mgd_format(mgd_handle(),pool,EVENT_COUNT_WHERE_2,
		                 (*startn)->value.lval,(*endn)->value.lval,
				 mgd_sitegroup(mgd_handle()));
	    break;
	}
    case 1:
	if (zend_get_parameters_ex(1, &endn) == SUCCESS) {
		convert_to_long_ex(endn);
		pool=mgd_alloc_pool();
		where=mgd_format(mgd_handle(),pool,EVENT_COUNT_WHERE_1,
		                 (*endn)->value.lval,
				 mgd_sitegroup(mgd_handle()));
	    break;
	}
    case 0:
		pool=mgd_alloc_pool();
		where=mgd_format(mgd_handle(),pool,EVENT_COUNT_WHERE_0,
		                 mgd_sitegroup(mgd_handle()));
	    break;
    default:
		WRONG_PARAM_COUNT;
    }
	res = mgd_query(mgd_handle(),"SELECT count(*) FROM $s WHERE $s",
                                table, where);
	if(res && mgd_fetch(res)) {
		RETVAL_LONG(mgd_sql2int(res,0));
		mgd_release(res);
	}
	if(pool!=NULL)
		mgd_free_pool(pool);
}

MGD_FUNCTION(ret_type, count_events_in_month, (type param))
{
    zval **monthn, **yearn, **typen;
    char *where;
    midgard_pool *pool=NULL;
    struct tm start,end;
    time_t tsstart,tsend;

    RETVAL_FALSE;
	CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 3:
	if (zend_get_parameters_ex(3, &monthn, &yearn, &typen) == SUCCESS) {
		convert_to_long_ex(monthn);
		convert_to_long_ex(yearn);
	    convert_to_long_ex(typen);
	    break;
	}
    case 2:
	if (zend_get_parameters_ex(2, &monthn, &yearn) == SUCCESS) {
		convert_to_long_ex(monthn);
		convert_to_long_ex(yearn);
	    break;
	}
    default:
		WRONG_PARAM_COUNT;
    }
	start.tm_year=end.tm_year=(*yearn)->value.lval-1900;
	start.tm_mon=(*monthn)->value.lval-1; end.tm_mon=(*monthn)->value.lval;
	start.tm_mday=end.tm_mday=1;
	start.tm_hour=end.tm_hour=0;
	start.tm_min=end.tm_min=0;
	start.tm_sec=0; end.tm_sec=-1;
	tsstart=mktime(&start);
	tsend=mktime(&end);
	memcpy((void *)&end,(void *)localtime(&tsend),sizeof(end));
    pool=mgd_alloc_pool();
	if(ZEND_NUM_ARGS()==3) {
		where=mgd_format(mgd_handle(),pool,EVENT_MONTH_WHERE_TYPE,
						 tsstart,tsend,
						 tsstart,tsend,
						 tsstart,tsend,
						 (*typen)->value.lval,
						 mgd_sitegroup(mgd_handle()));
	} else {
		where=mgd_format(mgd_handle(),pool,EVENT_MONTH_WHERE,
						 tsstart,tsend,
						 tsstart,tsend,
						 tsstart,tsend,
						 mgd_sitegroup(mgd_handle()));
	}
	php_midgard_select_count_event_in_month(return_value,
      "event", where, &start, &end);
	if(pool!=NULL)
		mgd_free_pool(pool);
}

MGD_FUNCTION(ret_type, copy_event, (type param))
{
    zval **id, **root; int id_r;

    RETVAL_FALSE;
	CHECK_MGD;
    if (ZEND_NUM_ARGS() != 2
	|| zend_get_parameters_ex(2, &id, &root) != SUCCESS)
	WRONG_PARAM_COUNT;
    convert_to_long_ex(id);
    convert_to_long_ex(root);

	/* root must be in same SG or be 0 */
	if ((*root)->value.lval != 0 && !mgd_exists_bool(mgd_handle(), "event src, event tgt",
										"src.id=$d AND tgt.id=$d"
										" AND (src.sitegroup=tgt.sitegroup"
											" OR src.sitegroup=0"
											" OR tgt.sitegroup=0)",
										(*id)->value.lval,(*root)->value.lval)) RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);

    id_r = mgd_copy_event(mgd_handle(),  (*id)->value.lval);
#if HAVE_MIDGARD_QUOTA
	if (mgd_get_quota_error(mgd_handle())) {
	  mgd_set_quota_error(mgd_handle(), 0);
	  RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
	}
#endif /* HAVE_MIDGARD_QUOTA */

    if(id_r) {
      php_midgard_update(return_value, "event", "up=$i", id_r, (*root)->value.lval);
      PHP_UPDATE_REPLIGARD("event",id_r);
    }
}

MGD_MOVE_FUNCTION(event,event,event,up);

MGD_WALK_FUNCTION(event);

MIDGARD_CLASS(MidgardEvent, event, midgardevent, event)

#endif
