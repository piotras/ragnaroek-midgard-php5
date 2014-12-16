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
	zval *self = getThis();
	char *name;
	int name_length;

	RETVAL_FALSE;
	CHECK_MGD;

	if (!mgd_isroot(mgd_handle()))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_length) == FAILURE) {
		return;
	}

	/* no 'magic' chars in sitegroup name */
	if (strpbrk(name, MIDGARD_LOGIN_RESERVED_CHARS))
		RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_NAME);

	if (mgd_exists_bool(mgd_handle(), "sitegroup", "name=$q", name))
		RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

	php_midgard_create(return_value, self, "sitegroup",
			   "name,admingroup", "$q,$d", name,
			   0);

	PHP_CREATE_REPLIGARD("sitegroup", Z_LVAL_P(return_value));
}

MGD_FUNCTION(ret_type, get_sitegroup, (type param))
{
	long id = 0;
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &id) == FAILURE) {
		return;
	}

	if (id == 0) {
		php_midgard_bless(return_value, &midgardsitegroup);
		mgd_object_init(return_value, "name", "admingroup", "realm", NULL);
		return;
	}

   	php_midgard_get_object(return_value, MIDGARD_OBJECT_SITEGROUP, id);
}

MGD_FUNCTION(ret_type, update_sitegroup, (type param))
{
	zval **zv_id, **zv_name, **zv_admingroup, **zv_realm, *self = getThis();
	int group;
	long id, admingroup;
	char *name = NULL, *realm = NULL;
	int name_length, realm_length;

	RETVAL_FALSE;
	CHECK_MGD;

	if (!mgd_isroot(mgd_handle()))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|lsls",
				&id, &name, &name_length, &admingroup, &realm, &realm_length) == FAILURE) {
		return;
	}

	if (self != NULL) {
		if (!MGD_PROPFIND(self, "id", zv_id)
		    || !MGD_PROPFIND(self, "name", zv_name)
		    || !MGD_PROPFIND(self, "admingroup", zv_admingroup)
		    || !MGD_PROPFIND(self, "realm", zv_realm)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
		id = Z_LVAL_PP(zv_id);
		name = Z_STRVAL_PP(zv_name);
		admingroup = Z_LVAL_PP(zv_admingroup);
		realm = Z_STRVAL_PP(zv_realm);
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lsls",
					&id, &name, &name_length, &admingroup, &realm, &realm_length) == FAILURE) {
			return;
		}
	}

	if (id == 0)
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	group = mgd_exists_id(mgd_handle(), "sitegroup", "name=$q", name);
	if (group != 0 && group != id)
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	/* check admingroup == sitegroup || admingroup == 0 */
	if (admingroup != 0 && 
			!mgd_exists_bool(mgd_handle(), "grp", "id=$d and sitegroup=$d", admingroup, id)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);
	}

	php_midgard_update(return_value, "sitegroup",
			   "name=$q, admingroup=$d, realm=$q",
			   id, name, admingroup, realm);
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

static zend_function_entry midgardsitegroupMethods[] = {
   PHP_FALIAS(midgardsitegroup,  mgd_get_sitegroup,	   NULL)
   PHP_FALIAS(create,	         mgd_create_sitegroup,	NULL)
   PHP_FALIAS(update,	         mgd_update_sitegroup,	NULL)
   PHP_FALIAS(delete,	         mgd_delete_sitegroup,	NULL)
   {  NULL,             NULL,                         NULL}
};

MidgardClass midgardsitegroup = {
   "midgardsitegroup",
   "sitegroup",
   midgardsitegroupMethods,
   {},
   NULL
};


