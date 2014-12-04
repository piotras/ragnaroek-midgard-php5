/* $Id: page.c 10324 2006-11-27 12:47:04Z piotras $
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
#include <midgard/pageresolve.h>

#ifdef PHP_MIDGARD_LEGACY_API

#if HAVE_MIDGARD_MULTILANG
void php_midgard_create_page_content_internal(zval *return_value, int longversion, int newid, const char* title, const char *content, int contentauthor, int contentowner, int lang) {

  if (longversion) {
    php_midgard_create(return_value, 0, "page_i",
		       "sid,title,content,changed,author,owner,lang",
		       "$d,$q,$q,Curdate(),$d,$d,$d",
			 newid, title, content, contentauthor, contentowner, lang);
  } else {
    php_midgard_create(return_value, 0, "page_i",
		       "sid,title,content,changed,lang",
			 "$d,$q,$q,Curdate(),$d",
		       newid, title, content, lang);
  }
  PHP_CREATE_REPLIGARD("page_i", return_value->value.lval);
  /* return return_value->value.lval; */
}

void php_midgard_update_page_content_internal(zval *return_value, int longversion, int i_id, const char* title, const char *content, int contentauthor, int contentowner) {
  if (longversion) {
    php_midgard_update(return_value, "page_i",
		       "changed=Curdate(),title=$q,content=$q,author=$d,owner=$d",
		       i_id,
		       title, content, contentauthor, contentowner);
  } else {
    php_midgard_update(return_value, "page_i",
		       "changed=Curdate(),title=$q,content=$q",
		       i_id,
		       title, content);

  }
  PHP_UPDATE_REPLIGARD("page_i", i_id);
  /* return return_value->value.lval; */
}

#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(ret_type, is_page_owner, (type param))
{
    IDINIT;
    CHECK_MGD;
    RETVAL_LONG(ispageowner(id));
}

MGD_FUNCTION(ret_type, copy_page, (type param))
{
    zval **id, **root; int id_r;

    RETVAL_FALSE;
	CHECK_MGD;
    if (ZEND_NUM_ARGS() != 2
	|| zend_get_parameters_ex(2, &id, &root) != SUCCESS)
	WRONG_PARAM_COUNT;
    convert_to_long_ex(id);
    convert_to_long_ex(root);

	/* root must be in same SG or be 0 */
	if ((*root)->value.lval != 0 && !mgd_exists_bool(mgd_handle(), "page src, page tgt",
										"src.id=$d AND tgt.id=$d"
										" AND (src.sitegroup=tgt.sitegroup"
											" OR src.sitegroup=0"
											" OR tgt.sitegroup=0)",
										(*id)->value.lval,(*root)->value.lval)) RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);

	id_r = mgd_copy_page(mgd_handle(),  (*id)->value.lval);
#if HAVE_MIDGARD_QUOTA
	g_log("midgard-lib", G_LOG_LEVEL_WARNING,
	      "QUOTA_ERR: %d", mgd_get_quota_error(mgd_handle()));

	if (mgd_get_quota_error(mgd_handle())) {
	  mgd_set_quota_error(mgd_handle(), 0);
	  g_log("midgard-lib", G_LOG_LEVEL_WARNING,
		"JUHU");
	  RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
	}
#endif /* HAVE_MIDGARD_QUOTA */
    if(id_r) {
      php_midgard_update(return_value, "page", "up=$i", id_r, (*root)->value.lval);
      PHP_UPDATE_REPLIGARD("page",id_r);
   }
	RETVAL_LONG(id_r);

   TOUCH_CACHE;
}

MGD_FUNCTION(ret_type, list_pages, (type param))
{
#if HAVE_MIDGARD_MULTILANG
  int lang = mgd_lang(mgd_handle());
#endif /* HAVE_MIDGARD_MULTILANG */
    IDINIT;
	CHECK_MGD;
#if ! HAVE_MIDGARD_MULTILANG
    php_midgard_select(&MidgardPage, return_value, "page.id AS id,name,style,title,changed,author,"
#else /* HAVE_MIDGARD_MULTILANG */
  php_midgard_select(&MidgardPage, return_value, "page.id AS id,name,style,title,lang,page.changed AS changed,page.author AS author, page.owner AS owner, page_i.author AS contentauthor, page_i.owner AS contentowner, "
#endif /* HAVE_MIDGARD_MULTILANG */
	       NAME_FIELD " AS authorname"
         ",page.sitegroup"
#if ! HAVE_MIDGARD_MULTILANG
         , "page,person",
	       "up=$d AND person.id=page.author", "name", id);
#else /* HAVE_MIDGARD_MULTILANG */
         , "page,page_i,person",
	       "up=$d AND page_i.sid=page.id AND page_i.lang=$d AND person.id=page.author", "name",  id, lang);
#endif /* HAVE_MIDGARD_MULTILANG */
}

MGD_FUNCTION(ret_type, is_in_page_tree, (type param))
{
    zval **root, **page;

    RETVAL_FALSE;
	CHECK_MGD;
    if (ZEND_NUM_ARGS() != 2
	|| zend_get_parameters_ex(2, &root, &page) != SUCCESS)
	WRONG_PARAM_COUNT;
    convert_to_long_ex(root);
    convert_to_long_ex(page);

	if((*page)->value.lval == 0 || /* useless to waste time if page=0 */
				!mgd_exists_id(mgd_handle(),
						"page", "id=$d",
						(*page)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	if ((*root)->value.lval == 0)
		RETURN_TRUE; /* always true if root=0 */
	if(!mgd_exists_id(mgd_handle(), "page", "id=$d", (*root)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

    if(mgd_is_in_tree(mgd_handle(), "page", "up",
							(*root)->value.lval, (*page)->value.lval))
		RETURN_TRUE;
}

MGD_FUNCTION(ret_type, get_page, (type param))
{
	zval **id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
		case 0:
		   php_midgard_bless(return_value, &MidgardPage);
#if ! HAVE_MIDGARD_MULTILANG
		   mgd_object_init(return_value, "up", "name", "style", "title", "content", "author", "auth", "active", NULL);
#else /* HAVE_MIDGARD_MULTILANG */
		   mgd_object_init(return_value, "up", "name", "style", "title", "content", "author", "auth", "active", "owner", "lang", "contentauthor", "contentowner", NULL);
#endif /* HAVE_MIDGARD_MULTILANG */
			return;

		case 1:
			if (zend_get_parameters_ex(1, &id) == FAILURE) {
			   WRONG_PARAM_COUNT;
			}

			convert_to_long_ex(id);
			break;
#if ! HAVE_MIDGARD_MULTILANG

#endif /* not HAVE_MIDGARD_MULTILANG */
		default:
			WRONG_PARAM_COUNT;
	}

   php_midgard_get_object(return_value, MIDGARD_OBJECT_PAGE, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, get_page_by_name, (type param))
{
   zval **root, **page;
   int objid;

   CHECK_MGD;

   if (ZEND_NUM_ARGS() != 2
         || zend_get_parameters_ex(2, &root, &page) != SUCCESS) {
      WRONG_PARAM_COUNT;
   }

   convert_to_long_ex(root);
   convert_to_string_ex(page);

   objid = mgd_exists_id(mgd_handle(), "page", "up=$d AND name=$q",
      (*root)->value.lval, (*page)->value.str.val);
   php_midgard_get_object(return_value, MIDGARD_OBJECT_PAGE, objid);
}

MGD_FUNCTION(ret_type, create_page, (type param))
{
#if ! HAVE_MIDGARD_MULTILANG
	zval **up, **name, **style, **title, **content, **author, **auth, **active, *self;
#else /* HAVE_MIDGARD_MULTILANG */
  zval **up, **name, **style, **title, **content, **author, **owner, **auth, **active, **contentauthor, **contentowner, *self;
  zval *retval_content0, *retval_content;
  int newid;
  int longversion, noowner;
  int success = 1, created0 = 0;
  int lang = mgd_lang(mgd_handle());

  longversion = 0; noowner = 0;
  MAKE_STD_ZVAL(retval_content0);
  MAKE_STD_ZVAL(retval_content);

#endif /* HAVE_MIDGARD_MULTILANG */

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "up", up)
		    || !MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "style", style)
		    || !MGD_PROPFIND(self, "title", title)
		    || !MGD_PROPFIND(self, "content", content)
		    || !MGD_PROPFIND(self, "author", author)
#if HAVE_MIDGARD_MULTILANG
		    || !MGD_PROPFIND(self, "owner", owner)
#endif /* HAVE_MIDGARD_MULTILANG */
		    || !MGD_PROPFIND(self, "auth", auth)
		    || !MGD_PROPFIND(self, "active", active)
#if HAVE_MIDGARD_MULTILANG
		    || !MGD_PROPFIND(self, "contentauthor", contentauthor)
		    || !MGD_PROPFIND(self, "contentowner", contentowner)
#endif /* HAVE_MIDGARD_MULTILANG */
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
#if ! HAVE_MIDGARD_MULTILANG
	}
	else {
		if (ZEND_NUM_ARGS() != 8
		    || zend_get_parameters_ex(8, &up, &name, &style, &title,
#else /* HAVE_MIDGARD_MULTILANG */
		longversion = 1;

	} else {
	  switch (ZEND_NUM_ARGS()) {
	  case 8:
	    if (zend_get_parameters_ex(8, &up, &name, &style, &title,
#endif /* HAVE_MIDGARD_MULTILANG */
				     &content, &author, &auth,
				     &active) == FAILURE) WRONG_PARAM_COUNT;
#if HAVE_MIDGARD_MULTILANG
	    noowner = 1;
	    break;
	  case 9:
	    if (zend_get_parameters_ex(9, &up, &name, &style, &title,
				       &content, &author, &owner, &auth,
				       &active) == FAILURE) WRONG_PARAM_COUNT;
	    break;
	  case 11:
	    if (zend_get_parameters_ex(11, &up, &name, &style, &title,
				       &content, &author, &owner, &auth,
				       &active, &contentauthor, &contentowner) == FAILURE) WRONG_PARAM_COUNT;
	    longversion = 1;
	    break;
	  default:
	    WRONG_PARAM_COUNT;

	  }
#endif /* HAVE_MIDGARD_MULTILANG */
	}

	convert_to_long_ex(up);
	convert_to_string_ex(name);
	convert_to_long_ex(style);
	convert_to_string_ex(title);
	convert_to_string_ex(content);
	convert_to_long_ex(author);
#if HAVE_MIDGARD_MULTILANG
	if (!noowner) {
	  convert_to_long_ex(owner);
	}
#endif /* HAVE_MIDGARD_MULTILANG */
	convert_to_long_ex(auth);
	convert_to_long_ex(active);
#if HAVE_MIDGARD_MULTILANG
	if (longversion) {
	  convert_to_long_ex(contentauthor);
	  convert_to_long_ex(contentowner);
	  if ((*contentauthor)->value.lval != 0 && !mgd_exists_id(mgd_handle(), "person", "id=$d", (*contentauthor)->value.lval))
	    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}
#endif /* HAVE_MIDGARD_MULTILANG */

#if ! HAVE_MIDGARD_MULTILANG
	if (!ispageowner((*up)->value.lval))
#else /* HAVE_MIDGARD_MULTILANG */
	/* need to be owner of both page and page content */
	if (!ispageowner((*up)->value.lval) || !ispagecontentowner((*up)->value.lval, lang))
#endif /* HAVE_MIDGARD_MULTILANG */
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if ((*up)->value.lval != 0
	      && mgd_exists_id(mgd_handle(), "page", "up=$d AND name=$q",
			     (*up)->value.lval, (*name)->value.str.val))
		RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

	if ((*up)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "page", "id=$d", (*up)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

#if HAVE_MIDGARD_MULTILANG

#endif /* HAVE_MIDGARD_MULTILANG */
	if ((*style)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "style", "id=$d", (*style)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if (!mgd_exists_id(mgd_handle(), "person", "id=$d", (*author)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	php_midgard_create(return_value, self, "page",
#if ! HAVE_MIDGARD_MULTILANG
			   "up,name,style,title,changed,content,author,info",
			   "$d,$q,$d,$q,Curdate(),$q,$d,$d",
#else /* HAVE_MIDGARD_MULTILANG */
			   "up,name,style,changed,author,owner,info",
			   "$d,$q,$d,Curdate(),$d,$d,$d",
#endif /* HAVE_MIDGARD_MULTILANG */
			   (*up)->value.lval, (*name)->value.str.val,
#if ! HAVE_MIDGARD_MULTILANG
			   (*style)->value.lval, (*title)->value.str.val,
			   (*content)->value.str.val, (*author)->value.lval,
#else /* HAVE_MIDGARD_MULTILANG */
			   (*style)->value.lval, (*author)->value.lval, (noowner)?0:(*owner)->value.lval,
#endif /* HAVE_MIDGARD_MULTILANG */
			   ((*auth)->value.lval == 1) | ((*active)->value.lval ==
						      1) << 1);

	PHP_CREATE_REPLIGARD("page", return_value->value.lval);

#if HAVE_MIDGARD_MULTILANG
	newid = return_value->value.lval;
	if (newid) {
	  if (lang > 0) {
	    php_midgard_create_page_content_internal(retval_content0, longversion, newid, "", "", longversion?(*contentauthor)->value.lval:0, longversion?(*contentowner)->value.lval:0, 0);
	    if (!(retval_content0->value.lval)) success = 0;
	    else created0 = 1;
	  }
	  if (success) {
	    php_midgard_create_page_content_internal(retval_content, longversion, newid, (*title)->value.str.val, (*content)->value.str.val, longversion?(*contentauthor)->value.lval:0, longversion?(*contentowner)->value.lval:0, lang);
	    if (!(retval_content->value.lval)) success = 0;
	  }
	  if (!success) {
	    if (created0) {
	      php_midgard_delete(retval_content, "page_i", retval_content0->value.lval);
	      PHP_DELETE_REPLIGARD ("page_i", retval_content0->value.lval);
	    }
	    php_midgard_delete(retval_content, "page", newid);
	    PHP_DELETE_REPLIGARD ("page", newid);
	    RETVAL_FALSE_BECAUSE(MGD_ERR_QUOTA);
	  }
	}

#endif /* HAVE_MIDGARD_MULTILANG */

	TOUCH_CACHE;
	}


#if HAVE_MIDGARD_MULTILANG

MGD_FUNCTION(ret_type, create_page_content, (type param))
{
  zval **id, **up, **name, **style, **title, **content, **author, **owner, **auth, **active, **contentauthor, **contentowner, *self;


  int longversion;
  int lang = mgd_lang(mgd_handle());
  longversion = 0;

  RETVAL_FALSE;
  CHECK_MGD;


	if ((self = getThis()) != NULL) {
	  if (ZEND_NUM_ARGS() != 0) {
	    WRONG_PARAM_COUNT;
	  }
	  if (!MGD_PROPFIND(self, "id", id)
	      || !MGD_PROPFIND(self, "up", up)
	      || !MGD_PROPFIND(self, "name", name)
	      || !MGD_PROPFIND(self, "style", style)
	      || !MGD_PROPFIND(self, "title", title)
	      || !MGD_PROPFIND(self, "content", content)
	      || !MGD_PROPFIND(self, "author", author)
	      || !MGD_PROPFIND(self, "owner", owner)
	      || !MGD_PROPFIND(self, "auth", auth)
	      || !MGD_PROPFIND(self, "active", active)
	      || !MGD_PROPFIND(self, "contentauthor", contentauthor)
	      || !MGD_PROPFIND(self, "contentowner", contentowner)
	      ) {
	    RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
	  }
	  longversion = 1;
	} else {
	  switch (ZEND_NUM_ARGS()) {
	  case 3:
	    if (zend_get_parameters_ex(3, &id, &title, &content) == FAILURE)
	      WRONG_PARAM_COUNT;
	    break;
	  case 5:
	    if (zend_get_parameters_ex(5, &id, &title, &content, &contentauthor, &contentowner) == FAILURE)
	      WRONG_PARAM_COUNT;
	    longversion = 1;
	    break;
	  default:
	    WRONG_PARAM_COUNT;
	  }
	}

	convert_to_long_ex(id);
	convert_to_string_ex(title);
	convert_to_string_ex(content);
	if (longversion) {
	  convert_to_long_ex(contentauthor);
	  convert_to_long_ex(contentowner);
	}

	/* only page content owner required */
	if (!ispagecontentowner((*id)->value.lval, lang))
	  RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if (!mgd_exists_id(mgd_handle(), "page", "id=$d", (*id)->value.lval))
	  RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if (mgd_exists_id(mgd_handle(), "page_i", "sid=$d AND lang=$d",
			  (*id)->value.lval, lang))
	  RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);


	if (longversion) {
	  if ((*contentauthor)->value.lval != 0 && !mgd_exists_id(mgd_handle(), "person", "id=$d", (*contentauthor)->value.lval))
	    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	}

	php_midgard_create_page_content_internal(return_value, longversion, (*id)->value.lval, (*title)->value.str.val, (*content)->value.str.val, longversion?(*contentauthor)->value.lval:0, longversion?(*contentowner)->value.lval:0, lang);

   TOUCH_CACHE;
	RETURN_TRUE;
}

#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(ret_type, update_page, (type param))
{
#if ! HAVE_MIDGARD_MULTILANG
	zval **id, **name, **style, **title, **content, **author, **auth, **active, *self;
#else /* HAVE_MIDGARD_MULTILANG */
	zval **id, **name, **style, **title, **content, **author, **owner, **auth, **active, **lang, **contentauthor, **contentowner, *self;

 	midgard_res *res;

	int i_id;
	int longversion, lang_means_something, noowner;
	int lang_i = mgd_lang(mgd_handle());
	longversion = 0; lang_means_something = 0; noowner = 0;
#endif /* HAVE_MIDGARD_MULTILANG */

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "id", id)
		    || !MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "style", style)
		    || !MGD_PROPFIND(self, "title", title)
		    || !MGD_PROPFIND(self, "content", content)
		    || !MGD_PROPFIND(self, "author", author)
#if HAVE_MIDGARD_MULTILANG
		    || !MGD_PROPFIND(self, "owner", owner)
#endif /* HAVE_MIDGARD_MULTILANG */
		    || !MGD_PROPFIND(self, "auth", auth)
		    || !MGD_PROPFIND(self, "active", active)
#if HAVE_MIDGARD_MULTILANG
		    || !MGD_PROPFIND(self, "lang", lang)
		    || !MGD_PROPFIND(self, "contentauthor", contentauthor)
		    || !MGD_PROPFIND(self, "contentowner", contentowner)
#endif /* HAVE_MIDGARD_MULTILANG */
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
#if HAVE_MIDGARD_MULTILANG
		lang_means_something = 1;
		longversion = 1;
#endif /* HAVE_MIDGARD_MULTILANG */
	}
	else {
#if ! HAVE_MIDGARD_MULTILANG
		if (ZEND_NUM_ARGS() != 8
		    || zend_get_parameters_ex(8, &id, &name, &style, &title,
#else /* HAVE_MIDGARD_MULTILANG */
	  switch (ZEND_NUM_ARGS()) {
	  case 8:
	    if (zend_get_parameters_ex(8, &id, &name, &style, &title,
#endif /* HAVE_MIDGARD_MULTILANG */
				     &content, &author, &auth,
				     &active) == FAILURE) WRONG_PARAM_COUNT;
#if HAVE_MIDGARD_MULTILANG
	    noowner = 1;
	    break;
	  case 9:
	    if (zend_get_parameters_ex(9, &id, &name, &style, &title,
				       &content, &author, &owner, &auth,
				       &active) == FAILURE) WRONG_PARAM_COUNT;
	    break;
	  case 11:
	    if (zend_get_parameters_ex(11, &id, &name, &style, &title,
				       &content, &author, &owner, &auth,
				       &active, &contentauthor, &contentowner) == FAILURE) WRONG_PARAM_COUNT;
	    longversion = 1;
	    break;
	  default:
	    WRONG_PARAM_COUNT;
	  }
	}
	if (lang_means_something) {
	  convert_to_long_ex(lang);
	  lang_i = (*lang)->value.lval;
	}
	if (longversion) {
	  convert_to_long_ex(contentauthor);
	  convert_to_long_ex(contentowner);
#endif /* HAVE_MIDGARD_MULTILANG */
	}

	convert_to_long_ex(id);
	convert_to_string_ex(name);
	convert_to_long_ex(style);
	convert_to_string_ex(title);
	convert_to_string_ex(content);
	convert_to_long_ex(author);
#if HAVE_MIDGARD_MULTILANG
	if (!noowner) {
	  convert_to_long_ex(owner);
	}
#endif /* HAVE_MIDGARD_MULTILANG */
	convert_to_long_ex(auth);
	convert_to_long_ex(active);

#if ! HAVE_MIDGARD_MULTILANG
	if (!ispageowner((*id)->value.lval))
#else /* HAVE_MIDGARD_MULTILANG */
	/* need to be owner of both page and page content */
	if (!ispageowner((*id)->value.lval) || !ispagecontentowner((*id)->value.lval, lang_i))
#endif /* HAVE_MIDGARD_MULTILANG */
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if ((*style)->value.lval != 0
         && !mgd_exists_id(mgd_handle(), "style", "id=$d", (*style)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if (!mgd_exists_id(mgd_handle(), "person", "id=$d", (*author)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

   if (mgd_exists_id(mgd_handle(), "page", "up=$d AND name=$q AND id<>$d",
         mgd_idfield(mgd_handle(), "up", "page", (*id)->value.lval),
         (*name)->value.str.val, (*id)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

#if HAVE_MIDGARD_MULTILANG
 	if (longversion) {
 	  if ((*contentauthor)->value.lval != 0 && !mgd_exists_id(mgd_handle(), "person", "id=$d", (*contentauthor)->value.lval))
 	    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
 	}

	res = mgd_ungrouped_select(mgd_handle(), "id", "page_i", "sid=$d AND lang=$d", NULL, (*id)->value.lval, lang_i);

	if (res && mgd_fetch(res)) {
	  i_id = atol(mgd_colvalue(res, 0));
	} else {
	  RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}

	mgd_release(res);
#endif /* HAVE_MIDGARD_MULTILANG */
	php_midgard_update(return_value, "page",
#if ! HAVE_MIDGARD_MULTILANG
			   "name=$q,style=$d,title=$q,changed=Curdate(),"
			   "content=$q,author=$d,info=$d", (*id)->value.lval,
#else /* HAVE_MIDGARD_MULTILANG */
			   "name=$q,style=$d,changed=Curdate(),"
			   "author=$d,owner=$d,info=$d", (*id)->value.lval,
#endif /* HAVE_MIDGARD_MULTILANG */
			   (*name)->value.str.val, (*style)->value.lval,
#if ! HAVE_MIDGARD_MULTILANG
			   (*title)->value.str.val, (*content)->value.str.val,
			   (*author)->value.lval,
#else /* HAVE_MIDGARD_MULTILANG */
		      (*author)->value.lval,			  (noowner)?0:(*owner)->value.lval,
#endif /* HAVE_MIDGARD_MULTILANG */
			   ((*auth)->value.lval == 1) | ((*active)->value.lval == 1) << 1);
	PHP_UPDATE_REPLIGARD("page", (*id)->value.lval);
#if HAVE_MIDGARD_MULTILANG
	php_midgard_update_page_content_internal(return_value,longversion, i_id, (*title)->value.str.val, (*content)->value.str.val, longversion?(*contentauthor)->value.lval:0, longversion?(*contentowner)->value.lval:0);

#endif /* HAVE_MIDGARD_MULTILANG */

   TOUCH_CACHE;
}

#if HAVE_MIDGARD_MULTILANG
MGD_FUNCTION(ret_type, update_page_content, (type param))
{
	zval **id, **title, **content, **lang, **contentauthor, **contentowner, *self;

	midgard_res *res;

	int i_id;
	int longversion, lang_means_something;
	int lang_i = mgd_lang(mgd_handle());
	longversion = 0; lang_means_something = 0;

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "id", id)
		    || !MGD_PROPFIND(self, "title", title)
		    || !MGD_PROPFIND(self, "content", content)
		    || !MGD_PROPFIND(self, "lang", lang)
		    || !MGD_PROPFIND(self, "contentauthor", contentauthor)
		    || !MGD_PROPFIND(self, "contentowner", contentowner)
		   ) {
		  RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
		lang_means_something = 1;
		longversion = 1;
	} else {
	  switch (ZEND_NUM_ARGS()) {
	  case 3:
	    if (zend_get_parameters_ex(3, &id, &title,
				       &content) == FAILURE) WRONG_PARAM_COUNT;
	    break;
	  case 5:
	    if (zend_get_parameters_ex(5, &id, &title,
				       &content, &contentauthor, &contentowner) == FAILURE) WRONG_PARAM_COUNT;

	    longversion = 1;
	    break;
	  default:
	    WRONG_PARAM_COUNT;
	  }
	}

	convert_to_long_ex(id);
	convert_to_string_ex(title);
	convert_to_string_ex(content);
	if (lang_means_something) {
	  convert_to_long_ex(lang);
	  lang_i = (*lang)->value.lval;
	}
	if (longversion) {
	  convert_to_long_ex(contentauthor);
	  convert_to_long_ex(contentowner);
	}

	/* only page content owner required */
	if (!ispagecontentowner((*id)->value.lval, lang_i))
	  RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if (longversion) {
	  if ((*contentauthor)->value.lval != 0 && !mgd_exists_id(mgd_handle(), "person", "id=$d", (*contentauthor)->value.lval))
	    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}

	res = mgd_ungrouped_select(mgd_handle(), "id", "page_i", "sid=$d AND lang=$d", NULL, (*id)->value.lval, lang_i);

	if (res && mgd_fetch(res)) {
	  i_id = atol(mgd_colvalue(res, 0));
	} else {
	  RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}

	mgd_release(res);
	php_midgard_update_page_content_internal(return_value,longversion, i_id, (*title)->value.str.val, (*content)->value.str.val, longversion?(*contentauthor)->value.lval:0, longversion?(*contentowner)->value.lval:0);

   TOUCH_CACHE;
	RETURN_TRUE;
}


#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(ret_type, delete_page, (type param))
{
#if HAVE_MIDGARD_MULTILANG
  midgard_res *res;
  zval *content_return;
  int i_id, lang;
#endif /* HAVE_MIDGARD_MULTILANG */
    IDINIT;
	CHECK_MGD;
    if(mgd_has_dependants(mgd_handle(),id,"page")
	    || mgd_exists_id(mgd_handle(), "page", "up=$d", id)
	    || mgd_exists_id(mgd_handle(), "host", "root=$d", id))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

    if (!ispageowner(id))
	RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	/* check for page elements */
    if (mgd_exists_id(mgd_handle(), "pageelement", "page=$d", id)) RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
#if HAVE_MIDGARD_PAGELINKS
	/* check for page links */
    if (mgd_exists_id(mgd_handle(), "pagelink", "$d IN (up,target)", id))
    RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
#endif

#if HAVE_MIDGARD_MULTILANG
  /* need to be owner of all page contents */
  res = mgd_ungrouped_select(mgd_handle(), "lang", "page_i", "sid=$d", NULL, id);
  if (res) {
    while (mgd_fetch(res)) {
      lang = atol(mgd_colvalue(res,0));
      if (!ispagecontentowner(id, lang))
	RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    }
  }
  mgd_release(res);

  /* need to delete all page contents */
  res = mgd_ungrouped_select(mgd_handle(), "id", "page_i", "sid=$d", NULL, id);
  if (res) {
    while (mgd_fetch(res)) {
      i_id = atol(mgd_colvalue(res,0));
      MAKE_STD_ZVAL(content_return);
      php_midgard_delete(content_return, "page_i", i_id);
      PHP_DELETE_REPLIGARD("page_i", i_id);
    }
  }

  mgd_release(res);
#endif /* HAVE_MIDGARD_MULTILANG */
    php_midgard_delete(return_value, "page", id);
#if HAVE_MIDGARD_MULTILANG

#endif /* HAVE_MIDGARD_MULTILANG */
    PHP_DELETE_REPLIGARD("page", id);

   TOUCH_CACHE;
}

#if HAVE_MIDGARD_MULTILANG

MGD_FUNCTION(ret_type, delete_page_content, (type param))
{
  zval **lang;
  int i_id, lang_i;
  midgard_res* res;
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

    /* only page content owner required */
  if (!ispagecontentowner(id, lang_i))
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

  res = mgd_ungrouped_select(mgd_handle(), "id", "page_i", "sid=$d AND lang=$d", NULL, id, lang_i);

  if (res && mgd_fetch(res)) {
    i_id = atol(mgd_colvalue(res, 0));
  } else {
    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
  }

  mgd_release(res);
  php_midgard_delete(return_value, "page_i", i_id);

  PHP_DELETE_REPLIGARD("page_i", i_id);

}

#endif /* HAVE_MIDGARD_MULTILANG */


MGD_FUNCTION(ret_type, page_has_children, (type param))
{
    IDINIT;
	CHECK_MGD;
   RETVAL_FALSE;
    if (mgd_exists_id(mgd_handle(), "page", "up=$d", id)) { RETVAL_TRUE; }
}

MGD_FUNCTION(ret_type, delete_page_tree, (type param))
{
    IDINIT;
	CHECK_MGD;
   RETVAL_FALSE;
    if (!ispageowner(mgd_idfield(mgd_handle(), "up", "page", id)))
	RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    if(mgd_exists_id(mgd_handle(), "host", "root=$d", id))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
    if(mgd_delete_page(mgd_handle(),  id))
      RETVAL_TRUE;

   TOUCH_CACHE;
}

MGD_MOVE_AND_TOUCH(page,page,page,up,1)

MGD_WALK_FUNCTION(page)

#if ! HAVE_MIDGARD_MULTILANG
MIDGARD_CLASS(MidgardPage, page, midgardpage, page)
#else /* HAVE_MIDGARD_MULTILANG */
MIDGARD_CLASS_I18N(MidgardPage, page, midgardpage, page)
#endif /* HAVE_MIDGARD_MULTILANG */

#endif

