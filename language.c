/* $Id: language.c 10324 2006-11-27 12:47:04Z piotras $
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

MGD_FUNCTION(ret_type, has_multilang, (type param))
{
#if HAVE_MIDGARD_MULTILANG
	RETURN_TRUE;
#else
	RETURN_FALSE;
#endif
}

#if HAVE_MIDGARD_MULTILANG 
MGD_FUNCTION(ret_type, list_languages, (type param))
{
  CHECK_MGD;
  php_midgard_select(&MidgardLanguage, return_value,
		     "id,code,name" SITEGROUP_SELECT, "language", NULL, NULL);
}

MGD_FUNCTION(ret_type, get_language, (type param))
{
  zval **id;
  CHECK_MGD;
  
  switch (ZEND_NUM_ARGS()) {
  case 0:
    php_midgard_bless(return_value, &MidgardLanguage);
    mgd_object_init(return_value, "id", "code", "name", NULL);
    return;
  case 1:
    if (zend_get_parameters_ex(1, &id) != SUCCESS) {
      WRONG_PARAM_COUNT;
    }
    convert_to_long_ex(id);
			
    php_midgard_get_object(return_value, MIDGARD_OBJECT_LANGUAGE, (*id)->value.lval);
    break;
  default:
    WRONG_PARAM_COUNT;
  }
}


MGD_FUNCTION(ret_type, get_language_by_code, (type param))
{
  zval **code;
  int objid;
  CHECK_MGD;

  if (ZEND_NUM_ARGS() != 1
      || zend_get_parameters_ex(1, &code) == FAILURE) {
    WRONG_PARAM_COUNT;
  }
  convert_to_string_ex(code);
  objid = mgd_exists_id(mgd_handle(), "language", "code=$q",
			(*code)->value.str.val);
  php_midgard_get_object(return_value, MIDGARD_OBJECT_LANGUAGE, objid);
}


MGD_FUNCTION(ret_type, create_language, (type param))
{
  zval **code, **name, *self;

  RETVAL_FALSE;
  CHECK_MGD;

  if ((self = getThis()) != NULL) {
    if (!MGD_PROPFIND(self, "code", code)
	|| !MGD_PROPFIND(self, "name", name)) {
      RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
    }
  }
  else {
    if (ZEND_NUM_ARGS() != 2
	|| zend_get_parameters_ex(2, &code, &name) == FAILURE) {
      WRONG_PARAM_COUNT;
    }
  }
  
  convert_to_string_ex(code);
  convert_to_string_ex(name);

  if (!mgd_isadmin(mgd_handle())) {
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
  }
	
  if (strlen((*code)->value.str.val) != 2) {
    RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_NAME);
    WRONG_PARAM_COUNT;
  }

  if (mgd_exists_id(mgd_handle(), "language", "code=$q",
		    (*code)->value.str.val))
    RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

  php_midgard_create(return_value, self,
		     "language", "code, name",
		     "$q,$q", 
		     (*code)->value.str.val, (*name)->value.str.val);
  PHP_CREATE_REPLIGARD("language", return_value->value.lval);
  TOUCH_CACHE;
}

MGD_FUNCTION(ret_type, update_language, (type param))
{
  zval **id, **code, **name, *self;
  int objid;
  
  RETVAL_FALSE;
  CHECK_MGD;

  if ((self = getThis()) != NULL) {
    if (!MGD_PROPFIND(self, "id", id)
	|| !MGD_PROPFIND(self, "code", code)
	|| !MGD_PROPFIND(self, "name", name)) {
      RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
    }
  }
  else {
    if (ZEND_NUM_ARGS() != 3
	|| zend_get_parameters_ex(3, &id, &code, &name) == FAILURE) {
      WRONG_PARAM_COUNT;
    }
  }
  convert_to_long_ex(id);
  convert_to_string_ex(code);
  convert_to_string_ex(name);
	
  if (!mgd_isadmin(mgd_handle())) {
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
  }

  if ((*code)->value.str.val[0] == '\0') {
    WRONG_PARAM_COUNT;
  }
	
  objid = mgd_exists_id(mgd_handle(), "language", "code=$q", (*code)->value.str.val);

  if (objid != 0 && objid != (*id)->value.lval)
    RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

  php_midgard_update(return_value, "language", "code=$q, name=$q",
		     (*id)->value.lval,
		     (*code)->value.str.val, (*name)->value.str.val);
  PHP_UPDATE_REPLIGARD("language", (*id)->value.lval);
  TOUCH_CACHE;
}

MGD_FUNCTION(ret_type, delete_language, (type param))
{
  IDINIT;
  CHECK_MGD;

  if (!mgd_isadmin(mgd_handle())) { 
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
  }

  php_midgard_delete(return_value, "language", id);
  PHP_DELETE_REPLIGARD("language", id);

  TOUCH_CACHE;
}

MIDGARD_CLASS(MidgardLanguage, language, midgardlanguage, language)

#endif /* HAVE_MIDGARD_MULTILANG */

#endif
