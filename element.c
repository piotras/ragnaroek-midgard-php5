/* $Id: element.c 10324 2006-11-27 12:47:04Z piotras $
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
#include "mgd_access.h"
#include "mgd_oop.h"
#include <midgard/pageresolve.h>

#ifdef PHP_MIDGARD_LEGACY_API

#if HAVE_MIDGARD_MULTILANG
void php_midgard_create_element_content_internal(zval *return_value, int newid, const char* value, int lang) {

  php_midgard_create(return_value, 0, "element_i",
		     "sid,value,lang",
		     "$d,$q,$d",
		     newid, value, lang);
  PHP_CREATE_REPLIGARD("element_i", return_value->value.lval);
}

void php_midgard_update_element_content_internal(zval *return_value, int i_id,  const char* value) {

  php_midgard_update(return_value, "element_i",
		     "value=$q",
		     i_id,
		     value);
  PHP_UPDATE_REPLIGARD("element_i", i_id);
}


#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(ret_type, list_elements, (type param))
{
	IDINIT;
	CHECK_MGD;
	php_midgard_select(&MidgardElement, return_value,
#if ! HAVE_MIDGARD_MULTILANG
			   "id,name" SITEGROUP_SELECT, "element", "style=$d", "name", id);
}
#else /* HAVE_MIDGARD_MULTILANG */
			   "id,name,element.sitegroup as sitegroup", "element", "style=$d", "name", id);}
#endif /* HAVE_MIDGARD_MULTILANG */

MGD_FUNCTION(ret_type, get_element, (type param))
{
	zval **id;
	zval **style, **name;
   int objid;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
		case 0:
			php_midgard_bless(return_value, &MidgardElement);
			mgd_object_init(return_value, "id", "style", "name",
#if ! HAVE_MIDGARD_MULTILANG
					 "value", NULL);
#else /* HAVE_MIDGARD_MULTILANG */
					 "value", "lang" ,NULL);
#endif /* HAVE_MIDGARD_MULTILANG */
			return;
		case 1:
			if (zend_get_parameters_ex(1, &id) != SUCCESS) {
				WRONG_PARAM_COUNT;
			}
			convert_to_long_ex(id);

         php_midgard_get_object(return_value, MIDGARD_OBJECT_ELEMENT, (*id)->value.lval);
			break;
		case 2:
			if (zend_get_parameters_ex(2, &style, &name) != SUCCESS) {
				WRONG_PARAM_COUNT;
			}
			convert_to_long_ex(style);
			convert_to_string_ex(name);
         objid = mgd_exists_id(mgd_handle(), "element", "style=$d AND name=$q",
            (*style)->value.lval, (*name)->value.str.val);
         php_midgard_get_object(return_value, MIDGARD_OBJECT_ELEMENT, objid);
			break;
		default:
			WRONG_PARAM_COUNT;
	}
}

MGD_FUNCTION(ret_type, create_element, (type param))
{
	zval **style, **name, **value, *self;
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
		if (!MGD_PROPFIND(self, "style", style)
		    || !MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "value", value)) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 3
		    || zend_get_parameters_ex(3, &style, &name,
					      &value) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
	}

	convert_to_long_ex(style);
	convert_to_string_ex(name);
	convert_to_string_ex(value);

	if (!(*style)->value.lval
	    || !mgd_exists_id(mgd_handle(), "style", "id=$d",
			   (*style)->value.lval))
		   RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if (!isstyleowner((*style)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

   if ((*name)->value.str.val[0] == '\0') {
		WRONG_PARAM_COUNT;
   }

	if (mgd_exists_id(mgd_handle(), "element", "style=$d AND name=$q",
		       (*style)->value.lval, (*name)->value.str.val))
		RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

	php_midgard_create(return_value, self,
#if ! HAVE_MIDGARD_MULTILANG
			   "element", "style,name,value",
			   "$d,$q,$q", (*style)->value.lval,
			   (*name)->value.str.val, (*value)->value.str.val);
#else /* HAVE_MIDGARD_MULTILANG */
		     "element", "style,name",
		     "$d,$q", (*style)->value.lval,
		     (*name)->value.str.val);
#endif /* HAVE_MIDGARD_MULTILANG */
		PHP_CREATE_REPLIGARD("element", return_value->value.lval);
#if HAVE_MIDGARD_MULTILANG
		newid = return_value->value.lval;
		if (newid) {
                   if (lang > 0)  {
                      php_midgard_create_element_content_internal(retval_content0, return_value->value.lval, "", 0);
                      if (!(retval_content0->value.lval)) success = 0;
                      else created0 = 1;
                   }
                   if (success) {
                       php_midgard_create_element_content_internal(retval_content, return_value->value.lval, (*value)->value.str.val, lang);
                       if (!(retval_content->value.lval)) success = 0;
                   }
                   if (!success) {
                     if (created0) {
                        php_midgard_delete(retval_content, "element_i", retval_content0->value.lval);
                        PHP_DELETE_REPLIGARD ("element_i", retval_content0->value.lval);
                     }
                     php_midgard_delete(retval_content, "element", newid);
                     PHP_DELETE_REPLIGARD ("element", newid);
                     RETVAL_FALSE_BECAUSE(MGD_ERR_QUOTA);
                   }
                 }

  TOUCH_CACHE;
}

MGD_FUNCTION(ret_type, create_element_content, (type param))
{
  zval **id, **value, *self;
  int lang = mgd_lang(mgd_handle());

  RETVAL_FALSE;
  CHECK_MGD;

  if ((self = getThis()) != NULL) {
    if (!MGD_PROPFIND(self, "id", id)
	|| !MGD_PROPFIND(self, "value", value)) {
      RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
    }
  }
  else {
    if (ZEND_NUM_ARGS() != 2
	|| zend_get_parameters_ex(2, &id, &value) == FAILURE) {
      WRONG_PARAM_COUNT;
    }
  }

  convert_to_string_ex(value);



  if (!isstyleowner(mgd_idfield(mgd_handle(), "style", "element", (*id)->value.lval)))
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

  if (mgd_exists_id(mgd_handle(), "element_i", "sid=$d AND lang=$d",
		    (*id)->value.lval, lang))
	  RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);


  php_midgard_create_element_content_internal(return_value, (*id)->value.lval, (*value)->value.str.val, lang);
#endif /* HAVE_MIDGARD_MULTILANG */

   TOUCH_CACHE;
#if HAVE_MIDGARD_MULTILANG
  RETURN_TRUE;
#endif /* HAVE_MIDGARD_MULTILANG */
}

MGD_FUNCTION(ret_type, update_element, (type param))
{
#if ! HAVE_MIDGARD_MULTILANG
	zval **id, **name, **value, *self;
#else /* HAVE_MIDGARD_MULTILANG */
  zval **id, **name, **value, **lang, *self;
  int i_id, lang_means_something;
  midgard_res *res;
  int lang_i = mgd_lang(mgd_handle());
  lang_means_something = 0;
#endif /* HAVE_MIDGARD_MULTILANG */

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (!MGD_PROPFIND(self, "id", id)
		    || !MGD_PROPFIND(self, "name", name)
#if ! HAVE_MIDGARD_MULTILANG
		    || !MGD_PROPFIND(self, "value", value)) {
#else /* HAVE_MIDGARD_MULTILANG */
		    || !MGD_PROPFIND(self, "value", value)
		    || !MGD_PROPFIND(self, "lang", lang)) {
#endif /* HAVE_MIDGARD_MULTILANG */
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
#if HAVE_MIDGARD_MULTILANG
		lang_means_something = 1;
#endif /* HAVE_MIDGARD_MULTILANG */
	}
	else {
		if (ZEND_NUM_ARGS() != 3
		    || zend_get_parameters_ex(3, &id, &name, &value) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
	}
	convert_to_long_ex(id);
	convert_to_string_ex(name);
	convert_to_string_ex(value);
#if ! HAVE_MIDGARD_MULTILANG

#else /* HAVE_MIDGARD_MULTILANG */
	if (lang_means_something) {
	  convert_to_long_ex(lang);
	  lang_i = (*lang)->value.lval;
	}
#endif /* HAVE_MIDGARD_MULTILANG */
	if (!isstyleowner(mgd_idfield(mgd_handle(), "style", "element",
				      (*id)->value.lval)))
		   RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

   if ((*name)->value.str.val[0] == '\0') {
		WRONG_PARAM_COUNT;
   }

   if (mgd_exists_id(mgd_handle(), "element", "style=$d AND name=$q AND id<>$d",
         mgd_idfield(mgd_handle(), "style", "element", (*id)->value.lval),
         (*name)->value.str.val, (*id)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

#if ! HAVE_MIDGARD_MULTILANG
	php_midgard_update(return_value, "element", "name=$q,value=$q",
#else /* HAVE_MIDGARD_MULTILANG */
	res = mgd_ungrouped_select(mgd_handle(), "id", "element_i", "sid=$d AND lang=$d", NULL, (*id)->value.lval, lang_i);

	if (res && mgd_fetch(res)) {
	  i_id = atol(mgd_colvalue(res, 0));
	} else {
	  RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}


	php_midgard_update(return_value, "element", "name=$q",
#endif /* HAVE_MIDGARD_MULTILANG */
			   (*id)->value.lval,
#if ! HAVE_MIDGARD_MULTILANG
			   (*name)->value.str.val, (*value)->value.str.val);
#else /* HAVE_MIDGARD_MULTILANG */
			   (*name)->value.str.val);
#endif /* HAVE_MIDGARD_MULTILANG */
	PHP_UPDATE_REPLIGARD("element", (*id)->value.lval);
#if HAVE_MIDGARD_MULTILANG
	php_midgard_update_element_content_internal(return_value, i_id, (*value)->value.str.val);
	TOUCH_CACHE;
}




MGD_FUNCTION(ret_type, update_element_content, (type param))
{
  zval **id, **value, **lang, *self;
  midgard_res *res;
  int i_id, lang_means_something, lang_i;
  lang_i = mgd_lang(mgd_handle()); lang_means_something = 0;
  RETVAL_FALSE;
  CHECK_MGD;
#endif /* HAVE_MIDGARD_MULTILANG */

#if HAVE_MIDGARD_MULTILANG
  if ((self = getThis()) != NULL) {
    if (!MGD_PROPFIND(self, "id", id)
	|| !MGD_PROPFIND(self, "value", value)
	|| !MGD_PROPFIND(self, "lang", lang)) {
      RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
    }
    lang_means_something = 1;
  }
  else {
    if (ZEND_NUM_ARGS() != 2
	|| zend_get_parameters_ex(2, &id, &value) == FAILURE) {
      WRONG_PARAM_COUNT;
    }
  }
  convert_to_long_ex(id);
  convert_to_string_ex(value);
  if (lang_means_something) {
    convert_to_long_ex(lang);
    lang_i = (*lang)->value.lval;
  }

  if (!isstyleowner(mgd_idfield(mgd_handle(), "style", "element",
				(*id)->value.lval)))
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

  res = mgd_ungrouped_select(mgd_handle(), "id", "element_i", "sid=$d AND lang=$d", NULL, (*id)->value.lval, lang_i);

  if (res && mgd_fetch(res)) {
    i_id = atol(mgd_colvalue(res, 0));
  } else {
    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
  }

  mgd_release(res);
  php_midgard_update_element_content_internal(return_value, (*id)->value.lval, (*value)->value.str.val);
#endif /* HAVE_MIDGARD_MULTILANG */
   TOUCH_CACHE;
#if HAVE_MIDGARD_MULTILANG
  RETURN_TRUE;
#endif /* HAVE_MIDGARD_MULTILANG */
}

#if HAVE_MIDGARD_MULTILANG

#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(ret_type, delete_element, (type param))
{
#if HAVE_MIDGARD_MULTILANG
  zval *content_return;
  midgard_res *res;
  int i_id;
#endif /* HAVE_MIDGARD_MULTILANG */
	IDINIT;
	CHECK_MGD;
	if (mgd_has_dependants(mgd_handle(), id, "element"))
		RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

	if (!isstyleowner(mgd_idfield(mgd_handle(), "style", "element", id)))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

#if HAVE_MIDGARD_MULTILANG
  /* need to delete all element content */
  res = mgd_ungrouped_select(mgd_handle(), "id", "element_i", "sid=$d", NULL, id);
  if (res) {
    while (mgd_fetch(res)) {
      i_id = atol(mgd_colvalue(res,0));
      MAKE_STD_ZVAL(content_return);
      php_midgard_delete(content_return, "element_i", i_id);
      PHP_DELETE_REPLIGARD("element_i", i_id);
    }
  }


#endif /* HAVE_MIDGARD_MULTILANG */
	php_midgard_delete(return_value, "element", id);
	PHP_DELETE_REPLIGARD("element", id);

   TOUCH_CACHE;
}

#if HAVE_MIDGARD_MULTILANG
MGD_FUNCTION(ret_type, delete_element_content, (type param))
{
  zval **lang;
  midgard_res *res;
  int i_id, lang_i;
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

  if (!isstyleowner(mgd_idfield(mgd_handle(), "style", "element", id)))
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

  res = mgd_ungrouped_select(mgd_handle(), "id", "element_i", "sid=$d AND lang=$d", NULL, id, lang_i);

  if (res && mgd_fetch(res)) {
    i_id = atol(mgd_colvalue(res, 0));
  } else {
    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
  }

  mgd_release(res);
  php_midgard_delete(return_value, "element_i", i_id);

  PHP_DELETE_REPLIGARD("element_i", i_id);

  TOUCH_CACHE;
}

#endif /* HAVE_MIDGARD_MULTILANG */

MGD_FUNCTION(bool, delete_element_tree, (int id))
{
  IDINIT;
  CHECK_MGD;

  if (!isstyleowner(mgd_idfield(mgd_handle(), "style", "element", id)))
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

  if (mgd_delete_element(mgd_handle(), id)) {
    TOUCH_CACHE;
    RETURN_TRUE;
  }

  TOUCH_CACHE;
  RETURN_FALSE;
}


MGD_FUNCTION(ret_type, copy_element, (type param))
{
  zval **id, **newstyle;
  int r_id;
  RETVAL_FALSE;
  CHECK_MGD;
  switch (ZEND_NUM_ARGS()) {
  case 2:
    if (zend_get_parameters_ex(2, &id, &newstyle) !=
	SUCCESS) WRONG_PARAM_COUNT;
    break;
  case 1:
    if (zend_get_parameters_ex(1, &id) != SUCCESS)
      WRONG_PARAM_COUNT;
    newstyle = NULL;
    break;
  default:
    WRONG_PARAM_COUNT;
  }

  convert_to_long_ex(id);
  if (newstyle)
    convert_to_long_ex(newstyle);

  /* newstyle must be in same SG or be 0 */
  if (newstyle && !mgd_exists_bool(mgd_handle(), "style,element",
				   "style.id=$d AND element.id=$d"
				   " AND (style.sitegroup=element.sitegroup"
				   " OR style.sitegroup=0"
				   " OR element.sitegroup=0)",
				   (*newstyle)->value.lval, (*id)->value.lval))
    RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);

	r_id = mgd_copy_element(mgd_handle(), (*id)->value.lval,
			       newstyle ? (*newstyle)->value.lval : 0);
#if HAVE_MIDGARD_QUOTA
	if (mgd_get_quota_error(mgd_handle())) {
	  mgd_set_quota_error(mgd_handle(), 0);
	  RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
	}
#endif /* HAVE_MIDGARD_QUOTA */
	RETVAL_LONG(r_id);

	TOUCH_CACHE;
}

MGD_MOVE_AND_TOUCH(element, style, element, style, 1)

#if ! HAVE_MIDGARD_MULTILANG
MIDGARD_CLASS(MidgardElement, element, midgardelement, element)
#else /* HAVE_MIDGARD_MULTILANG */
MIDGARD_CLASS_I18N(MidgardElement, element, midgardelement, element)
#endif /* HAVE_MIDGARD_MULTILANG */


#endif
