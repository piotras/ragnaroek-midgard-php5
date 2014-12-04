/* $Id: topic.c 10324 2006-11-27 12:47:04Z piotras $
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

MGD_FUNCTION(ret_type, is_topic_owner, (type param))
{
	IDINIT;
	CHECK_MGD;
	RETVAL_LONG(istopicowner(id));
}

static const char *topic_sort(const char *order)
{
	static struct
	{
		const char *order, *sort;
	}
	sort[] =
	{
		{
		"alpha", "name ASC"}
		, {
		"reverse alpha", "name DESC"}
		, {
		"score", "score ASC,name ASC"}
		, {
		"reverse score", "score DESC,name ASC"}
		, {
		"revised", "revised ASC"}
		, {
		"reverse revised", "revised DESC"}
		, {
		"created", "created ASC"}
		, {
		"reverse created", "created DESC"}
		, {
		NULL, "score DESC,name"}
	};
	int i;

	for (i = 0; sort[i].order; i++)
		if (strcmp(order, sort[i].order) == 0)
			return sort[i].sort;

	return sort[i].sort;
}

MGD_FUNCTION(ret_type, list_topics, (type param))
{
	const char *sortv = NULL;
	zval **id, **sortn;

	RETVAL_FALSE;
	CHECK_MGD;
	switch (ZEND_NUM_ARGS()) {
		case 2:
			if (zend_get_parameters_ex(2, &id, &sortn) == SUCCESS) {
				convert_to_long_ex(id);
				convert_to_string_ex(sortn);
				sortv = topic_sort((*sortn)->value.str.val);
				break;
			}
		case 1:
			if (zend_get_parameters_ex(1, &id) == SUCCESS) {
				convert_to_long_ex(id);
				sortv = "score DESC,name";
				break;
			}
		default:
			WRONG_PARAM_COUNT;
	}

#if 0
	if (!istopicreader(id))
		return;
#endif

	php_midgard_select(&MidgardTopic, return_value,
			"topic.id AS id,score,name,owner,topic_i.extra AS extra,description,code"
			",created,revised,creator,revisor, topic.sitegroup AS sitegroup",
			"topic, topic_i", 
			"up=$d AND topic.id=topic_i.sid", 
			sortv, 
			(*id)->value.lval);
}

MGD_FUNCTION(ret_type, is_in_topic_tree, (type param))
{
	zval **root, **topic;
	int i;

	RETVAL_FALSE;
	CHECK_MGD;
	if (ZEND_NUM_ARGS() != 2
	    || zend_get_parameters_ex(2, &root, &topic) != SUCCESS)
		   WRONG_PARAM_COUNT;
	convert_to_long_ex(root);
	convert_to_long_ex(topic);

	if((*topic)->value.lval == 0 || /* useless to waste time if topic=0 */
				!mgd_exists_id(mgd_handle(),
						"topic", "id=$d",
						(*topic)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	if ((*root)->value.lval == 0)
		RETURN_TRUE; /* always true if root=0 */
	if(!mgd_exists_id(mgd_handle(), "topic", "id=$d", (*root)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
#if 0
	if (!istopicreader((*topic)->value.lval))
		return;
#endif

	if((i = mgd_is_in_tree(mgd_handle(), "topic", "up",
				(*root)->value.lval, (*topic)->value.lval)))
		RETURN_TRUE;
}

MGD_FUNCTION(ret_type, get_topic, (type param))
{
	zval **id, **name;
   int objid;

	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
		case 0:
			php_midgard_bless(return_value, &MidgardTopic);
			mgd_object_init(return_value, "id", "up", "name",
					 "extra", "owner", "score",
					 "description", "revised", "created",
					 "revisor", "creator", "revision",
					 "code", NULL);
			return;
		case 1:
			if (zend_get_parameters_ex(1, &id) == FAILURE) {
				WRONG_PARAM_COUNT;
         }

			convert_to_long_ex(id);
         php_midgard_get_object(return_value, MIDGARD_OBJECT_TOPIC,
            (*id)->value.lval);
			break;

		case 2:
			if (zend_get_parameters_ex(2, &id, &name) == FAILURE) {
				WRONG_PARAM_COUNT;
         }

			convert_to_long_ex(id);
			convert_to_string_ex(name);
         objid = mgd_exists_id(mgd_handle(), "topic", "up=$d AND name=$q",
            (*id)->value.lval, (*name)->value.str.val);
         php_midgard_get_object(return_value, MIDGARD_OBJECT_TOPIC, objid);
			break;

		default:
			WRONG_PARAM_COUNT;
	}
}

MGD_FUNCTION(ret_type, create_topic, (type param))
{
	zval **up, **name, **description, **extra, **owner, **code;
	zval *self;

	CHECK_MGD;
	RETVAL_FALSE;

	if ((self = getThis()) != NULL) {
		if (!MGD_PROPFIND(self, "up", up)
		    || !MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "description", description)
		    || !MGD_PROPFIND(self, "extra", extra)
		    || !MGD_PROPFIND(self, "owner", owner)
		    || !MGD_PROPFIND(self, "code", code)) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 6
		    || zend_get_parameters_ex(6, &up, &name, &description,
					      &extra, &owner, &code) != SUCCESS)
			WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(up);
	convert_to_string_ex(name);
	convert_to_string_ex(description);
	convert_to_string_ex(extra);
	convert_to_long_ex(owner);
	convert_to_string_ex(code);

	if (!istopicowner((*up)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

   if (mgd_exists_id(mgd_handle(), "topic", "up=$d AND name=$q",
			     (*up)->value.lval, (*name)->value.str.val))
		RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

	if ((*up)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "topic", "id=$d", (*up)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	/* TODO: should we in fact allow owner == 0 for non-root? */
	if ((*owner)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "grp", "id=$d", (*owner)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	php_midgard_create(return_value, self, "topic",
			   "up,name,owner,code,"
			   "creator,created,revisor,revised,revision",
			   "$d,$q,$d,$q,$d,Now(),$d,Now(),0",
			   (*up)->value.lval, (*name)->value.str.val,
			   (*owner)->value.lval,
			   (*code)->value.str.val, mgd_user(mgd_handle()),
			   mgd_user(mgd_handle()));
	if(return_value->value.lval > 0){
		/* NO REPLIGARD ENTRY FOR ML CONTENT! */
		zval *dummy;
		MAKE_STD_ZVAL(dummy);
		php_midgard_create(dummy, NULL, "topic_i",
				"description, extra, sid",
				"$q,$q,$d",
				(*description)->value.str.val,
				(*extra)->value.str.val,
				return_value->value.lval
				);
	}	
	PHP_CREATE_REPLIGARD("topic", return_value->value.lval);
	RETURN_LONG(return_value->value.lval);
}

MGD_FUNCTION(ret_type, update_topic, (type param))
{
	zval **id, **name, **description, **extra, **owner, **code, *self;
   zval **score;
   char *pscore;

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (!MGD_PROPFIND(self, "id", id)
			|| !MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "description", description)
		    || !MGD_PROPFIND(self, "extra", extra)
		    || !MGD_PROPFIND(self, "owner", owner)
		    || !MGD_PROPFIND(self, "code", code)) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
		if (!MGD_PROPFIND(self, "score", score)) {
         score = NULL;
      }
	}
	else {
		if (ZEND_NUM_ARGS() != 6
		    || zend_get_parameters_ex(6, &id, &name, &description,
					      &extra, &owner, &code) != SUCCESS) {
			WRONG_PARAM_COUNT;
		}

      score = NULL;
	}

	convert_to_long_ex(id);
	convert_to_string_ex(name);
	convert_to_string_ex(description);
	convert_to_string_ex(extra);
	convert_to_string_ex(code);
	convert_to_long_ex(owner);
   if (score) {
      convert_to_long_ex(score);
      convert_to_string_ex(score);
      pscore = (*score)->value.str.val;
   } else {
      pscore = "score";
   }

	if (!istopicowner((*id)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	/* TODO: should we in fact allow owner == 0 for non-root? */
	if ((*owner)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "grp", "id=$d", (*owner)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

   if (mgd_exists_id(mgd_handle(), "topic", "up=$d AND name=$q AND id<>$d",
         mgd_idfield(mgd_handle(), "up", "topic", (*id)->value.lval),
         (*name)->value.str.val, (*id)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

	php_midgard_update(return_value, "topic",
			   "name=$q,owner=$d,code=$q,"
			   "revisor=$d,revised=Now(),revision=revision+1,score=$s",
			   (*id)->value.lval, (*name)->value.str.val,
			   (*owner)->value.lval,
			   (*code)->value.str.val, mgd_user(mgd_handle()),
            pscore
            );
	long i_id = 0;
	midgard_res *res = mgd_ungrouped_select(
			mgd_handle(), 
			"id", "topic_i", 
			"sid=$d AND lang=$d", 
			NULL, 
			(*id)->value.lval, mgd_lang(mgd_handle()));
	
	if (res && mgd_fetch(res)) {
		i_id = atol(mgd_colvalue(res, 0));
		mgd_release(res);
	}

	if(i_id > 0){
		php_midgard_update(return_value, "topic_i",
				"description=$q,extra=$q",
				i_id,
				(*description)->value.str.val,
				(*extra)->value.str.val);
	
	PHP_UPDATE_REPLIGARD("topic", (*id)->value.lval);
	RETURN_TRUE;
	}
}

MGD_FUNCTION(ret_type, update_topic_score, (type param))
{
	zval **id, **score, *self;

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &score) !=
		    SUCCESS) {
			WRONG_PARAM_COUNT;
		}
		if (!MGD_PROPFIND(self, "id", id)) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		if (ZEND_NUM_ARGS() != 2
		    || zend_get_parameters_ex(2, &id, &score) != SUCCESS)
			WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(id);
	convert_to_long_ex(score);

	if (!istopicowner((*id)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	php_midgard_update(return_value, "topic", "score=$d", (*id)->value.lval,
			   (*score)->value.lval);
	PHP_UPDATE_REPLIGARD("topic", (*id)->value.lval);
}

MGD_FUNCTION(ret_type, delete_topic, (type param))
{
	IDINIT;
	CHECK_MGD;
	if (mgd_has_dependants(mgd_handle(), id, "topic"))
		RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
	if (mgd_exists_id(mgd_handle(), "article", "topic=$d", id))
		RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

	if (!istopicowner(mgd_idfield(mgd_handle(), "up", "topic", id)))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	php_midgard_delete(return_value, "topic", id);
	PHP_DELETE_REPLIGARD("topic", id);
}

MGD_FUNCTION(ret_type, copy_topic, (int id[, int root]))
{
	zval **id, **root;
	int id_r;
	int new_root;

	RETVAL_FALSE;
	CHECK_MGD;
        switch (ZEND_NUM_ARGS()) {
		case 2:
			if (zend_get_parameters_ex(2, &id, &root) !=
			    SUCCESS) WRONG_PARAM_COUNT;
			convert_to_long_ex(root);
	                new_root = (*root)->value.lval;
			break;
		case 1:
			if (zend_get_parameters_ex(1, &id) != SUCCESS)
				WRONG_PARAM_COUNT;
		      convert_to_long_ex(id);
			new_root =   mgd_idfield(mgd_handle(), "up", "topic",(*id)->value.lval);
			break;
		default:
			WRONG_PARAM_COUNT;
	}
	convert_to_long_ex(id);

	/* if new_root is 0 or if not owner, access denied (unless isadmin) */
	if (!istopicowner(new_root)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }

	/* root must be in same SG or be 0 */
	if (new_root != 0 &&
			!mgd_exists_bool(mgd_handle(), "topic src, topic tgt",
			   "src.id=$d AND tgt.id=$d"
			   " AND (src.sitegroup=tgt.sitegroup"
			   " OR src.sitegroup=0" " OR tgt.sitegroup=0)",
			   (*id)->value.lval, new_root))
		RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);

	id_r = mgd_copy_topic(mgd_handle(), (*id)->value.lval);
#if HAVE_MIDGARD_QUOTA
	if (mgd_get_quota_error(mgd_handle())) {
	  mgd_set_quota_error(mgd_handle(), 0);
	  RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
	}
#endif /* HAVE_MIDGARD_QUOTA */

	if (id_r) {
		php_midgard_update(return_value, "topic", "up=$i", id_r,
				   new_root);
		PHP_UPDATE_REPLIGARD("topic", id_r);
	}
	RETVAL_LONG(id_r);
}

MGD_FUNCTION(ret_type, delete_topic_tree, (type param))
{
	IDINIT;
	CHECK_MGD;
	if (!istopicowner(mgd_idfield(mgd_handle(), "up", "topic", id)))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	if (mgd_delete_topic(mgd_handle(), id))
		RETVAL_TRUE;
}


MGD_FUNCTION(ret_type, get_topic_by_path, (type param))
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
              
  if(!MGD_PARSE_COMMON_PATH(mgd_handle(), (*path)->value.str.val, "topic", "topic", &id, &up)) {
    php_midgard_get_object(return_value, MIDGARD_OBJECT_TOPIC, id);
    return;
  }
}



MGD_MOVE_FUNCTION(topic, topic, topic, up)

MGD_WALK_FUNCTION(topic)

static zend_function_entry MidgardTopicMethods[] = {
   PHP_FALIAS(midgardtopic,   mgd_get_topic,          NULL)
   PHP_FALIAS(create,         mgd_create_topic,       NULL)
   PHP_FALIAS(update,         mgd_update_topic,       NULL)
   PHP_FALIAS(delete,         mgd_delete_topic,       NULL)
   PHP_FALIAS(setscore,       mgd_update_topic_score, NULL)
   PHP_FALIAS(fetch,          mgd_oop_list_fetch,     NULL)
   MIDGARD_OOP_ATTACHMENT_METHODS
   MIDGARD_OOP_SITEGROUP_METHODS
   MIDGARD_OOP_PARAMETER_METHODS
   {  NULL,             NULL,                   NULL}
};
MidgardClass MidgardTopic = {
   "MidgardTopic",
   "topic",
   MidgardTopicMethods,
   {},
   NULL
};

#endif
