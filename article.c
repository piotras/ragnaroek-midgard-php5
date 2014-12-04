/* $Id: article.c 10324 2006-11-27 12:47:04Z piotras $
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
#include "mgd_article.h"
#include <midgard/pageresolve.h>

#ifdef PHP_MIDGARD_LEGACY_API

#if HAVE_MIDGARD_MULTILANG
void php_midgard_create_article_content_internal(zval *return_value, int longversion, int newid, const char* title, const char *abstract, const char *content, const char *url, int contentauthor, int lang) {

  if (longversion) {
	php_midgard_create(return_value, 0, "article_i",
			   "sid,title,abstract,content,url,created,author,lang",
			   "$d,$q,$q,$q,$q,Now(),$d,$d",
			   newid, title, abstract, content, url, contentauthor, lang);
  } else {
	php_midgard_create(return_value, 0, "article_i",
			   "sid,title,abstract,content,url,created,lang",
			 "$d,$q,$q,$q,$q,Now(),$d",
			   newid, title, abstract, content, url, lang);
  }
  PHP_CREATE_REPLIGARD("article_i", return_value->value.lval);
}

void php_midgard_update_article_content_internal(zval *return_value, int longversion, int i_id, const char* title, const char *abstract, const char *content, const char *url, int contentauthor) {

  if (longversion) {
	php_midgard_update(return_value, "article_i",
			   "title=$q,abstract=$q,content=$q,url=$q,author=$d",
			   i_id,
			   title, abstract, content, url, contentauthor);
  } else {
	php_midgard_update(return_value, "article_i",
			   "title=$q,abstract=$q,content=$q,url=$q",
			   i_id,
			   title, abstract, content, url);
  }
  PHP_UPDATE_REPLIGARD("article_i", i_id);
}



#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(bool, is_article_owner, (int id))
{
	IDINIT;
	CHECK_MGD;
	RETVAL_LONG(isarticleowner(id));
}

MGD_FUNCTION(bool, is_article_in_topic_tree, (int topic_root, int id))
{
	zval **root, **article;

	RETVAL_FALSE;
	CHECK_MGD;
	if (ZEND_NUM_ARGS() != 2
		|| zend_get_parameters_ex(2, &root, &article) != SUCCESS)
		WRONG_PARAM_COUNT;
	convert_to_long_ex(root);
	convert_to_long_ex(article);

	if((*article)->value.lval == 0 || /* useless to waste time if article=0 */
				!mgd_exists_id(mgd_handle(),
						"article", "id=$d",
						(*article)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	if ((*root)->value.lval == 0)
		RETURN_TRUE; /* always true if topic = 0 */
	if(!mgd_exists_id(mgd_handle(), "topic", "id=$d", (*root)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

#if 0
	if (!isarticlereader((*article)->value.lval))
		return;
#endif

	if(mgd_is_in_tree(mgd_handle(), "topic", "up", (*root)->value.lval,
				mgd_idfield(mgd_handle(), "topic", "article",
						(*article)->value.lval)))
		RETURN_TRUE;
}

const char *article_sort(const char *order)
{
	static struct
	{
		const char *order, *sort;
	} sort[] =
	{
		{ "calendar",			"article.calstart" },
		{ "reverse calendar",	"article.calstart DESC" },
	  { "alpha",			  "title ASC" },
	  { "reverse alpha",	  "title DESC" },
	  { "name",				  "name ASC" },
	  { "reverse name",		  "name DESC" },
	  { "score",			  "article.score ASC,title ASC" },
	  { "reverse score",	  "article.score DESC, title ASC" },
	  { "revised",			  "article.revised ASC" },
	  { "reverse revised",	  "article.revised DESC" },
	  { "created",			  "article.created ASC" },
	  { "reverse created",	  "article.created DESC" },
	  { NULL,				  "article.created DESC" }
	};
	int i;

	for (i = 0; sort[i].order; i++)
		if (strcmp(order, sort[i].order) == 0)
			return sort[i].sort;

	return sort[i].sort;
}

MGD_FUNCTION(mixed, list_topic_articles, (int id, [string sort, [int type, [int up]]]))
{
	const char *sortv = NULL;
	int typev = 0;
	zval **id, **sortn, **typen, **upn;
	int up = 0, up_wild = 0;
    
	RETVAL_FALSE;
	CHECK_MGD;
	switch (ZEND_NUM_ARGS()) {
		case 4:
			if (zend_get_parameters_ex(4, &id, &sortn, &typen, &upn)
				== SUCCESS) {
				convert_to_long_ex(id);
				convert_to_string_ex(sortn);
				convert_to_long_ex(typen);
				convert_to_long_ex(upn);
				sortv = article_sort((*sortn)->value.str.val);
				typev = (*typen)->value.lval;
				up = (*upn)->value.lval;
				up_wild = (up == 0 ? 1 : 0);
				break;
			}
		case 3:
			if (zend_get_parameters_ex(3, &id, &sortn, &typen) ==
				SUCCESS) {
				convert_to_long_ex(id);
				convert_to_string_ex(sortn);
				convert_to_long_ex(typen);
				sortv = article_sort((*sortn)->value.str.val);
				typev = (*typen)->value.lval;
				up = 0;
				up_wild = 0;
				break;
			}
		case 2:
			if (zend_get_parameters_ex(2, &id, &sortn) == SUCCESS) {
				convert_to_long_ex(id);
				convert_to_string_ex(sortn);
				sortv = article_sort((*sortn)->value.str.val);
				typev = -1;
				up = 0;
				up_wild = 0;
				break;
			}
		case 1:
			if (zend_get_parameters_ex(1, &id) == SUCCESS) {
				convert_to_long_ex(id);
				sortv = "article.created DESC";
				typev = -1;
				up = 0;
				up_wild = 0;
				break;
			}
		default:
			WRONG_PARAM_COUNT;
	}

    if(mgd_lang(mgd_handle()) > 0) {
        sortv = g_strconcat(sortv, ", lang ASC", NULL);
    }
    
	if (typev == -1)
		php_midgard_select(&MidgardArticle, return_value,
			   ARTICLE_SELECT, ARTICLE_FROM,
#if ! HAVE_MIDGARD_MULTILANG
				   "article.topic=$d AND author=person.id"
				   " AND (article.up=$d OR 1=$d)",
				   sortv, (*id)->value.lval, up, up_wild);
#else /* HAVE_MIDGARD_MULTILANG */
				   "article.topic=$d AND article.author=person.id"
				   " AND (article.up=$d OR 1=$d)"
				   ARTICLE_I_WHERE,
					 sortv, (*id)->value.lval, up, up_wild,mgd_lang(mgd_handle()));
#endif /* HAVE_MIDGARD_MULTILANG */
	else
		php_midgard_select(&MidgardArticle, return_value,
			   ARTICLE_SELECT, ARTICLE_FROM,
				   "article.type=$d AND article.topic=$d"
				   " AND (article.up=$d OR 1=$d)"
#if ! HAVE_MIDGARD_MULTILANG
				   " AND author=person.id",
				   sortv, typev, (*id)->value.lval, up,
				   up_wild);
#else /* HAVE_MIDGARD_MULTILANG */
				   " AND article.author=person.id"
				   ARTICLE_I_WHERE,
				   sortv, typev, (*id)->value.lval, up,
				   up_wild, mgd_lang(mgd_handle()));
#endif /* HAVE_MIDGARD_MULTILANG */
}

MGD_FUNCTION(mixed, list_reply_articles, (int id))
{
	IDINIT;
	CHECK_MGD;
	php_midgard_select(&MidgardArticle, return_value,
			ARTICLE_SELECT, ARTICLE_FROM,
#if ! HAVE_MIDGARD_MULTILANG
			   "article.up=$d AND author=person.id",
		  "article.created DESC", id);
#else /* HAVE_MIDGARD_MULTILANG */
			   "article.up=$d AND article.author=person.id"
			   ARTICLE_I_WHERE,
		  "article.created DESC", id, mgd_lang(mgd_handle()));
#endif /* HAVE_MIDGARD_MULTILANG */
}

MGD_FUNCTION(mixed, list_topic_articles_all, (int id, [string sort, [int type, [int up]]]))
{
	int *topics;
	const char *sortv = NULL;
	int typev = 0;
	zval **id, **sortn, **typen, **upn;
	int up = 0, up_wild = 0;

	RETVAL_FALSE;
	CHECK_MGD;
	switch (ZEND_NUM_ARGS()) {
		case 4:
			if (zend_get_parameters_ex(4, &id, &sortn, &typen, &upn)
				== SUCCESS) {
				convert_to_long_ex(id);
				convert_to_string_ex(sortn);
				convert_to_long_ex(typen);
				convert_to_long_ex(upn);
				sortv = article_sort((*sortn)->value.str.val);
				typev = (*typen)->value.lval;
				up = (*upn)->value.lval;
				up_wild = (up == 0 ? 1 : 0);
				break;
			}
		case 3:
			if (zend_get_parameters_ex(3, &id, &sortn, &typen) ==
				SUCCESS) {
				convert_to_long_ex(id);
				convert_to_string_ex(sortn);
				convert_to_long_ex(typen);
				sortv = article_sort((*sortn)->value.str.val);
				typev = (*typen)->value.lval;
				up = 0;
				up_wild = 0;
				break;
			}
		case 2:
			if (zend_get_parameters_ex(2, &id, &sortn) == SUCCESS) {
				convert_to_long_ex(id);
				convert_to_string_ex(sortn);
				sortv = article_sort((*sortn)->value.str.val);
				typev = -1;
				up = 0;
				up_wild = 0;
				break;
			}
		case 1:
			if (zend_get_parameters_ex(1, &id) == SUCCESS) {
				convert_to_long_ex(id);
				sortv = "article.created DESC";
				typev = -1;
				up = 0;
				up_wild = 0;
				break;
			}
		default:
			WRONG_PARAM_COUNT;
	}

	topics = mgd_tree(mgd_handle(), "topic", "up", (*id)->value.lval, 0, NULL);

	if(topics) {
		if (typev == -1)
			php_midgard_select(&MidgardArticle, return_value,
				   ARTICLE_SELECT,
					   ARTICLE_FROM,
#if ! HAVE_MIDGARD_MULTILANG
					   "article.topic IN $D AND author=person.id"
					   " AND (article.up=$d OR 1=$d)",
					   sortv, topics, up, up_wild);
#else /* HAVE_MIDGARD_MULTILANG */
					   "article.topic IN $D AND article.author=person.id"
					   " AND (article.up=$d OR 1=$d)"
					   ARTICLE_I_WHERE,
					   sortv, topics, up, up_wild, mgd_lang(mgd_handle()));
#endif /* HAVE_MIDGARD_MULTILANG */
		else
			php_midgard_select(&MidgardArticle, return_value,
				   ARTICLE_SELECT,
					   ARTICLE_FROM,
					   "article.type=$d AND article.topic IN $D"
					   " AND (article.up=$d OR 1=$d)"
#if ! HAVE_MIDGARD_MULTILANG
					   " AND author=person.id",
					   sortv, typev, topics, up, up_wild);
#else /* HAVE_MIDGARD_MULTILANG */
					   " AND article.author=person.id"
								   ARTICLE_I_WHERE,
					   sortv, typev, topics, up, up_wild, mgd_lang(mgd_handle()));
#endif /* HAVE_MIDGARD_MULTILANG */
		free(topics);
	}
}

MGD_FUNCTION(mixed, list_topic_articles_all_fast, (int id, [string sort, [int type, [int up]]]))
{
	int *topics;
	const char *sortv = NULL;
	int typev = 0;
	zval **id, **sortn, **typen, **upn;
	int up = 0, up_wild = 0;

	RETVAL_FALSE;
	CHECK_MGD;
	switch (ZEND_NUM_ARGS()) {
		case 4:
			if (zend_get_parameters_ex(4, &id, &sortn, &typen, &upn)
				== SUCCESS) {
				convert_to_long_ex(id);
				convert_to_string_ex(sortn);
				convert_to_long_ex(typen);
				sortv = article_sort((*sortn)->value.str.val);
				typev = (*typen)->value.lval;
				up = (*upn)->value.lval;
				up_wild = (up == 0 ? 1 : 0);
				break;
			}
		case 3:
			if (zend_get_parameters_ex(3, &id, &sortn, &typen) ==
				SUCCESS) {
				convert_to_long_ex(id);
				convert_to_string_ex(sortn);
				convert_to_long_ex(typen);
				sortv = article_sort((*sortn)->value.str.val);
				typev = (*typen)->value.lval;
				up = 0;
				up_wild = 0;
				break;
			}
		case 2:
			if (zend_get_parameters_ex(2, &id, &sortn) == SUCCESS) {
				convert_to_long_ex(id);
				convert_to_string_ex(sortn);
				sortv = article_sort((*sortn)->value.str.val);
				typev = -1;
				up = 0;
				up_wild = 0;
				break;
			}
		case 1:
			if (zend_get_parameters_ex(1, &id) == SUCCESS) {
				convert_to_long_ex(id);
				sortv = "article.created DESC";
				typev = -1;
				up = 0;
				up_wild = 0;
				break;
			}
		default:
			WRONG_PARAM_COUNT;
	}

	topics = mgd_tree(mgd_handle(), "topic", "up", (*id)->value.lval, 0, NULL);

	if(topics) {
		if (typev == -1)
			php_midgard_select(&MidgardArticle, return_value,
					   ARTICLE_SELECT_FAST,
					   ARTICLE_FROM_FAST,
					   "article.topic IN $D"
#if ! HAVE_MIDGARD_MULTILANG
					   " AND (article.up=$d OR 1=$d)",
					   sortv, topics, up, up_wild);
#else /* HAVE_MIDGARD_MULTILANG */
					   " AND (article.up=$d OR 1=$d)"
					   ARTICLE_I_WHERE,
					   sortv, topics, up, up_wild, mgd_lang(mgd_handle()));
#endif /* HAVE_MIDGARD_MULTILANG */
		else
			php_midgard_select(&MidgardArticle, return_value,
					   ARTICLE_SELECT_FAST,
					   ARTICLE_FROM_FAST,
					   "article.type=$d AND article.topic IN $D"
#if ! HAVE_MIDGARD_MULTILANG
					   " AND (article.up=$d OR 1=$d)",
					   sortv, typev, topics, up, up_wild);
#else /* HAVE_MIDGARD_MULTILANG */
					   " AND (article.up=$d OR 1=$d)"
					   ARTICLE_I_WHERE,
					   sortv, typev, topics, up, up_wild, mgd_lang(mgd_handle()));
#endif /* HAVE_MIDGARD_MULTILANG */
		free(topics);
	}
}

MGD_FUNCTION(mixed, list_topic_articles_all_of_person, (int topic, int person))
{
	int *topics;
	zval **topic, **person;
	RETVAL_FALSE;
	CHECK_MGD;
	if (ZEND_NUM_ARGS() != 2
		|| zend_get_parameters_ex(2, &topic, &person) != SUCCESS)
		WRONG_PARAM_COUNT;
	convert_to_long_ex(topic);
	convert_to_long_ex(person);

	topics =
	   mgd_tree(mgd_handle(), "topic", "up", (*topic)->value.lval, 0, NULL);

	if(topics) {
		php_midgard_select(&MidgardArticle, return_value,
				ARTICLE_SELECT, ARTICLE_FROM,
#if ! HAVE_MIDGARD_MULTILANG
				   "article.topic IN $D AND author=person.id"
				   " AND author=$d AND article.up=0",
				   "article.created DESC", topics,
				   (*person)->value.lval);
#else /* HAVE_MIDGARD_MULTILANG */
				   "article.topic IN $D AND article.author=person.id"
				   " AND article.author=$d AND article.up=0"
				   ARTICLE_I_WHERE,
				   "article.created DESC", topics,
				   (*person)->value.lval, mgd_lang(mgd_handle()));
#endif /* HAVE_MIDGARD_MULTILANG */
		free(topics);
	}
}

MGD_FUNCTION(mixed, get_article, ([int id]))
{
	zval **id, **name;
   int objid;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
		case 0:
			php_midgard_bless(return_value, &MidgardArticle);
			mgd_object_init(return_value, "up", "topic", "title",
					 "abstract", "content", "author",
					 "created", "url", "calstart",
					 "caldays", "icon", "view", "print",
					 "extra1", "extra2", "extra3", "name",
					 "creator", "revisor", "revision",
					 "approver", "revised", "approved",
					 "score", "type", "locked", "locker",
#if HAVE_MIDGARD_MULTILANG
					"lang", "contentauthor",
#endif /* HAVE_MIDGARD_MULTILANG */
					 NULL);
			return;
		case 1:
			if (zend_get_parameters_ex(1, &id) == FAILURE) { WRONG_PARAM_COUNT; }
			convert_to_long_ex(id);
		 objid = (*id)->value.lval;
			break;

	  case 2:
			if (zend_get_parameters_ex(2, &id, &name) == FAILURE) {
			   WRONG_PARAM_COUNT;
		 }
		 convert_to_long_ex(id);
		 convert_to_string_ex(name);
		 objid = mgd_exists_id(mgd_handle(), "article", "topic=$d AND name=$q",
			(*id)->value.lval, (*name)->value.str.val);
		 break;

		default:
			WRONG_PARAM_COUNT;
	}

   php_midgard_get_object(return_value, MIDGARD_OBJECT_ARTICLE, objid);
}

MGD_FUNCTION(mixed, get_reply_by_name, ([int article, string name]))
{
	zval **id, **name;
   int objid;

	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
		case 2:
			if (zend_get_parameters_ex(2, &id, &name) == FAILURE) {
			   WRONG_PARAM_COUNT;
		 }

			convert_to_long_ex(id);
			convert_to_string_ex(name);
			break;

		default:
			WRONG_PARAM_COUNT;
	}

   objid = mgd_exists_id(mgd_handle(), "article", "up=$d AND name=$q",
	  (*id)->value.lval, (*name)->value.str.val);
   php_midgard_get_object(return_value, MIDGARD_OBJECT_ARTICLE, objid);
}

#if ! HAVE_MIDGARD_MULTILANG
MGD_FUNCTION(int, create_article, (int up, int topic, string name,
									string title, string abstract,
									string content, int author,
									string url, string calstart,
									int caldays, int icon, int view,
									int print, string extra1, string extra2,
									string extra3, int type))
#else /* HAVE_MIDGARD_MULTILANG */
  MGD_FUNCTION(int, create_article, (int up, int topic, string name,
					 string title, string abstract,
					 string content, int author,
					 string url, string calstart,
					 int caldays, int icon, int view,
					 int print, string extra1, string extra2,
					 string extra3, int type, int contentauthor))
#endif /* HAVE_MIDGARD_MULTILANG */
{
	zval **up, **topic, **name, **title, **abstract, **content, **author;
	zval **url, **calstart, **caldays, **icon, **view, **print;
#if ! HAVE_MIDGARD_MULTILANG
	zval **extra1, **extra2, **extra3, **type;
#else /* HAVE_MIDGARD_MULTILANG */
	  zval **extra1, **extra2, **extra3, **type, **contentauthor;
#endif /* HAVE_MIDGARD_MULTILANG */
	zval *self;
	zval **zScore = NULL;
   long lScore = 0;
#if HAVE_MIDGARD_MULTILANG
   zval *retval_content0, *retval_content;
   int success = 1, created0 = 0, newid, longversion = 0;
   int lang = mgd_lang(mgd_handle());
   MAKE_STD_ZVAL(retval_content0);
   MAKE_STD_ZVAL(retval_content);
#endif /* HAVE_MIDGARD_MULTILANG */

	  /* [eeh] 0 is the default it'd get assigned by the database anyway */



	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (!MGD_PROPFIND(self, "up", up)
			|| !MGD_PROPFIND(self, "topic", topic)
			|| !MGD_PROPFIND(self, "name", name)
			|| !MGD_PROPFIND(self, "title", title)
			|| !MGD_PROPFIND(self, "abstract", abstract)
			|| !MGD_PROPFIND(self, "content", content)
			|| !MGD_PROPFIND(self, "author", author)
			|| !MGD_PROPFIND(self, "url", url)
			|| !MGD_PROPFIND(self, "calstart", calstart)
			|| !MGD_PROPFIND(self, "caldays", caldays)
			|| !MGD_PROPFIND(self, "icon", icon)
			|| !MGD_PROPFIND(self, "view", view)
			|| !MGD_PROPFIND(self, "print", print)
			|| !MGD_PROPFIND(self, "extra1", extra1)
			|| !MGD_PROPFIND(self, "extra2", extra2)
			|| !MGD_PROPFIND(self, "extra3", extra3)
#if ! HAVE_MIDGARD_MULTILANG
			|| !MGD_PROPFIND(self, "type", type)) {
#else /* HAVE_MIDGARD_MULTILANG */
	  || !MGD_PROPFIND(self, "type", type)
		   || !MGD_PROPFIND(self, "contentauthor", contentauthor)) {
#endif /* HAVE_MIDGARD_MULTILANG */
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
	  if (!MGD_PROPFIND(self, "score", zScore)) {
		 zScore = NULL;
	  }
	}
	else {
#if ! HAVE_MIDGARD_MULTILANG
		if (ZEND_NUM_ARGS() != 17
			|| zend_get_parameters_ex(17, &up, &topic, &name, &title,
						  &abstract, &content, &author,
						  &url, &calstart, &caldays, &icon,
						  &view, &print, &extra1, &extra2,
						  &extra3, &type) != SUCCESS)
			WRONG_PARAM_COUNT;
#else /* HAVE_MIDGARD_MULTILANG */
	switch (ZEND_NUM_ARGS()) {
	case 17:
	  if (zend_get_parameters_ex(17, &up, &topic, &name, &title,
					 &abstract, &content, &author,
					 &url, &calstart, &caldays, &icon,
					 &view, &print, &extra1, &extra2,
					 &extra3, &type) != SUCCESS)
		WRONG_PARAM_COUNT;
	  break;
	case 18:
	  if (zend_get_parameters_ex(18, &up, &topic, &name, &title,
					 &abstract, &content, &author,
					 &url, &calstart, &caldays, &icon,
					 &view, &print, &extra1, &extra2,
					 &extra3, &type, &contentauthor) != SUCCESS)
		WRONG_PARAM_COUNT;
	  longversion = 1;
	  break;
	default:
	  WRONG_PARAM_COUNT;
	}
#endif /* HAVE_MIDGARD_MULTILANG */

	}

	convert_to_long_ex(up);
	convert_to_long_ex(topic);
	convert_to_string_ex(name);
	convert_to_string_ex(title);
	convert_to_string_ex(abstract);
	convert_to_string_ex(content);
	convert_to_long_ex(author);
	convert_to_string_ex(url);
	convert_to_string_ex(calstart);
	convert_to_long_ex(caldays);
	convert_to_long_ex(icon);
	convert_to_long_ex(view);
	convert_to_long_ex(print);
	convert_to_string_ex(extra1);
	convert_to_string_ex(extra2);
	convert_to_string_ex(extra3);
	convert_to_long_ex(type);
#if HAVE_MIDGARD_MULTILANG
	  if (longversion) {
	convert_to_long_ex(contentauthor);
	if ((*contentauthor)->value.lval != 0 && !mgd_exists_id(mgd_handle(), "person", "id=$d", (*contentauthor)->value.lval))
	  RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	  }
#endif /* HAVE_MIDGARD_MULTILANG */
   if (zScore != NULL) {
	  convert_to_long_ex(zScore);
	  lScore = (*zScore)->value.lval;
   }

	  if (!mgd_exists_id(mgd_handle(), "person", "id=$d", (*author)->value.lval))
	RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	  if (!mgd_exists_id(mgd_handle(), "topic", "id=$d", (*topic)->value.lval))
	RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	  if ((*up)->value.lval != 0 && !mgd_exists_id(mgd_handle(), "article", "id=$d", (*up)->value.lval))
	RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	  if (!istopicowner((*topic)->value.lval)) {
	RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	  }

	  if ((*name)->value.str.len != 0 && mgd_exists_id(mgd_handle(), "article",
							   "topic=$d AND name=$q AND up=$d",
							   (*topic)->value.lval, (*name)->value.str.val, (*up)->value.lval)) {
	RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);
	  }
	  php_midgard_create(return_value, self, "article",
#if ! HAVE_MIDGARD_MULTILANG
			 "up,topic,name,title,abstract,content,author,"
			 "url,calstart,caldays,icon,view,print,"
#else /* HAVE_MIDGARD_MULTILANG */
			 "up,topic,name,author,"
			 "calstart,caldays,icon,view,print,"
#endif /* HAVE_MIDGARD_MULTILANG */
			 "extra1,extra2,extra3,type,"
			 "created,creator,revised,revisor,"
			 "revision,approved,approver,score",
#if ! HAVE_MIDGARD_MULTILANG
			 "$d,$d,$q,$q,$q,$q,$d,$q,$t,$d,$d,$d,$d,$q,$q,$q,$d,"
#else /* HAVE_MIDGARD_MULTILANG */
			 "$d,$d,$q,$d,$t,$d,$d,$d,$d,$q,$q,$q,$d,"
#endif /* HAVE_MIDGARD_MULTILANG */
			 "Now(),$d,Now(),$d,1,0,0,$d",
			 (*up)->value.lval, (*topic)->value.lval,
#if ! HAVE_MIDGARD_MULTILANG
			 (*name)->value.str.val, (*title)->value.str.val,
			 (*abstract)->value.str.val,
			 (*content)->value.str.val, (*author)->value.lval,
			 (*url)->value.str.val, (*calstart)->value.str.val,
#else /* HAVE_MIDGARD_MULTILANG */
			 (*name)->value.str.val, (*author)->value.lval,
			 (*calstart)->value.str.val,
#endif /* HAVE_MIDGARD_MULTILANG */
			 (*caldays)->value.lval, (*icon)->value.lval,
			 (*view)->value.lval, (*print)->value.lval,
			 (*extra1)->value.str.val, (*extra2)->value.str.val,
			 (*extra3)->value.str.val, (*type)->value.lval,
			 mgd_user(mgd_handle()), mgd_user(mgd_handle()), lScore);
	  PHP_CREATE_REPLIGARD("article", return_value->value.lval);
#if HAVE_MIDGARD_MULTILANG
	  newid = return_value->value.lval;
	if (newid) {
	  if (lang > 0) {
		php_midgard_create_article_content_internal(retval_content0, longversion, return_value->value.lval, "", "", "", "", longversion?(*contentauthor)->value.lval:0, 0);
		if (!(retval_content0->value.lval)) success = 0;
		else created0 = 1;
	  }
	  if (success) {
		php_midgard_create_article_content_internal(retval_content, longversion, return_value->value.lval, (*title)->value.str.val, (*abstract)->value.str.val, (*content)->value.str.val, (*url)->value.str.val, longversion?(*contentauthor)->value.lval:0, lang);
		if (!(retval_content->value.lval)) success = 0;
	  }
	  if (!success) {
		if (created0) {
		  php_midgard_delete(retval_content, "article_i", retval_content0->value.lval);
		  PHP_DELETE_REPLIGARD ("article_i", retval_content0->value.lval);
		}
		php_midgard_delete(retval_content, "article", newid);
		PHP_DELETE_REPLIGARD ("article", newid);
		RETVAL_FALSE_BECAUSE(MGD_ERR_QUOTA);
	  }
	}
#endif /* HAVE_MIDGARD_MULTILANG */
}


#if HAVE_MIDGARD_MULTILANG

MGD_FUNCTION(ret_type, create_article_content, (type param)) {
zval **id, **up, **topic, **name, **title, **abstract, **content, **author;
zval **url, **calstart, **caldays, **icon, **view, **print;
zval **extra1, **extra2, **extra3, **type, **contentauthor;
zval *self;
int longversion;
int lang = mgd_lang(mgd_handle());
longversion = 0;


RETVAL_FALSE;
CHECK_MGD;

if ((self = getThis()) != NULL) {
if (!MGD_PROPFIND(self, "id", id)
	 || !MGD_PROPFIND(self, "up", up)
	 || !MGD_PROPFIND(self, "topic", topic)
	 || !MGD_PROPFIND(self, "name", name)
	 || !MGD_PROPFIND(self, "title", title)
	 || !MGD_PROPFIND(self, "abstract", abstract)
	 || !MGD_PROPFIND(self, "content", content)
	 || !MGD_PROPFIND(self, "author", author)
	 || !MGD_PROPFIND(self, "url", url)
	 || !MGD_PROPFIND(self, "calstart", calstart)
	 || !MGD_PROPFIND(self, "caldays", caldays)
	 || !MGD_PROPFIND(self, "icon", icon)
	 || !MGD_PROPFIND(self, "view", view)
	 || !MGD_PROPFIND(self, "print", print)
	 || !MGD_PROPFIND(self, "extra1", extra1)
	 || !MGD_PROPFIND(self, "extra2", extra2)
	 || !MGD_PROPFIND(self, "extra3", extra3)
	 || !MGD_PROPFIND(self, "type", type)
	 || !MGD_PROPFIND(self, "contentauthor", contentauthor))
	 RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
	 longversion = 1;
	 } else {
switch (ZEND_NUM_ARGS()) {
case 5:
if (zend_get_parameters_ex(5, &id, &title, &abstract, &content,
  &url)	 != SUCCESS)
	 WRONG_PARAM_COUNT;
	 break;
	 case 6:
	 if (zend_get_parameters_ex(6, &id, &title, &abstract, &content,
	   &url, &contentauthor) != SUCCESS)
	 WRONG_PARAM_COUNT;
	 longversion = 1;
	 break;
	 default:
	 WRONG_PARAM_COUNT;
}
}

convert_to_long_ex(id);
convert_to_string_ex(title);
convert_to_string_ex(abstract);
convert_to_string_ex(content);
convert_to_string_ex(url);
if (longversion) {
convert_to_long_ex(contentauthor);
if ((*contentauthor)->value.lval != 0 && !mgd_exists_id(mgd_handle(), "person", "id=$d", (*contentauthor)->value.lval))
	 RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	 }

if (!mgd_exists_id(mgd_handle(), "article", "id=$d", (*id)->value.lval))
	 RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	 if (mgd_exists_id(mgd_handle(), "article_i", "sid=$d AND lang=$d",
	   (*id)->value.lval, lang))
	 RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

	 php_midgard_create_article_content_internal(return_value, longversion, (*id)->value.lval, (*title)->value.str.val, (*abstract)->value.str.val, (*content)->value.str.val, (*url)->value.str.val, longversion?(*contentauthor)->value.lval:0, lang);

	 RETURN_TRUE;

}
#endif /* HAVE_MIDGARD_MULTILANG */


MGD_FUNCTION(bool, update_article_score, (int id, int score))
{
	zval **id, **score, *self;
	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1,
								   &score) !=
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

	if (!isarticleowner((*id)->value.lval)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	}

	php_midgard_update(return_value, "article", "score=$d",
			   (*id)->value.lval, (*score)->value.lval);
	PHP_UPDATE_REPLIGARD("article", (*id)->value.lval);
}

MGD_FUNCTION(bool, update_article_created, (int id, int timestamp))
{
	zval **id, **timestamp;
	RETVAL_FALSE;
	CHECK_MGD;
	if (ZEND_NUM_ARGS() != 2
		|| zend_get_parameters_ex(2, &id, &timestamp) != SUCCESS)
		WRONG_PARAM_COUNT;
	convert_to_long_ex(id);
	convert_to_long_ex(timestamp);

	if (!isarticleowner((*id)->value.lval)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }

	php_midgard_update(return_value, "article", "created=from_unixtime($d)",
			   (*id)->value.lval, (*timestamp)->value.lval);
	PHP_UPDATE_REPLIGARD("article", (*id)->value.lval);
}

MGD_FUNCTION(bool, update_article_replyto, (int id, int up))
{
	zval **id, **up;
	int parent_topic;
	RETVAL_FALSE;
	CHECK_MGD;

	if (ZEND_NUM_ARGS() != 2
		|| zend_get_parameters_ex(2, &id,
					  &up) != SUCCESS) WRONG_PARAM_COUNT;

	convert_to_long_ex(id);
	convert_to_long_ex(up);

	if (up && ((*up)->value.lval != 0)) {
		if (!mgd_exists_bool(mgd_handle(), "article src, article tgt",
										"src.id=$d AND tgt.id=$d"
										" AND (src.sitegroup=tgt.sitegroup"
											" OR src.sitegroup=0"
											" OR tgt.sitegroup=0)",
										(*id)->value.lval, (*up)->value.lval)) {
			RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);
		}
	}

	if (!isarticleowner((*id)->value.lval)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }

	parent_topic =
	   mgd_idfield(mgd_handle(), "topic", "article", (*up)->value.lval);

	if (up && ((*up)->value.lval != 0)) {
      php_midgard_update(return_value, "article", "up=$d,topic=$d",
        (*id)->value.lval, (*up)->value.lval, parent_topic);
      } else {
        php_midgard_update(return_value, "article", "up=$d",
            (*id)->value.lval, (*up)->value.lval);
      }

	PHP_UPDATE_REPLIGARD("article", (*id)->value.lval);
}

MGD_FUNCTION(bool, update_article_type, (int id, int type))
{
	zval **id, **type, *self;
	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1,
								   &type) !=
			SUCCESS) WRONG_PARAM_COUNT;
		if (!MGD_PROPFIND(self, "id", id)) {
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
	  }
	}
	else {
		if (ZEND_NUM_ARGS() != 2
			|| zend_get_parameters_ex(2, &id, &type) != SUCCESS)
			WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(id);
	convert_to_long_ex(type);

	if (!isarticleowner((*id)->value.lval)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }

	php_midgard_update(return_value, "article", "type=$d",
			   (*id)->value.lval, (*type)->value.lval);
	PHP_UPDATE_REPLIGARD("article", (*id)->value.lval);
}

MGD_FUNCTION(bool, toggle_article_lock, (int id))
{
	zval **id;
	RETVAL_FALSE;
	CHECK_MGD;
	if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &id) != SUCCESS)
		WRONG_PARAM_COUNT;
	convert_to_long_ex(id);

	if (!isarticleowner((*id)->value.lval)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }

	php_midgard_update(return_value, "article",
			   "locked=Now(),locker=If(locker,0,$d)",
			   (*id)->value.lval, mgd_user(mgd_handle()));
	PHP_UPDATE_REPLIGARD("article", (*id)->value.lval);
}

MGD_FUNCTION(bool, approve_article, (int id, [bool approve=1]))
{
	zval **id;
	zval **approve;
	int flag;
	RETVAL_FALSE;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
		case 1:
			if (zend_get_parameters_ex(1, &id) != SUCCESS)
				WRONG_PARAM_COUNT;
			convert_to_long_ex(id);
			flag = 1;
			break;

		case 2:
			if (zend_get_parameters_ex(2, &id, &approve) != SUCCESS)
				WRONG_PARAM_COUNT;
			convert_to_long_ex(id);
			convert_to_boolean_ex(approve);
			flag = (*approve)->value.lval;
			break;

		default:
			WRONG_PARAM_COUNT;
	}

	if (!isarticleowner((*id)->value.lval)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }

	if (flag)
		php_midgard_update(return_value, "article",
				   "approved=Now(),approver=$d",
				   (*id)->value.lval, mgd_user(mgd_handle()));
	else
		php_midgard_update(return_value, "article",
				   "approved='0000-00-00 00:00:00',approver=0",
				   (*id)->value.lval);
	PHP_UPDATE_REPLIGARD("article", (*id)->value.lval);
}

#if ! HAVE_MIDGARD_MULTILANG
MGD_FUNCTION(bool, update_article, (int id, int topic, string name,
									string title, string abstract,
									string content, int author,
									string url, string calstart,
									int caldays, int icon, int view,
									int print, string extra1, string extra2,
									string extra3))
#else /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(bool, update_article, (int id, int topic, string name,
									string title, string abstract,
									string content, int author,
									string url, string calstart,
									int caldays, int icon, int view,
									int print, string extra1, string extra2,
					string extra3, int contentauthor))
#endif /* HAVE_MIDGARD_MULTILANG */
{
	zval **id, **topic, **name, **title, **abstract, **content, **author;
	zval **url, **calstart, **caldays, **icon, **view, **print;
	zval **extra1, **extra2, **extra3;
	zval **type, **score, **up, *self;
#if HAVE_MIDGARD_MULTILANG
	zval **contentauthor, **lang;
#endif /* HAVE_MIDGARD_MULTILANG */
   long upval;
#if HAVE_MIDGARD_MULTILANG
	midgard_res *res;
	int i_id = 0;
	int longversion, lang_means_something;
	int lang_i = mgd_lang(mgd_handle());
	longversion = 0; lang_means_something = 0;
#endif /* HAVE_MIDGARD_MULTILANG */

	RETVAL_FALSE;
	CHECK_MGD;

	if ((self = getThis()) != NULL) {
		if (!MGD_PROPFIND(self, "id", id)
		|| !MGD_PROPFIND(self, "topic", topic)
		|| !MGD_PROPFIND(self, "name", name)
		|| !MGD_PROPFIND(self, "title", title)
		|| !MGD_PROPFIND(self, "abstract", abstract)
		|| !MGD_PROPFIND(self, "content", content)
		|| !MGD_PROPFIND(self, "author", author)
		|| !MGD_PROPFIND(self, "url", url)
		|| !MGD_PROPFIND(self, "calstart", calstart)
		|| !MGD_PROPFIND(self, "caldays", caldays)
		|| !MGD_PROPFIND(self, "icon", icon)
		|| !MGD_PROPFIND(self, "view", view)
		|| !MGD_PROPFIND(self, "print", print)
		|| !MGD_PROPFIND(self, "extra1", extra1)
		|| !MGD_PROPFIND(self, "extra2", extra2)
		|| !MGD_PROPFIND(self, "extra3", extra3)
		|| !MGD_PROPFIND(self, "type", type)
		|| !MGD_PROPFIND(self, "up", up)
#if ! HAVE_MIDGARD_MULTILANG
		|| !MGD_PROPFIND(self, "score", score)) {
#else /* HAVE_MIDGARD_MULTILANG */
		|| !MGD_PROPFIND(self, "score", score)
		|| !MGD_PROPFIND(self, "lang", lang)
		|| !MGD_PROPFIND(self, "contentauthor", contentauthor)) {
#endif /* HAVE_MIDGARD_MULTILANG */
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
		}
#if ! HAVE_MIDGARD_MULTILANG
	} else {
#else /* HAVE_MIDGARD_MULTILANG */
		longversion = 1;
	} else {
#endif /* HAVE_MIDGARD_MULTILANG */
	  	score = type = up = NULL;
#if ! HAVE_MIDGARD_MULTILANG
	  	if (ZEND_NUM_ARGS() != 16
		|| zend_get_parameters_ex(16, &id, &topic, &name, &title,
		&abstract, &content, &author, &url, &calstart, &caldays, &icon,
		&view, &print, &extra1, &extra2, &extra3) != SUCCESS) {
			WRONG_PARAM_COUNT;
		}
#else /* HAVE_MIDGARD_MULTILANG */
		switch (ZEND_NUM_ARGS()) {
		case 16:
			if (zend_get_parameters_ex(16, &id, &topic, &name, &title,
			&abstract, &content, &author,
			&url, &calstart, &caldays, &icon,
			&view, &print, &extra1, &extra2,
			&extra3) != SUCCESS) {
				WRONG_PARAM_COUNT;
			}
			break;
		case 17:
			if (zend_get_parameters_ex(17, &id, &topic, &name, &title,
			&abstract, &content, &author,
			&url, &calstart, &caldays, &icon,
			&view, &print, &extra1, &extra2,
			&extra3, &contentauthor) != SUCCESS) {
		 		WRONG_PARAM_COUNT;
			}
		}
#endif /* HAVE_MIDGARD_MULTILANG */
	}
	convert_to_long_ex(id);
	convert_to_long_ex(topic);
	convert_to_string_ex(name);
	convert_to_string_ex(title);
	convert_to_string_ex(abstract);
	convert_to_string_ex(content);
	convert_to_long_ex(author);
	convert_to_string_ex(url);
	convert_to_string_ex(calstart);
	convert_to_long_ex(caldays);
	convert_to_long_ex(icon);
	convert_to_long_ex(view);
	convert_to_long_ex(print);
	convert_to_string_ex(extra1);
	convert_to_string_ex(extra2);
	convert_to_string_ex(extra3);

#if HAVE_MIDGARD_MULTILANG
	if (lang_means_something) {
		convert_to_long_ex(lang);
		lang_i = (*lang)->value.lval;
	}

	if (longversion) {
		convert_to_long_ex(contentauthor);
		if ((*contentauthor)->value.lval != 0
		&& !mgd_exists_id(mgd_handle(), "person", "id=$d", (*contentauthor)->value.lval)) {
			RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
		}
	}

#endif /* HAVE_MIDGARD_MULTILANG */
	/* EEH: conversion to string is intentional, see update code */
	if (type)
		convert_to_string_ex(type);
	if (score)
		convert_to_string_ex(score);
	if (up) {
		convert_to_long_ex(up);
		upval = (*up)->value.lval;
	  
		if (upval != 0 && !mgd_exists_id(mgd_handle(), "article", "id=$d", upval))
			RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
		convert_to_string_ex(up);
	}


	if (!isarticleowner((*id)->value.lval)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	}

	if (!mgd_exists_id(mgd_handle(), "person", "id=$d", (*author)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	if (!mgd_exists_id(mgd_handle(), "topic", "id=$d", (*topic)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);

	/* skip duplicate check for empty names because the name isn't mandatory */
	if ((*name)->value.str.len != 0
	&& mgd_exists_id(mgd_handle(), "article", "topic=$d AND name=$q AND up=$s AND id<>$d",
	(*topic)->value.lval, (*name)->value.str.val,
	up ? (*up)->value.str.val : "up",
	(*id)->value.lval)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);
	}

	if (!istopicowner((*topic)->value.lval)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	}

#if HAVE_MIDGARD_MULTILANG
	res = mgd_ungrouped_select(mgd_handle(), "id", "article_i", "sid=$d AND lang=$d", NULL, (*id)->value.lval, lang_i);

	if (res && mgd_fetch(res)) {
		i_id = atol(mgd_colvalue(res, 0));
	} else {
	    php_midgard_create_article_content_internal(return_value, longversion, (*id)->value.lval, (*title)->value.str.val, (*abstract)->value.str.val, (*content)->value.str.val, (*url)->value.str.val, longversion?(*contentauthor)->value.lval:0, lang_i);

        res = mgd_ungrouped_select(mgd_handle(), "id", "article_i", "sid=$d AND lang=$d", NULL, (*id)->value.lval, lang_i);

        if (res && mgd_fetch(res)) {
            i_id = atol(mgd_colvalue(res, 0));
        }
                                 
	}

	mgd_release(res);

	php_midgard_update(return_value, "article",
			   "topic=$d,name=$q,"
			   "author=$d,calstart=$t,caldays=$d,icon=$d,"
			   "view=$d,print=$d,extra1=$q,extra2=$q,extra3=$q,"
			   "revised=Now(),revisor=$d,revision=revision+1,"
			   "type=$s,score=$s,up=$s",
			   (*id)->value.lval, (*topic)->value.lval, (*name)->value.str.val,
			   (*author)->value.lval,
			   (*calstart)->value.str.val,
			   (*caldays)->value.lval, (*icon)->value.lval,
			   (*view)->value.lval, (*print)->value.lval,
			   (*extra1)->value.str.val, (*extra2)->value.str.val,
			   (*extra3)->value.str.val, mgd_user(mgd_handle()),
			/* EEH I don't know why there would be an defined-but-empty
			   string here, but best be sure */
			   (type && *((*type)->value.str.val)) ? (*type)->value.str.val : "type",
			   (score && *((*score)->value.str.val)) ? (*score)->value.str.val : "score",
			   (up && *((*up)->value.str.val)) ? (*up)->value.str.val : "up" );

#else /* HAVE_MIDGARD_MULTILANG */
	php_midgard_update(return_value, "article",
			   "topic=$d,name=$q,title=$q,abstract=$q,content=$q,"
			   "author=$d,url=$q,calstart=$t,caldays=$d,icon=$d,"
			   "view=$d,print=$d,extra1=$q,extra2=$q,extra3=$q,"
			   "revised=Now(),revisor=$d,revision=revision+1,"
			   "type=$s,score=$s,up=$s",
			   (*id)->value.lval,
			   (*topic)->value.lval, (*name)->value.str.val,
			   (*title)->value.str.val, (*abstract)->value.str.val,
			   (*content)->value.str.val, (*author)->value.lval,
			   (*url)->value.str.val, (*calstart)->value.str.val,
			   (*caldays)->value.lval, (*icon)->value.lval,
			   (*view)->value.lval, (*print)->value.lval,
			   (*extra1)->value.str.val, (*extra2)->value.str.val,
			   (*extra3)->value.str.val, mgd_user(mgd_handle()),
			/* EEH I don't know why there would be an defined-but-empty
			   string here, but best be sure */
			   (type && *((*type)->value.str.val)) ? (*type)->value.str.val : "type",
			   (score && *((*score)->value.str.val)) ? (*score)->value.str.val : "score",
			   (up && *((*up)->value.str.val)) ? (*up)->value.str.val : "up" );

#endif /* HAVE_MIDGARD_MULTILANG */
	PHP_UPDATE_REPLIGARD("article", (*id)->value.lval);
#if HAVE_MIDGARD_MULTILANG

	php_midgard_update_article_content_internal(return_value, longversion, i_id, (*title)->value.str.val, (*abstract)->value.str.val, (*content)->value.str.val, (*url)->value.str.val, longversion?(*contentauthor)->value.lval:0);
}


MGD_FUNCTION(ret_type, update_article_content, (type param))
{
	zval **id, **title, **abstract, **content, **url, **lang, **contentauthor, *self;

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
			|| !MGD_PROPFIND(self, "abstract", abstract)
			|| !MGD_PROPFIND(self, "content", content)
			|| !MGD_PROPFIND(self, "url", url)
			|| !MGD_PROPFIND(self, "lang", lang)
			|| !MGD_PROPFIND(self, "contentauthor", contentauthor)
			) {
		  RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);

		}
		lang_means_something = 1;
		longversion = 1;

	} else {
	  switch (ZEND_NUM_ARGS()) {
	  case 5:
		if (zend_get_parameters_ex(5, &id, &title, &abstract, &content,
					   &url)  != SUCCESS)
		  WRONG_PARAM_COUNT;
		break;
	  case 6:
		if (zend_get_parameters_ex(6, &id, &title, &abstract, &content,
					   &url, &contentauthor) != SUCCESS)
		  WRONG_PARAM_COUNT;
		longversion = 1;
		break;
	  default:
		WRONG_PARAM_COUNT;
}
	}

	convert_to_long_ex(id);
	convert_to_string_ex(title);
	convert_to_string_ex(abstract);
	convert_to_string_ex(content);
	convert_to_string_ex(url);

	if (lang_means_something) {
	  convert_to_long_ex(lang);
	  lang_i = (*lang)->value.lval;
	}

	if (!isarticleowner((*id)->value.lval)) {
	  RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	}

	if (longversion) {
	  convert_to_long_ex(contentauthor);
	  if ((*contentauthor)->value.lval != 0 && !mgd_exists_id(mgd_handle(), "person", "id=$d", (*contentauthor)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}


	res = mgd_ungrouped_select(mgd_handle(), "id", "article_i", "sid=$d AND lang=$d", NULL, (*id)->value.lval, lang_i);

	if (res && mgd_fetch(res)) {
	  i_id = atol(mgd_colvalue(res, 0));
	} else {
	  RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}

	mgd_release(res);
	php_midgard_update_article_content_internal(return_value, longversion, i_id, (*title)->value.str.val, (*abstract)->value.str.val, (*content)->value.str.val, (*url)->value.str.val, longversion?(*contentauthor)->value.lval:0);
	RETURN_TRUE;
#endif /* HAVE_MIDGARD_MULTILANG */
}

MGD_FUNCTION(bool, delete_article, (int id))
{
#if HAVE_MIDGARD_MULTILANG
	zval *content_return;
	midgard_res *res;
	int i_id;

#endif /* HAVE_MIDGARD_MULTILANG */
	IDINIT;
	CHECK_MGD;

	if (mgd_has_dependants(mgd_handle(), id, "article")) {
		RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
   }

	if (mgd_exists_id(mgd_handle(), "article", "up=$d", id)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
   }

	if (!isarticleowner(id)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }

#if HAVE_MIDGARD_MULTILANG
	/* need to delete all article contents */
	res = mgd_ungrouped_select(mgd_handle(), "id", "article_i", "sid=$d", NULL, id);
	if (res) {
	  while (mgd_fetch(res)) {
	i_id = atol(mgd_colvalue(res,0));
	MAKE_STD_ZVAL(content_return);
		php_midgard_delete(content_return, "article_i", i_id);
	PHP_DELETE_REPLIGARD("article_i", i_id);
	  }
	}
	mgd_release(res);
#endif /* HAVE_MIDGARD_MULTILANG */
	php_midgard_delete(return_value, "article", id);
	PHP_DELETE_REPLIGARD("article", id);
}

MGD_FUNCTION(int, copy_article, (int id, [int topic]))
{
	zval **id, **newtopic;
	int r_id;
	RETVAL_FALSE;
	CHECK_MGD;
	switch (ZEND_NUM_ARGS()) {
		case 2:
			if (zend_get_parameters_ex(2, &id, &newtopic) !=
				SUCCESS) WRONG_PARAM_COUNT;
			break;
		case 1:
			if (zend_get_parameters_ex(1, &id) != SUCCESS)
				WRONG_PARAM_COUNT;
			newtopic = NULL;
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(id);
	if (newtopic)
		convert_to_long_ex(newtopic);

	/* newtopic must be in same SG or be 0 */
	if (newtopic && !mgd_exists_bool(mgd_handle(), "topic,article",
					"topic.id=$d AND article.id=$d"
					" AND (topic.sitegroup=article.sitegroup"
					" OR topic.sitegroup=0"
					" OR article.sitegroup=0)",
					(*newtopic)->value.lval, (*id)->value.lval)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_SITEGROUP_VIOLATION);
	}
	
	r_id = mgd_copy_article(mgd_handle(), (*id)->value.lval, newtopic ? (*newtopic)->value.lval : 0);
#if HAVE_MIDGARD_QUOTA
	if (mgd_get_quota_error(mgd_handle())) {
	  mgd_set_quota_error(mgd_handle(), 0);
	  RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
	}
#endif /* HAVE_MIDGARD_QUOTA */
	  RETVAL_LONG(r_id);
}

#if HAVE_MIDGARD_MULTILANG
MGD_FUNCTION(ret_type, delete_article_content, (type param))
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

  if (!isarticleowner(id))
	RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

  res = mgd_ungrouped_select(mgd_handle(), "id", "article_i", "sid=$d AND lang=$d", NULL, id, lang_i);

  if (res && mgd_fetch(res)) {
	i_id = atol(mgd_colvalue(res, 0));
  } else {
	RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
  }

  mgd_release(res);
  php_midgard_delete(return_value, "article_i", i_id);

  PHP_DELETE_REPLIGARD("article_i", i_id);

  TOUCH_CACHE;
}


#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(bool, delete_article_tree, (int id))
{
	IDINIT;
	CHECK_MGD;

	if (!isarticleowner(id)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }

	if (mgd_delete_article(mgd_handle(), id)) {
		RETURN_TRUE;
   }

   RETURN_FALSE;
}

MGD_MOVE_FUNCTION(article, topic, article, topic);
MGD_MOVE_FUNCTION(article, article, reply, up);

MGD_WALK_FUNCTION(article);

static zend_function_entry MidgardArticleMethods[] = {
   PHP_FALIAS(MidgardArticle, mgd_get_article,			 NULL)
   PHP_FALIAS(create,		  mgd_create_article,		 NULL)
   PHP_FALIAS(update,		  mgd_update_article,		 NULL)
   PHP_FALIAS(delete,		  mgd_delete_article,		 NULL)
#if HAVE_MIDGARD_MULTILANG
   PHP_FALIAS(create_content,		  mgd_create_article_content,		 NULL)
   PHP_FALIAS(update_content,		  mgd_update_article_content,		 NULL)
   PHP_FALIAS(delete_content,		  mgd_delete_article_content,		 NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
   PHP_FALIAS(settype,		  mgd_update_article_type,	 NULL)
   PHP_FALIAS(setscore,		  mgd_update_article_score,	 NULL)
   PHP_FALIAS(fetch,		  mgd_oop_list_fetch,		 NULL)
   MIDGARD_OOP_PARAMETER_METHODS
   MIDGARD_OOP_ATTACHMENT_METHODS
   MIDGARD_OOP_SITEGROUP_METHODS
   {  NULL,				NULL,					   NULL}
};
MidgardClass MidgardArticle = {
   "MidgardArticle",
   "article",
   MidgardArticleMethods,
   {},
   NULL
};

#endif /* PHP_MIDGARD_LEGACY_API */

