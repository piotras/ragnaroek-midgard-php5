/* $Id: snippetdir.c 10324 2006-11-27 12:47:04Z piotras $
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

MGD_FUNCTION(ret_type, is_snippetdir_owner, (type param))
{
    IDINIT;
    CHECK_MGD;
    RETVAL_LONG(issnippetdirowner(id));
}

MGD_FUNCTION(ret_type, list_snippetdirs, (type param))
{
    zval **id;

    RETVAL_FALSE;
	CHECK_MGD;
    if (ZEND_NUM_ARGS() != 1
		|| zend_get_parameters_ex(1, &id) != SUCCESS)
		WRONG_PARAM_COUNT;
    convert_to_long_ex(id);

    php_midgard_select(&MidgardSnippetdir, return_value, "id,name" SITEGROUP_SELECT, "snippetdir", "up=$d", "name", (*id)->value.lval);
}

MGD_FUNCTION(ret_type, get_snippetdir, (type param))
{
	zval **id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	case 0:
		php_midgard_bless(return_value, &MidgardSnippetdir);
		mgd_object_init(return_value, "name", "description", "owner", NULL);
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

   php_midgard_get_object(return_value, MIDGARD_OBJECT_SNIPPETDIR, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, get_snippetdir_by_path, (type param))
{
	zval **path;
	int id, up;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	case 0:
		object_init(return_value);
		return;

	case 1:
		if (zend_get_parameters_ex(1, &path) == FAILURE) {
		   WRONG_PARAM_COUNT;
		}

		convert_to_string_ex(path);
		break;

	default:
		WRONG_PARAM_COUNT;
	}

    if(!MGD_PARSE_COMMON_PATH(mgd_handle(), (*path)->value.str.val, "snippetdir", "snippetdir", &id, &up)) {
         php_midgard_get_object(return_value, MIDGARD_OBJECT_SNIPPETDIR, id);
	return;
    }
}

static int snippetdir_owner_id = 0;
static int snippetdir_copy_hook(midgard *mgd, const char *table, const char *name, int up, int id, int flag) {
    if((flag == MIDGARD_PATH_ELEMENT_EXISTS)) {
	return id;
    }
    if(flag == MIDGARD_PATH_ELEMENT_NOTEXISTS) {
    if (!issnippetdirowner(up) ||
	mgd_exists_id(mgd_handle(), "topic", "up=$d AND name=$q", up, name)) return 0;

	/* up must be in same SG or be 0 */
	if (up != 0 && !mgd_exists_id(mgd_handle(), "snippetdir",
			"id=$d",
			up)) return 0;
	/* owner must be in same SG or be 0 */
	/* TODO: should we in fact allow owner == 0 for non-root? */
	if (snippetdir_owner_id != 0 && !mgd_exists_id(mgd_handle(), "grp",
			"id=$d",
			snippetdir_owner_id)) return 0;
        id = mgd_create(mgd, table, "name,up,owner", "$q,$d,$d", name,
			up,snippetdir_owner_id);
    	PHP_CREATE_REPLIGARD_VOID(table,id);
	return id;
    }
    return 0;
}

MGD_FUNCTION(ret_type, create_snippetdir, (type param))
{
	zval **up, **name, **description, **owner, *self;
	int id;

	RETVAL_FALSE;
	CHECK_MGD;

	up = NULL;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "description", description)
		    || !MGD_PROPFIND(self, "owner", owner)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}

		if (!MGD_PROPFIND(self, "up", up))
			up = NULL;
	}
	else {
		switch (ZEND_NUM_ARGS()) {
			case 3:
				if (zend_get_parameters_ex
				    (3, &name, &description,
				     &owner) != SUCCESS) WRONG_PARAM_COUNT;
				break;
			case 4:
				if (zend_get_parameters_ex
				    (4, &up, &name, &description,
				     &owner) != SUCCESS) WRONG_PARAM_COUNT;
				break;
			default:
				WRONG_PARAM_COUNT;
		}
	}

	if (up) convert_to_long_ex(up);
	convert_to_string_ex(name);
	convert_to_string_ex(description);
	convert_to_long_ex(owner);
	snippetdir_owner_id = (*owner)->value.lval;

	if (up) {
		if ((*up)->value.lval != 0
            && !mgd_exists_id(mgd_handle(), "snippetdir", "id=$d", (*up)->value.lval))
         RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

		/* TODO: should we in fact allow owner == 0 for non-root? */
		if ((*owner)->value.lval != 0
            && !mgd_exists_id(mgd_handle(), "grp", "id=$d", (*owner)->value.lval))
         RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

		if (!issnippetdirowner((*up)->value.lval))
         RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

      if (mgd_exists_id(mgd_handle(), "snippetdir",
				     "up=$d AND name=$q", (*up)->value.lval,
				     (*name)->value.str.val))
         RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

		php_midgard_create(return_value, self, "snippetdir",
				   "name,up,owner, description", "$q,$d,$d,$q",
				   (*name)->value.str.val, (*up)->value.lval,
				   snippetdir_owner_id,
				   (*description)->value.str.val);

		PHP_CREATE_REPLIGARD("snippetdir", return_value->value.lval);
	}
	else {
		if (!mgd_parse_path_with_hook
		    (mgd_handle(), (*name)->value.str.val, "snippetdir", NULL, NULL,
		     NULL, NULL, &id, snippetdir_copy_hook)) {

			RETURN_LONG(id);
		}
	}
}

MGD_FUNCTION(ret_type, update_snippetdir, (type param))
{
	zval **id, **name, **description, **owner, *self;

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "id", id)
		    || !MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "description", description)
		    || !MGD_PROPFIND(self, "owner", owner)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 4
		    || zend_get_parameters_ex(4, &id, &name, &description,
				     &owner) != SUCCESS) WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(id);
	convert_to_string_ex(name);
	convert_to_string_ex(description);
	convert_to_long_ex(owner);
	if (!issnippetdirowner((*id)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if (mgd_exists_id
	    (mgd_handle(), "snippetdir", "id!=$d AND up=$d AND name=$q",
	     (*id)->value.lval, mgd_idfield(mgd_handle(), "up", "snippetdir",
					 (*id)->value.lval),
	     (*name)->value.str.val)) RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

	if ((*owner)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "grp", "id=$d", (*owner)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	php_midgard_update(return_value, "snippetdir",
			   "name=$q, owner=$d, description=$q", (*id)->value.lval,
			   (*name)->value.str.val, (*owner)->value.lval,
			   (*description)->value.str.val);
	PHP_UPDATE_REPLIGARD("snippetdir", (*id)->value.lval);
}

MGD_FUNCTION(ret_type, delete_snippetdir, (type param))
{
    IDINIT;
	CHECK_MGD;
    if(mgd_has_dependants(mgd_handle(),id,"snippetdir"))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

    if (!issnippetdirowner(id)) RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

    if (mgd_exists_id(mgd_handle(), "snippetdir", "up=$d", id))
	    RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
    if (mgd_exists_id(mgd_handle(), "snippet", "up=$d", id))
	    RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
    php_midgard_delete(return_value, "snippetdir", id);
    PHP_DELETE_REPLIGARD("snippetdir", id);
}

MGD_FUNCTION(ret_type, delete_snippetdir_tree, (type param))
{
    IDINIT;
    CHECK_MGD;
    RETVAL_FALSE;
    if (!issnippetdirowner(mgd_idfield(mgd_handle(), "up", "snippetdir", id))) RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    if(mgd_delete_snippetdir(mgd_handle(),  id)) { RETVAL_TRUE; }
}

MGD_FUNCTION(ret_type, copy_snippetdir, (int id, string path))
{
    zval **id, **path; int id_r, id_v;

    RETVAL_FALSE;
	CHECK_MGD;
    if (ZEND_NUM_ARGS() != 2
	|| zend_get_parameters_ex(2, &id, &path) != SUCCESS)
	WRONG_PARAM_COUNT;
    convert_to_long_ex(id);
    convert_to_string_ex(path);

    /* Create destination snippetdir */
    if(!mgd_parse_path_with_hook(mgd_handle(), (*path)->value.str.val, "snippetdir", NULL, NULL,
    				NULL, NULL, &id_v, snippetdir_copy_hook)) {
	id_r = mgd_copy_snippetdir(mgd_handle(),  (*id)->value.lval);
#if HAVE_MIDGARD_QUOTA
	if (mgd_get_quota_error(mgd_handle())) {
	  mgd_set_quota_error(mgd_handle(), 0);
	  RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
	}
#endif /* HAVE_MIDGARD_QUOTA */
	if(id_r) {
	    php_midgard_update(return_value, "snippetdir", "up=$i", id_r, id_v);
    	    PHP_UPDATE_REPLIGARD("snippetdir",id_r);
	    RETURN_LONG(id_r);
	}
    }
}

MGD_MOVE_FUNCTION(snippetdir,snippetdir,snippetdir,up);

MGD_WALK_FUNCTION(snippetdir)

MIDGARD_CLASS(MidgardSnippetdir, snippetdir, midgardsnippetdir, snippetdir)

#endif
