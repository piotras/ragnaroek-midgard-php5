/* $Id: pageelement.c 10324 2006-11-27 12:47:04Z piotras $
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

#include <midgard/tablenames.h>
#include <midgard/pageresolve.h>

#ifdef PHP_MIDGARD_LEGACY_API

#if HAVE_MIDGARD_MULTILANG
void php_midgard_create_pageelement_content_internal(zval *return_value, int newid, const char *value, int lang) {

  php_midgard_create(return_value, 0, "pageelement_i",
		     "sid,value,lang",
		     "$d,$q,$d",
		     newid, value, lang);
  PHP_CREATE_REPLIGARD("pageelement_i", return_value->value.lval);
}

void php_midgard_update_pageelement_content_internal(zval *return_value, int i_id, const char *value) {
  php_midgard_update(return_value, "pageelement_i",
		       "value=$q",
		       i_id,
		       value);
  PHP_UPDATE_REPLIGARD("pageelement_i", i_id);
}

#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(ret_type, list_page_elements, (type param))
{
    IDINIT;
	CHECK_MGD;
#if ! HAVE_MIDGARD_MULTILANG
    php_midgard_select(&MidgardPageElement, return_value, "id,name" SITEGROUP_SELECT, "pageelement", "page=$d", "name", id);
#else /* HAVE_MIDGARD_MULTILANG */
    php_midgard_select(&MidgardPageElement, return_value, "id,name,pageelement.sitegroup as sitegroup", "pageelement", "page=$d", "name", id);
#endif /* HAVE_MIDGARD_MULTILANG */
}

MGD_FUNCTION(ret_type, get_page_element, (type param))
{
	zval **id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	case 0:
		php_midgard_bless(return_value, &MidgardPageElement);
#if ! HAVE_MIDGARD_MULTILANG
		mgd_object_init(return_value, "page", "name", "value", "inherit", NULL);
#else /* HAVE_MIDGARD_MULTILANG */
		mgd_object_init(return_value, "page", "name", "value", "inherit", "lang", NULL);
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
   php_midgard_get_object(return_value, MIDGARD_OBJECT_PAGEELEMENT, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, get_page_element_by_name, (type param))
{
   zval **page, **name;
   int objid;

   CHECK_MGD;

   if (ZEND_NUM_ARGS() != 2
         || zend_get_parameters_ex(2, &page, &name) != SUCCESS) {
      WRONG_PARAM_COUNT;
   }

   convert_to_long_ex(page);
   convert_to_string_ex(name);
   objid = mgd_exists_id(mgd_handle(), "pageelement", "page=$d AND name=$q",
      (*page)->value.lval, (*name)->value.str.val);
   php_midgard_get_object(return_value, MIDGARD_OBJECT_PAGEELEMENT, objid);
}

MGD_FUNCTION(ret_type, create_page_element, (type param))
{
   zval **page, **name, **value, **inherit, *self;
#if HAVE_MIDGARD_MULTILANG

   zval *retval_content0, *retval_content;
   int success = 1, created0 = 0, newid;
   int lang = mgd_lang(mgd_handle());
   MAKE_STD_ZVAL(retval_content0);
   MAKE_STD_ZVAL(retval_content);
#endif /* HAVE_MIDGARD_MULTILANG */
	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "page", page)
		    || !MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "value", value)
		    || !MGD_PROPFIND(self, "inherit", inherit)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 4
		    || zend_get_parameters_ex(4, &page, &name, &value,
				     &inherit) != SUCCESS) WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(page);
	convert_to_string_ex(name);
	convert_to_string_ex(value);
	convert_to_long_ex(inherit);

	if (!(*page)->value.lval
            || !mgd_exists_id(mgd_handle(), "page", "id=$d", (*page)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

   if ((*name)->value.str.val[0] == '\0') {
		WRONG_PARAM_COUNT;
   }

#if ! HAVE_MIDGARD_MULTILANG
	if (!mgd_global_is_owner(mgd_handle(), MIDGARD_OBJECT_PAGE, (*page)->value.lval))
#else /* HAVE_MIDGARD_MULTILANG */
  if (!mgd_global_is_owner_lang(mgd_handle(), MIDGARD_OBJECT_PAGE, (*page)->value.lval, 1, lang))
#endif /* HAVE_MIDGARD_MULTILANG */
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if (mgd_exists_id(mgd_handle(), "pageelement", "page=$d AND name=$q",
			  (*page)->value.lval, (*name)->value.str.val))
		RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

#if ! HAVE_MIDGARD_MULTILANG
	php_midgard_create(return_value, self, "pageelement", "page,name,value,info",
			   "$d,$q,$q,$d", (*page)->value.lval, (*name)->value.str.val,
			   (*value)->value.str.val, ((*inherit)->value.lval != 0));
#else /* HAVE_MIDGARD_MULTILANG */
  php_midgard_create(return_value, self, "pageelement", "page,name,info",
		     "$d,$q,$d", (*page)->value.lval, (*name)->value.str.val,
		     ((*inherit)->value.lval != 0));
#endif /* HAVE_MIDGARD_MULTILANG */

	PHP_CREATE_REPLIGARD("pageelement", return_value->value.lval);

#if HAVE_MIDGARD_MULTILANG
	newid = return_value->value.lval;
	if (newid) {
	  if (lang > 0) {
	    php_midgard_create_pageelement_content_internal(retval_content0, newid, "", 0);
	    if (!(retval_content0->value.lval)) success = 0;
	    else created0 = 1;
	  }
	  if (success) {
            php_midgard_create_pageelement_content_internal(retval_content, newid, (*value)->value.str.val, lang);
	    if (!(retval_content->value.lval)) success = 0;
	  }
	  if (!success) {
	    if (created0) {
	      php_midgard_delete(retval_content, "pageelement_i", retval_content0->value.lval);
	      PHP_DELETE_REPLIGARD ("pageelement_i", retval_content0->value.lval);
	    }
	    php_midgard_delete(retval_content, "pageelement", newid);
	    PHP_DELETE_REPLIGARD ("pageelement", newid);
	    RETVAL_FALSE_BECAUSE(MGD_ERR_QUOTA);
	  }
	}

   TOUCH_CACHE;
}

MGD_FUNCTION(ret_type, create_page_element_content, (type param))
{
   zval **id, **value, *self;
   int lang = mgd_lang(mgd_handle());
	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "id", id)
		    || !MGD_PROPFIND(self, "value", value)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
	  if (ZEND_NUM_ARGS() != 2
	      || zend_get_parameters_ex(2, &id, &value) != SUCCESS) WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(id);
	convert_to_string_ex(value);



	if (!ispagecontentowner(mgd_idfield(mgd_handle(), "page", "pageelement", (*id)->value.lval), lang))
	  RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if (!mgd_exists_id(mgd_handle(), "pageelement", "id=$d", (*id)->value.lval))
	  RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if (mgd_exists_id(mgd_handle(), "pageelement_i", "sid=$d AND lang=$d",
			  (*id)->value.lval, lang))
	  RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

	php_midgard_create_pageelement_content_internal(return_value, (*id)->value.lval, (*value)->value.str.val, lang);

#endif /* HAVE_MIDGARD_MULTILANG */
   TOUCH_CACHE;
#if HAVE_MIDGARD_MULTILANG
	RETURN_TRUE;
#endif /* HAVE_MIDGARD_MULTILANG */
}

MGD_FUNCTION(ret_type, update_page_element, (type param))
{
#if ! HAVE_MIDGARD_MULTILANG
	zval **id, **name, **value, **inherit, *self;
#else /* HAVE_MIDGARD_MULTILANG */
	zval **id, **name, **value, **inherit, **lang, *self;

	midgard_res *res;
	int i_id, lang_means_something;
	int lang_i = mgd_lang(mgd_handle());
	lang_means_something = 0;
#endif /* HAVE_MIDGARD_MULTILANG */

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "id", id)
		    || !MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "value", value)
#if HAVE_MIDGARD_MULTILANG
		    || !MGD_PROPFIND(self, "lang", lang)
#endif /* HAVE_MIDGARD_MULTILANG */
		    || !MGD_PROPFIND(self, "inherit", inherit)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
#if ! HAVE_MIDGARD_MULTILANG
	}
	else {
#else /* HAVE_MIDGARD_MULTILANG */
		lang_means_something = 1;
	} else {
#endif /* HAVE_MIDGARD_MULTILANG */
		if (ZEND_NUM_ARGS() != 4
		    || zend_get_parameters_ex(4, &id, &name, &value,
				     &inherit) != SUCCESS) WRONG_PARAM_COUNT;
	}
	convert_to_long_ex(id);
	convert_to_string_ex(name);
	convert_to_string_ex(value);
	convert_to_long_ex(inherit);

#if ! HAVE_MIDGARD_MULTILANG
	if (!mgd_global_is_owner(mgd_handle(), MIDGARD_OBJECT_PAGEELEMENT, (*id)->value.lval))
#else /* HAVE_MIDGARD_MULTILANG */
	if (lang_means_something) {
	  convert_to_long_ex(lang);
	  lang_i = (*lang)->value.lval;
	}

	if (!mgd_global_is_owner_lang(mgd_handle(), MIDGARD_OBJECT_PAGEELEMENT, (*id)->value.lval, 1, lang_i))
#endif /* HAVE_MIDGARD_MULTILANG */
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

   if ((*name)->value.str.val[0] == '\0') {
		WRONG_PARAM_COUNT;
   }

   if (mgd_exists_id(mgd_handle(), "pageelement", "page=$d AND name=$q AND id<>$d",
         mgd_idfield(mgd_handle(), "page", "pageelement", (*id)->value.lval),
         (*name)->value.str.val, (*id)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

#if HAVE_MIDGARD_MULTILANG
   res = mgd_ungrouped_select(mgd_handle(), "id", "pageelement_i", "sid=$d AND lang=$d", NULL, (*id)->value.lval, lang_i);

   if (res && mgd_fetch(res)) {
     i_id = atol(mgd_colvalue(res, 0));
   } else {
     RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
   }

#endif /* HAVE_MIDGARD_MULTILANG */
	php_midgard_update(return_value, "pageelement",
#if ! HAVE_MIDGARD_MULTILANG
			   "name=$q,value=$q,info=$d", (*id)->value.lval,
			   (*name)->value.str.val, (*value)->value.str.val,
#else /* HAVE_MIDGARD_MULTILANG */
		      "name=$q,info=$d", (*id)->value.lval,
		      (*name)->value.str.val,
#endif /* HAVE_MIDGARD_MULTILANG */
			   ((*inherit)->value.lval != 0));
	PHP_UPDATE_REPLIGARD("pageelement", (*id)->value.lval);

#if HAVE_MIDGARD_MULTILANG
   php_midgard_update_pageelement_content_internal(return_value, i_id, (*value)->value.str.val);
   TOUCH_CACHE;
}

MGD_FUNCTION(ret_type, update_page_element_content, (type param))
{
  zval **id, **value, **lang, *self;

  midgard_res *res;
  int i_id, lang_means_something;
  int lang_i = mgd_lang(mgd_handle());
  lang_means_something = 0;

  RETVAL_FALSE;
  CHECK_MGD;

  if ((self = getThis()) != NULL) {
    if (ZEND_NUM_ARGS() != 0) {
      WRONG_PARAM_COUNT;
    }

    if (!MGD_PROPFIND(self, "id", id)
	|| !MGD_PROPFIND(self, "value", value)
	|| !MGD_PROPFIND(self, "lang", lang)
	) {
      RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
    }
    lang_means_something = 1;
  } else {
    if (ZEND_NUM_ARGS() != 2
	|| zend_get_parameters_ex(2, &id, &value) != SUCCESS) WRONG_PARAM_COUNT;
  }

  convert_to_long_ex(id);
  convert_to_string_ex(value);
  if (lang_means_something) {
    convert_to_long_ex(lang);
    lang_i = (*lang)->value.lval;
  }

  if (!ispagecontentowner(mgd_idfield(mgd_handle(), "page", "pageelement", (*id)->value.lval), lang_i))
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);


  res = mgd_ungrouped_select(mgd_handle(), "id", "pageelement_i", "sid=$d AND lang=$d", NULL, (*id)->value.lval, lang_i);

  if (res && mgd_fetch(res)) {
    i_id = atol(mgd_colvalue(res, 0));
  } else {
    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
  }

  php_midgard_update_pageelement_content_internal(return_value, i_id, (*value)->value.str.val);
#endif /* HAVE_MIDGARD_MULTILANG */
   TOUCH_CACHE;
#if HAVE_MIDGARD_MULTILANG
  RETURN_TRUE;
#endif /* HAVE_MIDGARD_MULTILANG */
}

#if HAVE_MIDGARD_MULTILANG

#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(ret_type, delete_page_element, (type param))
{
#if HAVE_MIDGARD_MULTILANG
  zval *content_return;
  midgard_res *res;
  int lang, i_id;
#endif /* HAVE_MIDGARD_MULTILANG */
    IDINIT;
	CHECK_MGD;
    if(mgd_has_dependants(mgd_handle(),id,"pageelement"))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

#if ! HAVE_MIDGARD_MULTILANG
  if (!mgd_global_is_owner(mgd_handle(), MIDGARD_OBJECT_PAGEELEMENT, id))
#else /* HAVE_MIDGARD_MULTILANG */
    /* need to be owner of all pageelement content */
    res = mgd_ungrouped_select(mgd_handle(), "lang", "pageelement_i", "sid=$d", NULL, id);
  if (res) {
    while (mgd_fetch(res)) {
      lang = atol(mgd_colvalue(res,0));
      if (!mgd_global_is_owner_lang(mgd_handle(), MIDGARD_OBJECT_PAGEELEMENT, id, 1, lang))
#endif /* HAVE_MIDGARD_MULTILANG */
	RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
#if HAVE_MIDGARD_MULTILANG
    }
  }

  mgd_release(res);
  /* need to delete all pageelement content */
  res = mgd_ungrouped_select(mgd_handle(), "id", "pageelement_i", "sid=$d", NULL, id);
  if (res) {
    while (mgd_fetch(res)) {
      i_id = atol(mgd_colvalue(res,0));
      MAKE_STD_ZVAL(content_return);
      php_midgard_delete(content_return, "pageelement_i", i_id);
      PHP_DELETE_REPLIGARD("pageelement_i", i_id);
    }
  }
#endif /* HAVE_MIDGARD_MULTILANG */

#if HAVE_MIDGARD_MULTILANG
  mgd_release(res);
#endif /* HAVE_MIDGARD_MULTILANG */
  php_midgard_delete(return_value, "pageelement", id);
  PHP_DELETE_REPLIGARD("pageelement", id);

#if HAVE_MIDGARD_MULTILANG

  TOUCH_CACHE;
}

MGD_FUNCTION(ret_type, delete_page_element_content, (type param))
{
  zval **lang;
  int i_id, lang_i;
  midgard_res *res;
  IDINIT;
  CHECK_MGD;

  /* need lang field because it may not be the same as current lang */
  if ((self = getThis()) != NULL) {
    if (ZEND_NUM_ARGS() != 0) {
      WRONG_PARAM_COUNT;
    }
    if (!MGD_PROPFIND(self, "lang", lang)) {
      RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
    }
    lang_i = (*lang)->value.lval;
  } else {
    lang_i = mgd_lang(mgd_handle());
  }

  res = mgd_ungrouped_select(mgd_handle(), "id", "pageelement_i", "sid=$d AND lang=$d", NULL, id, lang_i);

  if (res && mgd_fetch(res)) {
    i_id = atol(mgd_colvalue(res, 0));
  } else {
    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
  }

  mgd_release(res);

  /* only content owner required */
  if (!ispagecontentowner(mgd_idfield(mgd_handle(), "page", "pageelement", i_id), lang_i))
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

  php_midgard_delete(return_value, "pageelement_i", i_id);
  PHP_DELETE_REPLIGARD("pageelement_i", i_id);

#endif /* HAVE_MIDGARD_MULTILANG */
  TOUCH_CACHE;
}


MGD_FUNCTION(bool, delete_page_element_tree, (int id))
{
  midgard_res *res;
  int lang;
  IDINIT;
  CHECK_MGD;

#if ! HAVE_MIDGARD_MULTILANG
  if (!mgd_global_is_owner(mgd_handle(), MIDGARD_OBJECT_PAGEELEMENT, id))
#else /* HAVE_MIDGARD_MULTILANG */
    /* need to be owner of all pageelement content */
    res = mgd_ungrouped_select(mgd_handle(), "lang", "pageelement_i", "sid=$d", NULL, id);
  if (res) {
    while (mgd_fetch(res)) {
      lang = atol(mgd_colvalue(res,0));
      if (!mgd_global_is_owner_lang(mgd_handle(), MIDGARD_OBJECT_PAGEELEMENT, id, 1, lang))
#endif /* HAVE_MIDGARD_MULTILANG */
	RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
#if HAVE_MIDGARD_MULTILANG
    }
  }
#endif

  if (mgd_delete_page_element(mgd_handle(), id)) {
    TOUCH_CACHE;
    RETURN_TRUE;
  }
  TOUCH_CACHE;
  RETURN_FALSE;
}


MGD_FUNCTION(ret_type, copy_page_element, (type param))
{
  zval **id, **newpage;
  int r_id = 0;
  RETVAL_FALSE;
  CHECK_MGD;
  switch (ZEND_NUM_ARGS()) {
  case 2:
    if (zend_get_parameters_ex(2, &id, &newpage) != SUCCESS)
      WRONG_PARAM_COUNT;
    break;
  case 1:
    if (zend_get_parameters_ex(1, &id) != SUCCESS)
      WRONG_PARAM_COUNT;
    newpage = NULL;
    break;
  default:
    WRONG_PARAM_COUNT;
  }

  convert_to_long_ex(id);
  if(newpage) convert_to_long_ex(newpage);

  /* newpage must be in same SG or be 0 */
  if (newpage && (*newpage)->value.lval != 0
      && !mgd_exists_bool(mgd_handle(), "pageelement pe, page src,page tgt",
			  "pe.id=$d AND src.id=pe.page AND tgt.id=$d"
			  " AND (src.sitegroup=tgt.sitegroup"
			  " OR src.sitegroup=0"
			  " OR tgt.sitegroup=0)",
			  (*id)->value.lval, (*newpage)->value.lval)) RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);


  mgd_copy_page_element(mgd_handle(), (*id)->value.lval,
			newpage ? (*newpage)->value.lval : 0);
#if HAVE_MIDGARD_QUOTA
	if (mgd_get_quota_error(mgd_handle())) {
	  mgd_set_quota_error(mgd_handle(), 0);
	  RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
	}
#endif /* HAVE_MIDGARD_QUOTA */
	RETVAL_LONG (r_id);
  TOUCH_CACHE;
}


MGD_MOVE_AND_TOUCH(pageelement,page,page_element,page,1)

#if ! HAVE_MIDGARD_MULTILANG
MIDGARD_CLASS(MidgardPageElement, pageelement, midgardpageelement, page_element)
#else /* HAVE_MIDGARD_MULTILANG */
MIDGARD_CLASS_I18N(MidgardPageElement, pageelement, midgardpageelement, page_element)
#endif /* HAVE_MIDGARD_MULTILANG */

#endif
