/* $Id: mgd_oop.h 16860 2008-07-11 13:04:37Z piotras $
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

#ifndef MGD_OOP_H
#define MGD_OOP_H

#include "php_midgard.h"
#include <midgard/tablenames.h>

MGD_FUNCTION(ret_type, oop_parameter, (type param));
MGD_FUNCTION(ret_type, oop_parameter_list, (type param));
MGD_FUNCTION(ret_type, oop_parameter_search, (type param));

MGD_FUNCTION(ret_type, get_attachment, (type param));
MGD_FUNCTION(ret_type, oop_attachment_create, (type param));
MGD_FUNCTION(ret_type, oop_attachment_list, (type param));
MGD_FUNCTION(ret_type, open_attachment, (type param));
MGD_FUNCTION(ret_type, serve_attachment, (type param));
MGD_FUNCTION(ret_type, delete_attachment, (type param));
MGD_FUNCTION(ret_type, update_attachment, (type param));

MGD_FUNCTION(ret_type, create_sitegroup, (type param));
MGD_FUNCTION(ret_type, delete_sitegroup, (type param));
MGD_FUNCTION(ret_type, update_sitegroup, (type param));

MGD_FUNCTION(ret_type, get_object_by_guid, (type param));
#if HAVE_MIDGARD_MULTILANG
MGD_FUNCTION(ret_type, get_object_by_guid_all_langs, (type param));
MGD_FUNCTION(ret_type, is_owner_by_guid, (type param));
#endif /* HAVE_MIDGARD_MULTILANG */

typedef struct {
	int type;
	char * name;
/* default value ? */
} MidgardProperty;

typedef MidgardProperty* MidgardPropertyPtr;

typedef struct {
   const char *name;
   const char *table;
   zend_function_entry *methods;
   zend_class_entry class_entry;
   MidgardProperty *properties;
   zend_class_entry *entry_ptr;
} MidgardClass;

typedef MidgardClass* MidgardClassPtr;
extern MidgardClassPtr MidgardClasses[];

void mgd_object_init(zval *obj, ...);
void php_midgard_bless(zval *object, MidgardClass *species);
void php_midgard_ctor(zval *object, MidgardClass *species);
void php_midgard_delete(zval * return_value, const char *table, int id);
void php_midgard_delete_repligard(const char *table, int id);
void php_midgard_update(zval * return_value, const char *table,
   const char *fields, int id, ...);
void php_midgard_create(zval * return_value, zval * self, const char *table,
   const char *fields, const char *values, ...);
void php_midgard_select(MidgardClass *species, zval * return_value,
   const char *fields, const char *tables,
   const char *where, const char *order, ...);
void php_midgard_sitegroup_get(MidgardClass *species,
               zval * return_value, int grouped,
               const char *fields, const char *table, int id);
void php_midgard_get(MidgardClass *species, zval * return_value,
   const char *fields, const char *table, int id);

#if HAVE_MIDGARD_MULTILANG
void php_midgard_get_by_name_lang(MidgardClass *species, zval * return_value,
			     const char *fields, const char *table,
				  const char *idfield, int parent_id, const char *name, int lang);

#endif /* HAVE_MIDGARD_MULTILANG */

void php_midgard_get_object(zval *return_value, int table, int id);

#define MIDGARD_CLASS(name,table,constructor,functions) \
static zend_function_entry name ## Methods[] = \
   { \
      PHP_FALIAS(constructor, mgd_get_ ## functions ,    NULL) \
      PHP_FALIAS(create,      mgd_create_ ## functions , NULL) \
      PHP_FALIAS(update,      mgd_update_ ## functions , NULL) \
      PHP_FALIAS(delete,      mgd_delete_ ## functions , NULL) \
     {NULL, NULL, NULL} \
   }; \
MidgardClass name = { \
   #name, \
   #table, \
   name ## Methods, \
   {}, \
   NULL \
};

#endif
#if HAVE_MIDGARD_MULTILANG

#define MIDGARD_CLASS_I18N(name,table,constructor,functions) \
static zend_function_entry name ## Methods[] = \
   { \
      PHP_FALIAS(constructor,     mgd_get_ ## functions ,    NULL) \
      PHP_FALIAS(create,   mgd_create_ ## functions , NULL) \
      PHP_FALIAS(create_content,   mgd_create_ ## functions ## _content,   NULL) \
      PHP_FALIAS(update,   mgd_update_ ## functions , NULL) \
      PHP_FALIAS(update_content,   mgd_update_ ## functions ## _content,   NULL) \
      PHP_FALIAS(delete,   mgd_delete_ ## functions , NULL) \
      PHP_FALIAS(delete_content,   mgd_delete_ ## functions ## _content,   NULL) \
      PHP_FALIAS(fetch,    mgd_oop_list_fetch,        NULL) \
      MIDGARD_OOP_ATTACHMENT_METHODS \
      MIDGARD_OOP_SITEGROUP_METHODS \
      MIDGARD_OOP_PARAMETER_METHODS \
      {NULL, NULL, NULL} \
   }; \
MidgardClass name = { \
   #name, \
   #table, \
   name ## Methods, \
   {}, \
   NULL \
};

#endif /* HAVE_MIDGARD_MULTILANG */
