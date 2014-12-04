/* $Id: snippet.c 10324 2006-11-27 12:47:04Z piotras $
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

#if HAVE_MIDGARD_MULTILANG
void php_midgard_create_snippet_content_internal(zval *return_value, int newid, const char* code, const char *doc, const char *author, int lang) {

  php_midgard_create(return_value, 0, "snippet_i",
		     "sid,code,doc,author,lang",
		     "$d,$q,$q,$q,$d",
		     newid, code, doc, author, lang);
  PHP_CREATE_REPLIGARD("snippet_i", return_value->value.lval);
}

void php_midgard_update_snippet_content_internal(zval *return_value, int i_id,  const char* code, const char *doc, const char *author) {

  if (author) {
  php_midgard_update(return_value, "snippet_i",
		       "code=$q,doc=$q,author=$q",
		       i_id,
		       code, doc, author);
  } else {
      php_midgard_update(return_value, "snippet_i",
		       "code=$q,doc=$q",
		       i_id,
		       code, doc);
  }
  PHP_UPDATE_REPLIGARD("snippet_i", i_id);
}

#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(ret_type, snippet_exists, (type param))
{
	zval **file;
	int upval, idval;
	midgard *mgd = mgd_handle(); // DG: GLOBAL(midgard_module_env).mgd;

	RETVAL_FALSE;
	if (ZEND_NUM_ARGS() == 1) {
		if( zend_get_parameters_ex(1,&file) == FAILURE ) {
			WRONG_PARAM_COUNT;
		} else {
			convert_to_string_ex(file);
		}
	} else WRONG_PARAM_COUNT;

	if (MGD_PARSE_COMMON_PATH(mgd, (*file)->value.str.val,
					"snippetdir", "snippet", &idval, &upval))
		return;

	if(idval) RETURN_TRUE;
}

MGD_FUNCTION(ret_type, list_snippets, (type param))
{
#if HAVE_MIDGARD_MULTILANG
  int lang = mgd_lang(mgd_handle());
#endif /* HAVE_MIDGARD_MULTILANG */
    IDINIT;
	CHECK_MGD;
#if ! HAVE_MIDGARD_MULTILANG
    php_midgard_select(&MidgardSnippet, return_value, "id,name,author,creator,created,revisor,revised,revision" SITEGROUP_SELECT, "snippet", "up=$d", "name", id);
#else /* HAVE_MIDGARD_MULTILANG */
    php_midgard_select(&MidgardSnippet, return_value, "snippet.id AS id,name,lang,author,creator,created,revisor,revised,revision,snippet.sitegroup as sitegroup", "snippet, snippet_i", "up=$d AND snippet.id=snippet_i.sid AND snippet_i.lang=$d", "name", id, lang);
#endif /* HAVE_MIDGARD_MULTILANG */
}

MGD_FUNCTION(ret_type, get_snippet, (type param))
{
	zval **id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	case 0:
		php_midgard_bless(return_value, &MidgardSnippet);
#if ! HAVE_MIDGARD_MULTILANG
		mgd_object_init(return_value, "up", "name", "code", "doc", "author", NULL);
#else /* HAVE_MIDGARD_MULTILANG */
		mgd_object_init(return_value, "up", "name", "code", "doc", "author", "lang", NULL);
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

	if(!(*id)->value.lval)
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

    php_midgard_get_object(return_value, MIDGARD_OBJECT_SNIPPET, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, get_snippet_by_name, (type param))
{
   zval **snippetdir, **name;
   int objid;

   CHECK_MGD;

   if (ZEND_NUM_ARGS() != 2
         || zend_get_parameters_ex(2, &snippetdir, &name) != SUCCESS) {
      WRONG_PARAM_COUNT;
   }

   convert_to_long_ex(snippetdir);
   convert_to_string_ex(name);

   if(!(*snippetdir)->value.lval || !(*name)->value.str.len)
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

    objid = mgd_exists_id(mgd_handle(), "snippet", "up=$d AND name=$q",
      (*snippetdir)->value.lval, (*name)->value.str.val);
    php_midgard_get_object(return_value, MIDGARD_OBJECT_SNIPPET, objid);
}

MGD_FUNCTION(ret_type, create_snippet, (type param))
{
	zval **snippetdir, **name, **code, **doc, **author, *self;
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

		if (!MGD_PROPFIND(self, "up", snippetdir)
		    || !MGD_PROPFIND(self, "name", name)
		    || !MGD_PROPFIND(self, "code", code)
		    || !MGD_PROPFIND(self, "doc", doc)
		    || !MGD_PROPFIND(self, "author", author)
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
		self = NULL;
		if (ZEND_NUM_ARGS() != 5
		    || zend_get_parameters_ex(5, &snippetdir, &name, &code, &doc,
				     &author) == FAILURE)
			WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(snippetdir);
	convert_to_string_ex(name);
	convert_to_string_ex(code);
	convert_to_string_ex(doc);
	convert_to_string_ex(author);

	if (!mgd_exists_id(mgd_handle(), "snippetdir", "id=$d", (*snippetdir)->value.lval))
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if (!issnippetdirowner((*snippetdir)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

   if (mgd_exists_id(mgd_handle(), "snippet", "up=$d AND name=$q",
			     (*snippetdir)->value.lval, (*name)->value.str.val))
		RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

	php_midgard_create(return_value, self, "snippet",
#if ! HAVE_MIDGARD_MULTILANG
			   "up,name,code,doc,author,creator,created,revisor,revised,revision",
			   "$d,$q,$q,$q,$q,$d,Now(),$d,Now(),0",
#else /* HAVE_MIDGARD_MULTILANG */
			   "up,name,creator,created,revisor,revised,revision",
			   "$d,$q,$d,Now(),$d,Now(),0",
#endif /* HAVE_MIDGARD_MULTILANG */
			   (*snippetdir)->value.lval, (*name)->value.str.val,
#if ! HAVE_MIDGARD_MULTILANG
			   (*code)->value.str.val, (*doc)->value.str.val,
			   (*author)->value.str.val, mgd_user(mgd_handle()),
#else /* HAVE_MIDGARD_MULTILANG */
			   mgd_user(mgd_handle()),
#endif /* HAVE_MIDGARD_MULTILANG */
			   mgd_user(mgd_handle()));

	PHP_CREATE_REPLIGARD("snippet", return_value->value.lval);
#if HAVE_MIDGARD_MULTILANG
      newid = return_value->value.lval;
	if (newid) {
	if (lang > 0) {
	    php_midgard_create_snippet_content_internal(retval_content0 ,return_value->value.lval, "", "", "", 0);
	    if (!(retval_content0->value.lval)) success = 0;
	    else created0 = 1;
	  }
	  if (success) {
	    php_midgard_create_snippet_content_internal(retval_content,return_value->value.lval, (*code)->value.str.val, (*doc)->value.str.val, (*author)->value.str.val, lang);
	    if (!(retval_content->value.lval)) success = 0;
	  }
	  if (!success) {
	    if (created0) {
	      php_midgard_delete(retval_content, "snippet_i", retval_content0->value.lval);
	      PHP_DELETE_REPLIGARD ("snippet_i", retval_content0->value.lval);
	    }
	    php_midgard_delete(retval_content, "snippet", newid);
	    PHP_DELETE_REPLIGARD ("snippet", newid);
	    RETVAL_FALSE_BECAUSE(MGD_ERR_QUOTA);
	}
	}
}

MGD_FUNCTION(ret_type, create_snippet_content, (type param))
{
	zval **id, **code, **doc, **author, *self;
	int snippetdir_i;
	int lang = mgd_lang(mgd_handle());

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 0) {
			WRONG_PARAM_COUNT;
		}

		if (!MGD_PROPFIND(self, "id", id)
		    || !MGD_PROPFIND(self, "code", code)
		    || !MGD_PROPFIND(self, "doc", doc)
		    || !MGD_PROPFIND(self, "author", author)
		    ) {
		  RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	}
	else {
	  self = NULL;
		if (ZEND_NUM_ARGS() != 4
		    || zend_get_parameters_ex(4, &id, &code, &doc, &author) == FAILURE)
		  WRONG_PARAM_COUNT;
	}

	convert_to_string_ex(code);
	convert_to_string_ex(doc);
	convert_to_string_ex(author);

	snippetdir_i = mgd_idfield(mgd_handle(), "snippetdir", "snippet", (*id)->value.lval);

	if (!issnippetdirowner(snippetdir_i))
	  RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

	if (!mgd_exists_id(mgd_handle(), "snippet", "id=$d", (*id)->value.lval))
	  RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if (mgd_exists_id(mgd_handle(), "snippet_i", "sid=$d AND lang=$d",
			  (*id)->value.lval, lang))
	  RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

	php_midgard_create_snippet_content_internal(return_value,(*id)->value.lval, (*code)->value.str.val, (*doc)->value.str.val, (*author)->value.str.val, lang);
	RETURN_TRUE;

#endif /* HAVE_MIDGARD_MULTILANG */
}

MGD_FUNCTION(ret_type, update_snippet, (type param))
{
#if ! HAVE_MIDGARD_MULTILANG
	zval **id, **name, **code, **doc, **author, *self;
#else /* HAVE_MIDGARD_MULTILANG */
  zval **id, **name, **code, **doc, **author, **lang, *self;
  midgard_res * res;
#endif /* HAVE_MIDGARD_MULTILANG */
	int up;
#if ! HAVE_MIDGARD_MULTILANG

#else /* HAVE_MIDGARD_MULTILANG */
  int i_id;
  int lang_means_something;
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
		    || !MGD_PROPFIND(self, "code", code)
		    || !MGD_PROPFIND(self, "doc", doc)
#if HAVE_MIDGARD_MULTILANG
	|| !MGD_PROPFIND(self, "lang", lang)
#endif /* HAVE_MIDGARD_MULTILANG */
		   ) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
#if HAVE_MIDGARD_MULTILANG
    lang_means_something = 1;
#endif /* HAVE_MIDGARD_MULTILANG */

	convert_to_long_ex(id);
	convert_to_string_ex(name);
	convert_to_string_ex(code);
	convert_to_string_ex(doc);
#if HAVE_MIDGARD_MULTILANG
    convert_to_long_ex(lang);
    lang_i = (*lang)->value.lval;
#endif /* HAVE_MIDGARD_MULTILANG */
	author = NULL;
      if (mgd_isadmin(mgd_handle())) {
         if (!MGD_PROPFIND(self, "author", author)) {
            RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
         }
	convert_to_string_ex(author);
      }
#if ! HAVE_MIDGARD_MULTILANG
	}
	else {
#else /* HAVE_MIDGARD_MULTILANG */
  } else {
#endif /* HAVE_MIDGARD_MULTILANG */
		switch (ZEND_NUM_ARGS()) {
			case 4:
				if (zend_get_parameters_ex
				    (4, &id, &name, &code, &doc) == FAILURE) {
					WRONG_PARAM_COUNT;
				}
				convert_to_long_ex(id);
				convert_to_string_ex(name);
				convert_to_string_ex(code);
				convert_to_string_ex(doc);
				author = NULL;
				break;

			case 5:
				if (!mgd_isadmin(mgd_handle())
				    || zend_get_parameters_ex(5, &id, &name, &code,
						     &doc, &author) == FAILURE) {
					WRONG_PARAM_COUNT;
				}
				convert_to_long_ex(id);
				convert_to_string_ex(name);
				convert_to_string_ex(code);
				convert_to_string_ex(doc);
				convert_to_string_ex(author);
				break;

			default:
				WRONG_PARAM_COUNT;
		}
	}

	if (!mgd_exists_id(mgd_handle(), "snippet", "id=$d", (*id)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if (!issnippetdirowner
	    (up =
	     mgd_idfield(mgd_handle(), "up", "snippet",
			 (*id)->value.lval)))
	      RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	if (mgd_exists_id
	    (mgd_handle(), "snippet", "id!=$d AND up=$d AND name=$q",
	     (*id)->value.lval, up, (*name)->value.str.val))
		RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

#if ! HAVE_MIDGARD_MULTILANG
	if (author)
		php_midgard_update(return_value, "snippet",
				   "name=$q,code=$q,doc=$q,author=$q,revisor=$d,revised=Now(),revision=revision+1",
				   (*id)->value.lval, (*name)->value.str.val,
				   (*code)->value.str.val, (*doc)->value.str.val,
				   (*author)->value.str.val,
				   mgd_user(mgd_handle()));
	else
#else /* HAVE_MIDGARD_MULTILANG */

  res = mgd_ungrouped_select(mgd_handle(), "id", "snippet_i", "sid=$d AND lang=$d", NULL, (*id)->value.lval, lang_i);

  if (res && mgd_fetch(res)) {
    i_id = atol(mgd_colvalue(res, 0));
  } else {
    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
  }
#endif /* HAVE_MIDGARD_MULTILANG */
		php_midgard_update(return_value, "snippet",
#if ! HAVE_MIDGARD_MULTILANG
				   "name=$q,code=$q,doc=$q,revisor=$d,revised=Now(),revision=revision+1",
#else /* HAVE_MIDGARD_MULTILANG */
		     "name=$q,revisor=$d,revised=Now(),revision=revision+1",
#endif /* HAVE_MIDGARD_MULTILANG */
				   (*id)->value.lval, (*name)->value.str.val,
#if ! HAVE_MIDGARD_MULTILANG
				   (*code)->value.str.val, (*doc)->value.str.val,
#endif /* not HAVE_MIDGARD_MULTILANG */
				   mgd_user(mgd_handle()));
	PHP_UPDATE_REPLIGARD("snippet", (*id)->value.lval);
#if HAVE_MIDGARD_MULTILANG
  php_midgard_update_snippet_content_internal(return_value,i_id, (*code)->value.str.val, (*doc)->value.str.val, author?(*author)->value.str.val:NULL);
#endif /* HAVE_MIDGARD_MULTILANG */
}

#if HAVE_MIDGARD_MULTILANG

MGD_FUNCTION(ret_type, update_snippet_content, (type param))
{
  zval **id, **code, **doc, **author, **lang, *self;
  midgard_res * res;
  int up;
  int i_id;
  int lang_means_something;
  int lang_i = mgd_lang(mgd_handle());
  lang_means_something = 0;
  RETVAL_FALSE;
  CHECK_MGD;

  if ((self = getThis()) != NULL) {
    if (ZEND_NUM_ARGS() != 0) {
      WRONG_PARAM_COUNT;
    }

    if (!MGD_PROPFIND(self, "id", id)
	|| !MGD_PROPFIND(self, "code", code)
	|| !MGD_PROPFIND(self, "doc", doc)
	|| !MGD_PROPFIND(self, "lang", lang)
	) {
      RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
    }
    lang_means_something = 1;

    convert_to_long_ex(id);
    convert_to_string_ex(code);
    convert_to_string_ex(doc);
    convert_to_long_ex(lang);
    lang_i = (*lang)->value.lval;
    author = NULL;
    if (mgd_isadmin(mgd_handle())) {
      if (!MGD_PROPFIND(self, "author", author)) {
	RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
      }
      convert_to_string_ex(author);
    }
  } else {
    switch (ZEND_NUM_ARGS()) {
    case 3:
      if (zend_get_parameters_ex
	  (3, &id, &code, &doc) == FAILURE) {
	WRONG_PARAM_COUNT;
      }
      convert_to_long_ex(id);
      convert_to_string_ex(code);
      convert_to_string_ex(doc);
      author = NULL;
      break;

    case 4:
      if (!mgd_isadmin(mgd_handle())
	  || zend_get_parameters_ex(4, &id, &code,
				    &doc, &author) == FAILURE) {
	WRONG_PARAM_COUNT;
      }
      convert_to_long_ex(id);
      convert_to_string_ex(code);
      convert_to_string_ex(doc);
      convert_to_string_ex(author);
      break;

    default:
      WRONG_PARAM_COUNT;
    }
  }

  if (!mgd_exists_id(mgd_handle(), "snippet", "id=$d", (*id)->value.lval))
    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

  if (!issnippetdirowner
      (up =
       mgd_idfield(mgd_handle(), "up", "snippet",
		   (*id)->value.lval)))
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
  res = mgd_ungrouped_select(mgd_handle(), "id", "snippet_i", "sid=$d AND lang=$d", NULL, (*id)->value.lval, lang_i);

  if (res && mgd_fetch(res)) {
    i_id = atol(mgd_colvalue(res, 0));
  } else {
    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
}
  php_midgard_update_snippet_content_internal(return_value,i_id, (*code)->value.str.val, (*doc)->value.str.val, author?(*author)->value.str.val:NULL);
  RETURN_TRUE;
}


#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(ret_type, delete_snippet, (type param))
{
#if HAVE_MIDGARD_MULTILANG
  zval *content_return;
  midgard_res *res;
  int i_id;
#endif /* HAVE_MIDGARD_MULTILANG */
	IDINIT;
	CHECK_MGD;

	if (mgd_has_dependants(mgd_handle(), id, "snippet"))
		RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

	if (!issnippetdirowner(mgd_idfield(mgd_handle(), "up", "snippet", id)))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

#if HAVE_MIDGARD_MULTILANG
  /* need to delete all snippet content */
  res = mgd_ungrouped_select(mgd_handle(), "id", "snippet_i", "sid=$d", NULL, id);
  if (res) {
    while (mgd_fetch(res)) {
      i_id = atol(mgd_colvalue(res,0));
      MAKE_STD_ZVAL(content_return);
      php_midgard_delete(content_return, "snippet_i", i_id);
      PHP_DELETE_REPLIGARD("snippet_i", i_id);
    }
  }
#endif /* HAVE_MIDGARD_MULTILANG */
	php_midgard_delete(return_value, "snippet", id);
	PHP_DELETE_REPLIGARD("snippet", id);
}

#if HAVE_MIDGARD_MULTILANG
MGD_FUNCTION(ret_type, delete_snippet_content, (type param))
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

  if (!issnippetdirowner(mgd_idfield(mgd_handle(), "up", "snippet", id)))
    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

  res = mgd_ungrouped_select(mgd_handle(), "id", "snippet_i", "sid=$d AND lang=$d", NULL, id, lang_i);

  if (res && mgd_fetch(res)) {
    i_id = atol(mgd_colvalue(res, 0));
  } else {
    RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
  }

  mgd_release(res);
  php_midgard_delete(return_value, "snippet_i", i_id);

  PHP_DELETE_REPLIGARD("snippet_i", i_id);
}


#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(ret_type, copy_snippet, (int id[, int newsnippetdir]))
{
	zval **id, **newsnippetdir;
	int r_id;
	RETVAL_FALSE;
	CHECK_MGD;
	switch (ZEND_NUM_ARGS()) {
		case 2:
			if (zend_get_parameters_ex(2, &id, &newsnippetdir) !=
			    SUCCESS) WRONG_PARAM_COUNT;
			break;
		case 1:
			if (zend_get_parameters_ex(1, &id) != SUCCESS)
				WRONG_PARAM_COUNT;
			newsnippetdir = NULL;
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(id);
	if (newsnippetdir)
		convert_to_long_ex(newsnippetdir);

	/* newsnippetdir must be in same SG or be 0 */
	if (newsnippetdir && !mgd_exists_bool(mgd_handle(), "snippetdir,snippet",
				    "snippetdir.id=$d AND snippet.id=$d"
				    " AND (snippetdir.sitegroup=snippet.sitegroup"
				    " OR snippetdir.sitegroup=0"
				    " OR snippet.sitegroup=0)",
				    (*newsnippetdir)->value.lval, (*id)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);

	r_id = mgd_copy_snippet(mgd_handle(), (*id)->value.lval,
				newsnippetdir ? (*newsnippetdir)->value.lval : 0);
#if HAVE_MIDGARD_QUOTA
	if (mgd_get_quota_error(mgd_handle())) {
	  mgd_set_quota_error(mgd_handle(), 0);
	  RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
	}
#endif /* HAVE_MIDGARD_QUOTA */

	RETVAL_LONG(r_id);

}

MGD_FUNCTION(ret_type, get_snippet_by_path, (type param))
{
  zval **path;
  int id, up;
  
  CHECK_MGD;

  switch (ZEND_NUM_ARGS()) {
    case 0:
    WRONG_PARAM_COUNT;

    case 1:
    if (zend_get_parameters_ex(1, &path) == FAILURE) {
      WRONG_PARAM_COUNT;
    }
    convert_to_string_ex(path);
    break;

    default:
    WRONG_PARAM_COUNT;
  }

  if(!MGD_PARSE_COMMON_PATH(mgd_handle(), (*path)->value.str.val, "snippetdir", "snippet", &id, &up)) {
    php_midgard_get_object(return_value, MIDGARD_OBJECT_SNIPPET, id);
    return;
  }
}



MGD_MOVE_FUNCTION(snippet,snippetdir,snippet,up);

#if ! HAVE_MIDGARD_MULTILANG
MIDGARD_CLASS(MidgardSnippet, snippet, midgardsnippet, snippet)
#else /* HAVE_MIDGARD_MULTILANG */
MIDGARD_CLASS_I18N(MidgardSnippet, snippet, midgardsnippet, snippet)
#endif /* HAVE_MIDGARD_MULTILANG */

#endif
