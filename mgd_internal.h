/* $Id: mgd_internal.h 27410 2014-09-01 07:39:28Z piotras $
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

#ifdef WIN32
#include "config.w32.h"
#else
#include "config.h"
#endif

#include "php_midgard.h"
#include "mgd_access.h"
#include <midgard/pageresolve.h>

#ifndef HAVE_MGD_INTERNAL_H
#define HAVE_MGD_INTERNAL_H

extern int le_midgard_list_fetch;

#define RETURN_FALSE_BECAUSE(reason) { mgd_set_errno(reason); RETURN_FALSE; }
#define RETVAL_FALSE_BECAUSE(reason) { mgd_set_errno(reason); RETVAL_FALSE; }

#define MGD_FUNCTION(ret, name, param) \
   PHP_FUNCTION(mgd_##name)

#define MGD_FE(name, arg_types) \
   PHP_FE(mgd_##name, arg_types)

#define MGD_FALIAS(name, handler, arg_types) \
   PHP_FALIAS(mgd_##name, _mgd_##handler, arg_types)

#define MGD_PROPFIND(object, prop, retval) \
   (zend_hash_find(Z_OBJPROP_P(object), (prop), strlen(prop)+1, (void**)&(retval)) == SUCCESS)

#define MGD_PROPFIND_CONST(object, prop, retval) \
   (zend_hash_find(Z_OBJPROP_P(object), (prop), sizeof(prop), \
      (void**)&(retval)) == SUCCESS)

#define IDINIT \
   int id; zval *self, **id_zval; \
   long id_value; \
   if (!mgd_handle()) \
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_CONNECTED); \
   if ((self = getThis()) != NULL) { \
      if (! MGD_PROPFIND(self, "id", id_zval)) { \
         RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT); \
      } \
   } else { \
      if (ZEND_NUM_ARGS() != 1 \
            || zend_parse_parameters(1 TSRMLS_CC, "l", &id_value) != SUCCESS) \
      WRONG_PARAM_COUNT; \
   } \
   id = id_value;

#define PHP_CREATE_REPLIGARD(table,id)
#define PHP_CREATE_REPLIGARD_VOID(table,id)

#define PHP_DELETE_REPLIGARD(table,id) \
   { \
      if(id != 0) DELETE_REPLIGARD(mgd_handle(), table, id) \
      else RETURN_FALSE_BECAUSE(MGD_ERR_ERROR); \
   }

#define PHP_UPDATE_REPLIGARD(table,id) \
   UPDATE_REPLIGARD(mgd_handle(), table, id)

/* Commonly used macros
*/

#define SITEGROUP_SELECT ",sitegroup"

#define MGD_INIT_CLASS_ENTRY(class_container, class_name, functions) \
	{ \
		class_container.name = strdup(class_name); \
		class_container.name_length = strlen(class_name); \
		class_container.builtin_functions = functions; \
		class_container.handle_function_call = NULL; \
		class_container.handle_property_get = NULL; \
		class_container.handle_property_set = NULL; \
	}

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
#define MGD_INIT_OVERLOADED_CLASS_ENTRY(class_container, class_name, functions, handle_fcall, handle_propget, handle_propset) \
    {                                                           \
        class_container.name = strdup(class_name);              \
        class_container.name_length = strlen(class_name);       \
        class_container.info.internal.builtin_functions = functions;          \
        }
#else
#define MGD_INIT_OVERLOADED_CLASS_ENTRY(class_container, class_name, functions, handle_fcall, handle_propget, handle_propset) \
    {                                                           \
        class_container.name = strdup(class_name);              \
        class_container.name_length = strlen(class_name);       \
        class_container.builtin_functions = functions;          \
        }
#endif

/* * * * HACK ALERT ! * * * */

/* Piotras: I need to change current_user->sitegroup for object's 
 * parameters and attachments. Sitegroup is change on the fly and reverted
 * back to previous state. This is mandatory. Any other way we must change 
 * php_midgard_create in many many places and core's mgd_vcreate.
 * Such change will also affect old quota features. This one will be computed
 * for object and its sitegroup instead of SG0 when midgard admin will 
 * create non SG0 records.
 */ 

#define _MGD_SITEGROUP_FORCE() \
	if (!MGD_PROPFIND(getThis(), "sitegroup" , sitegroup_property)) {} \
	current_sitegroup = mgd_handle()->current_user->sitegroup; \
	convert_to_long_ex(sitegroup_property); \
	mgd_handle()->current_user->sitegroup = Z_LVAL_PP(sitegroup_property);

#define _MGD_SITEGROUP_FORCE_REVERT() \
	mgd_handle()->current_user->sitegroup = current_sitegroup;

/* * * * END HACK * * * */


#endif


GByteArray *mgd_preparse_string(char *phpcode);

#if MIDGARD_142MOD
#define TOUCH_CACHE
#else
#define TOUCH_CACHE mgd_cache_touch(mgd_handle(), 0)
#endif
