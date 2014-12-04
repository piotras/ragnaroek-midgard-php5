/* $Id: quota.c 10324 2006-11-27 12:47:04Z piotras $
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

#if HAVE_MIDGARD_QUOTA

MGD_FUNCTION(ret_type, list_quotas, (type param))
{
  CHECK_MGD;
  php_midgard_select(&MidgardQuota, return_value,
		     "id,sg,tablename,spacefields,number,space", "quota", NULL, NULL);
}

MGD_FUNCTION(ret_type, get_quota, (type param))
{
  zval **id;
  CHECK_MGD;
  
  switch (ZEND_NUM_ARGS()) {
  case 0:
    php_midgard_bless(return_value, &MidgardQuota);
    mgd_object_init(return_value, "id", "sg", "tablename", "spacefields", "number", "space", NULL);
    return;
  case 1:
    if (zend_get_parameters_ex(1, &id) != SUCCESS) {
      WRONG_PARAM_COUNT;
    }
    convert_to_long_ex(id);
			
    php_midgard_get_object(return_value, MIDGARD_OBJECT_QUOTA, (*id)->value.lval);
    break;
  default:
    WRONG_PARAM_COUNT;
  }
}

MGD_FUNCTION(ret_type, get_quota_by_tablename, (type param))
{
  zval **tablename, **sitegroup;
  int objid,sitegroup_param = 0, sg = 0;

   CHECK_MGD;

  switch (ZEND_NUM_ARGS()) {
  case 1:
    if (zend_get_parameters_ex(1, &tablename) != SUCCESS) {
      WRONG_PARAM_COUNT;
    }
    break;
  case 2:
    if (zend_get_parameters_ex(2, &tablename, &sitegroup) != SUCCESS) {
      WRONG_PARAM_COUNT;
    }
    sitegroup_param = 1;
    break;
  default:
      WRONG_PARAM_COUNT;
   }

   convert_to_string_ex(tablename);
   if (sitegroup_param) {
     convert_to_long_ex(sitegroup);
     if ((*sitegroup)->value.lval != mgd_sitegroup(mgd_handle()) && !mgd_isroot(mgd_handle())) {
       RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
     }

   }
  
   sg = (sitegroup_param)?(*sitegroup)->value.lval:mgd_sitegroup(mgd_handle());
   
   if (!(objid = mgd_exists_id(mgd_handle(), "quota", "tablename=$q and sg=$d",
			       (*tablename)->value.str.val, sg))) {
     RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
   }
   php_midgard_get_object(return_value, MIDGARD_OBJECT_QUOTA, objid);
}

MGD_FUNCTION(ret_type, get_quota_info_by_tablename, (type param))
{
  zval **tablename, **sitegroup, **ppresult;
  int objid, space, sitegroup_param = 0, sg = 0;

   CHECK_MGD;

  switch (ZEND_NUM_ARGS()) {
  case 1:
    if (zend_get_parameters_ex(1, &tablename) != SUCCESS) {
      WRONG_PARAM_COUNT;
    }
    break;
  case 2:
    if (zend_get_parameters_ex(2, &tablename, &sitegroup) != SUCCESS) {
      WRONG_PARAM_COUNT;
    }
    sitegroup_param = 1;
    break;
  default:
    WRONG_PARAM_COUNT;
  }
  
   convert_to_string_ex(tablename);
   if (sitegroup_param) {
     convert_to_long_ex(sitegroup);
     if ((*sitegroup)->value.lval != mgd_sitegroup(mgd_handle()) && !mgd_isroot(mgd_handle())) {
       RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
     }

   }
  
   sg = (sitegroup_param)?(*sitegroup)->value.lval:mgd_sitegroup(mgd_handle());
   
   if (!(objid = mgd_exists_id(mgd_handle(), "quota", "tablename=$q and sg=$d",
			       (*tablename)->value.str.val, sg))) {
     php_midgard_bless(return_value, &MidgardQuota);
     mgd_object_init(return_value, "id", "sg", "tablename", "spacefields", "number", "space", NULL);
   } else {
     php_midgard_get_object(return_value, MIDGARD_OBJECT_QUOTA, objid);
   }
   if (!(Z_OBJPROP_P(return_value)) || zend_hash_find(Z_OBJPROP_P(return_value), "spacefields", 12,
							      (void **)&ppresult) == FAILURE) {
     RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
   }
   convert_to_string_ex(ppresult);
   if (strlen((*ppresult)->value.str.val) > 0 || !strcmp((*tablename)->value.str.val, "blobsdata") || !strcmp((*tablename)->value.str.val, "wholesg")) {
     space = mgd_get_quota_space(mgd_handle(), (*tablename)->value.str.val, (*ppresult)->value.str.val, sg);
   } else {
     space = 0;
   }
   add_property_long(return_value, "eff_number", mgd_get_quota_count(mgd_handle(), (*tablename)->value.str.val, sg));
   add_property_long(return_value, "eff_space", space);
}
 
MGD_FUNCTION(ret_type, create_quota, (type param))
{
  zval **sg, **tablename, **spacefields, **number, **space, *self;

 RETVAL_FALSE;
  CHECK_MGD;

  if ((self = getThis()) != NULL) {
    if (!MGD_PROPFIND(self, "tablename", tablename)
	|| !MGD_PROPFIND(self, "spacefields", spacefields) 
	|| !MGD_PROPFIND(self, "number", number) 
	|| !MGD_PROPFIND(self, "space", space) 
	|| !MGD_PROPFIND(self, "sg", sg)) {
      RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
    }
  }
  else {
    if (ZEND_NUM_ARGS() != 5
	|| zend_get_parameters_ex(5, &sg, &tablename, &spacefields, &number, &space) == FAILURE) {
      WRONG_PARAM_COUNT;
    }
  }

  convert_to_long_ex(sg);
  convert_to_string_ex(tablename);
  convert_to_string_ex(spacefields);
  convert_to_long_ex(number);
  convert_to_long_ex(space);

  if (!mgd_isroot(mgd_handle())) {
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
  }
	
  if (mgd_exists_id(mgd_handle(), "quota", "tablename=$q AND sg=$d",
		    (*tablename)->value.str.val, (*sg)->value.lval))
    RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

  php_midgard_create(return_value, self,
		     "quota", "sg, tablename, spacefields, number, space",
		     "$d,$q,$q,$d,$d", 
		     (*sg)->value.lval, (*tablename)->value.str.val, (*spacefields)->value.str.val, (*number)->value.lval,(*space)->value.lval);
  PHP_CREATE_REPLIGARD("quota", return_value->value.lval);
  mgd_touch_recorded_quota(mgd_handle(), "wholesg", (*sg)->value.lval);
  mgd_touch_quotacache(mgd_handle());
}

MGD_FUNCTION(ret_type, update_quota, (type param))
{
  zval **id, **sg, **tablename, **spacefields, **number, **space, *self;
  int objid;
  
  RETVAL_FALSE;
  CHECK_MGD;

  if ((self = getThis()) != NULL) {
    if (!MGD_PROPFIND(self, "id", id) 
	|| !MGD_PROPFIND(self, "tablename", tablename)
	|| !MGD_PROPFIND(self, "spacefields", spacefields)
	|| !MGD_PROPFIND(self, "number", number) 
	|| !MGD_PROPFIND(self, "space", space) 
	|| !MGD_PROPFIND(self, "sg", sg)) {
      RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
    }
  }
  else {
    if (ZEND_NUM_ARGS() != 6
	|| zend_get_parameters_ex(6, &id, &sg, &tablename, &spacefields, &number, &space) == FAILURE) {
      WRONG_PARAM_COUNT;
    }
  }
  
  convert_to_long_ex(id);
  convert_to_long_ex(sg);
  convert_to_string_ex(tablename);
  convert_to_string_ex(spacefields);
  convert_to_long_ex(number);
  convert_to_long_ex(space);
	
  if (!mgd_isroot(mgd_handle())) {
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
  }
	
  objid = mgd_exists_id(mgd_handle(), "quota", "tablename=$q AND sg=$d",
			  (*tablename)->value.str.val, (*sg)->value.lval);

  if (objid != 0 && objid != (*id)->value.lval)
    RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

  php_midgard_update(return_value, "quota", "sg=$d, tablename=$q, spacefields=$q, number=$d, space=$d, count_is_current=0, space_is_current=0",
		     (*id)->value.lval,
		     (*sg)->value.lval, (*tablename)->value.str.val, (*spacefields)->value.str.val, (*number)->value.lval,(*space)->value.lval);
  PHP_UPDATE_REPLIGARD("quota", (*id)->value.lval);
  if (strcmp((*tablename)->value.str.val, "wholesg")) {
    mgd_touch_recorded_quota(mgd_handle(), "wholesg", (*sg)->value.lval);
  }
  mgd_touch_quotacache(mgd_handle());
}

MGD_FUNCTION(ret_type, delete_quota, (type param))
{
  int sg;
  IDINIT;
  CHECK_MGD;

   if (zend_parse_parameters(1 TSRMLS_CC, "l", &id)  == FAILURE) {
	   RETVAL_FALSE;     
	   WRONG_PARAM_COUNT;   
   }

   if (!mgd_isroot(mgd_handle())) {
	   RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }

   sg = mgd_idfield(mgd_handle(), "sg", "quota", id);
   php_midgard_delete(return_value, "quota", id);
   PHP_DELETE_REPLIGARD("quota", id);
   mgd_touch_recorded_quota(mgd_handle(), "wholesg", sg);
   mgd_touch_quotacache(mgd_handle());
}

MGD_FUNCTION(ret_type, get_sitegroup_size, (type param))
{
	guint id;
	CHECK_MGD;

	if (zend_parse_parameters(1 TSRMLS_CC, "l", &id)  == FAILURE) {
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(mgd_isroot(mgd_handle()))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	guint32 size = midgard_quota_get_sitegroup_size(
			mgd_handle(), id);

	RETVAL_LONG(size);
}

MIDGARD_CLASS(MidgardQuota, quota, midgardquota, quota)

#endif /* HAVE_MIDGARD_QUOTA */

#endif
