/* $Id: host.c 15758 2008-03-18 12:52:39Z piotras $
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

#ifdef PHP_MIDGARD_LEGACY_API

MGD_FUNCTION(ret_type, is_host_owner, (type param))
{
    IDINIT;
    CHECK_MGD;
    RETVAL_LONG(ishostowner(id));
}

MGD_FUNCTION(ret_type, list_hosts, (type param))
{
	zval **id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	case 0:
    	php_midgard_select(&MidgardHost, return_value,
#if ! HAVE_MIDGARD_MULTILANG
		   "id,name,port,online,root,style,owner,info&1 AS auth,prefix," HOSTNAME_FIELD SITEGROUP_SELECT,
#else /* HAVE_MIDGARD_MULTILANG */
		   "id,name,port,online,root,style,owner,info&1 AS auth,prefix,lang," HOSTNAME_FIELD SITEGROUP_SELECT,
#endif /* HAVE_MIDGARD_MULTILANG */
		   "host", NULL, "hostname, online DESC");
		return;

	case 1:
		if (zend_get_parameters_ex(1, &id) == FAILURE) {
		   WRONG_PARAM_COUNT;
		}

		convert_to_long_ex(id);
    	php_midgard_select(&MidgardHost, return_value,
#if ! HAVE_MIDGARD_MULTILANG
			   "id,name,port,online,root,style,owner,info&1 AS auth,prefix," HOSTNAME_FIELD SITEGROUP_SELECT,
#else /* HAVE_MIDGARD_MULTILANG */
			   "id,name,port,online,root,style,owner,info&1 AS auth,prefix,lang," HOSTNAME_FIELD SITEGROUP_SELECT,
#endif /* HAVE_MIDGARD_MULTILANG */
			   "host", "root=$d", "hostname, online DESC", (*id)->value.lval);
		break;

	default:
		WRONG_PARAM_COUNT;
	}
}

MGD_FUNCTION(ret_type, get_host, (type param))
{
	zval **id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	case 0:
	php_midgard_bless(return_value, &MidgardHost);
#if ! HAVE_MIDGARD_MULTILANG
	mgd_object_init(return_value, "name", "port", "online", "root", "style", "auth", "owner", NULL);
#else /* HAVE_MIDGARD_MULTILANG */
	mgd_object_init(return_value, "name", "port", "online", "root", "style", "auth", "owner", "lang", NULL);
#endif /* HAVE_MIDGARD_MULTILANG */
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

   php_midgard_get_object(return_value, MIDGARD_OBJECT_HOST, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, get_host_by_name, (type param))
{
   zval **name, **prefix;
   int objid;

	CHECK_MGD;

   if (ZEND_NUM_ARGS() != 2
         || zend_get_parameters_ex(2, &name, &prefix) != SUCCESS) {
	   WRONG_PARAM_COUNT;
   }

   convert_to_string_ex(name);
   convert_to_string_ex(prefix);

   objid = mgd_exists_id(mgd_handle(), "host", "name=$q AND prefix=$q",
      (*name)->value.str.val, (*prefix)->value.str.val);
   php_midgard_get_object(return_value, MIDGARD_OBJECT_HOST, objid);
}

MGD_FUNCTION(ret_type, create_host, (type param))
{
#if ! HAVE_MIDGARD_MULTILANG
	zval **name, **port, **online, **root, **style, **auth, **owner, **prefix, *self;
#else /* HAVE_MIDGARD_MULTILANG */
	zval **name, **port, **online, **root, **style, **auth, **owner, **prefix, **lang = NULL, *self;
#endif /* HAVE_MIDGARD_MULTILANG */

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "port", port)
		    || !MGD_PROPFIND(self, "online", online)
		    || !MGD_PROPFIND(self, "root", root)
		    || !MGD_PROPFIND(self, "style", style)
		    || !MGD_PROPFIND(self, "auth", auth)
		    || !MGD_PROPFIND(self, "owner", owner)
#if HAVE_MIDGARD_MULTILANG
		    || !MGD_PROPFIND(self, "lang", lang)
#endif /* HAVE_MIDGARD_MULTILANG */
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
		if (!MGD_PROPFIND(self, "prefix", prefix))
			prefix = NULL;
	}
	else {
		switch (ZEND_NUM_ARGS()) {
			case 7:
				if (zend_get_parameters_ex
				    (7, &name, &port, &online, &root,
				     &style, &auth, &owner) != SUCCESS) {
					WRONG_PARAM_COUNT;
            }

				prefix = NULL;
				break;
			case 8:
				if (zend_get_parameters_ex
				    (8, &name, &port, &online, &root,
				     &style, &auth, &owner, &prefix) != SUCCESS) {
					WRONG_PARAM_COUNT;
            }
#if HAVE_MIDGARD_MULTILANG
	    break;
	  case 9:
	    if (zend_get_parameters_ex
		(9, &name, &port, &online, &root,
		 &style, &auth, &owner, &prefix, &lang) != SUCCESS) {
	      WRONG_PARAM_COUNT;
            }
#endif /* HAVE_MIDGARD_MULTILANG */

				break;
			default:
				WRONG_PARAM_COUNT;
		}
	}
	if (prefix)
	convert_to_string_ex(prefix);
	convert_to_string_ex(name);
	convert_to_long_ex(port);
	convert_to_long_ex(online);
	convert_to_long_ex(root);
	convert_to_long_ex(style);
	convert_to_long_ex(auth);
	convert_to_long_ex(owner);
#if HAVE_MIDGARD_MULTILANG
	if (lang) {
	  convert_to_long_ex(lang);
	}
#endif /* HAVE_MIDGARD_MULTILANG */

	if (!mgd_isroot(mgd_handle()))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if ((*root)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "page", "id=$d", (*root)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if ((*style)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "style", "id=$d", (*style)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if ((*owner)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "grp", "id=$d", (*owner)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	php_midgard_create(return_value, self, "host",
#if ! HAVE_MIDGARD_MULTILANG
			   "name,port,online,root,style,info,owner,prefix",
			   "$q,$d,$d,$d,$d,$d,$d,$q", (*name)->value.str.val,
#else /* HAVE_MIDGARD_MULTILANG */
			   "name,port,online,root,style,info,owner,prefix,lang",
			   "$q,$d,$d,$d,$d,$d,$d,$q,$d", (*name)->value.str.val,
#endif /* HAVE_MIDGARD_MULTILANG */
			   (*port)->value.lval, (*online)->value.lval == 1,
			   (*root)->value.lval, (*style)->value.lval,
			   (*auth)->value.lval == 1, (*owner)->value.lval,
#if ! HAVE_MIDGARD_MULTILANG
			   prefix ? (*prefix)->value.str.val : "");
#else /* HAVE_MIDGARD_MULTILANG */
			   prefix ? (*prefix)->value.str.val : "", lang ? (*lang)->value.lval : 0);
#endif /* HAVE_MIDGARD_MULTILANG */

	PHP_CREATE_REPLIGARD("host", return_value->value.lval);
}

MGD_FUNCTION(ret_type, update_host, (type param))
{
#if ! HAVE_MIDGARD_MULTILANG
	zval **id, **name, **port, **online, **root, **style, **auth, **owner, **prefix, *self;
#else /* HAVE_MIDGARD_MULTILANG */
	zval **id, **name, **port, **online, **root, **style, **auth, **owner, **prefix, **lang = NULL, *self;
#endif /* HAVE_MIDGARD_MULTILANG */

	midgard_pool *pool = NULL;
	char *prefix_sql = "";
	char *name_port_sql = "";

	RETVAL_FALSE;
	CHECK_MGD;

	if (!(pool = mgd_alloc_pool()))
		RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "id", id)
		    || !MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "port", port)
		    || !MGD_PROPFIND(self, "online", online)
		    || !MGD_PROPFIND(self, "root", root)
		    || !MGD_PROPFIND(self, "style", style)
		    || !MGD_PROPFIND(self, "auth", auth)
		    || !MGD_PROPFIND(self, "owner", owner)
#if HAVE_MIDGARD_MULTILANG
		    || !MGD_PROPFIND(self, "lang", lang)
#endif /* HAVE_MIDGARD_MULTILANG */
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}

		if (
			   !mgd_isroot(mgd_handle()) ||
			   !MGD_PROPFIND(self, "prefix", prefix))
			prefix = NULL;
	}
	else {
		switch (ZEND_NUM_ARGS()) {
			case 8:
				if (zend_get_parameters_ex
				    (8, &id, &name, &port, &online, &root,
				     &style, &auth, &owner) != SUCCESS) {
					WRONG_PARAM_COUNT;
            }

				prefix = NULL;
				break;
			case 9:
				if (zend_get_parameters_ex
				    (9, &id, &name, &port, &online, &root,
				     &style, &auth, &owner, &prefix) != SUCCESS) {
					WRONG_PARAM_COUNT;
            }

				if (mgd_isroot(mgd_handle())) {
					convert_to_string_ex(prefix);
				} else
					prefix = NULL;
				break;
#if HAVE_MIDGARD_MULTILANG
			case 10:
				if (zend_get_parameters_ex
				    (10, &id, &name, &port, &online, &root,
				     &style, &auth, &owner, &prefix, &lang) != SUCCESS) {
					WRONG_PARAM_COUNT;
            }

				if (mgd_isroot(mgd_handle())) {
					convert_to_string_ex(prefix);
				} else
					prefix = NULL;
				break;
#endif /* HAVE_MIDGARD_MULTILANG */
			default:
				WRONG_PARAM_COUNT;
		}
	}

	if (prefix) convert_to_string_ex(prefix);
	convert_to_long_ex(id);
	convert_to_string_ex(name);
	convert_to_long_ex(port);
	convert_to_long_ex(online);
	convert_to_long_ex(root);
	convert_to_long_ex(style);
	convert_to_long_ex(auth);
	convert_to_long_ex(owner);
#if HAVE_MIDGARD_MULTILANG
	if (lang) {
	  convert_to_long_ex(lang);
	}
#endif /* HAVE_MIDGARD_MULTILANG */

	if (!ishostowner((*id)->value.lval)) {
		mgd_free_pool(pool);
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	}

	if (prefix != NULL)
		prefix_sql =
		   mgd_format(mgd_handle(), pool, ",prefix=$q",
			      (*prefix)->value.str.val);

	if (mgd_isroot(mgd_handle()))
		name_port_sql =
		   mgd_format(mgd_handle(), pool, ",name=$q,port=$d",
			      (*name)->value.str.val, (*port)->value.lval);

	if ((*root)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "page", "id=$d", (*root)->value.lval)) {
		mgd_free_pool(pool);
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
   }

	if ((*style)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "style", "id=$d", (*style)->value.lval)) {
		mgd_free_pool(pool);
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
   }

	if ((*owner)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "grp", "id=$d", (*owner)->value.lval)) {
		mgd_free_pool(pool);
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
   }

	php_midgard_update(return_value, "host",
			   "online=$d,root=$d,"
#if ! HAVE_MIDGARD_MULTILANG
			   "style=$d,info=$d,owner=$d $s $s",
#else /* HAVE_MIDGARD_MULTILANG */
  "style=$d,info=$d,owner=$d,lang=$d $s $s",
#endif /* HAVE_MIDGARD_MULTILANG */
			   (*id)->value.lval,
			   (*online)->value.lval == 1,
			   (*root)->value.lval, (*style)->value.lval,
#if ! HAVE_MIDGARD_MULTILANG
			   (*auth)->value.lval == 1, (*owner)->value.lval,
#else /* HAVE_MIDGARD_MULTILANG */
 (*auth)->value.lval == 1, (*owner)->value.lval, (lang)?(*lang)->value.lval:0,
#endif /* HAVE_MIDGARD_MULTILANG */
			   prefix_sql, name_port_sql);
	PHP_UPDATE_REPLIGARD("host", (*id)->value.lval);
	mgd_free_pool(pool);
TOUCH_CACHE;
}

MGD_FUNCTION(ret_type, delete_host, (type param))
{
    IDINIT;
	CHECK_MGD;
	if(mgd_has_dependants(mgd_handle(),id,"host"))
		RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

	if (!mgd_isroot(mgd_handle()))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	php_midgard_delete(return_value, "host", id);
	PHP_DELETE_REPLIGARD("host", id);
}

MIDGARD_CLASS(MidgardHost, host, midgardhost, host)

#endif
