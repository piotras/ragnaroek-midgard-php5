/* $Id: calendar.c 10324 2006-11-27 12:47:04Z piotras $
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

#define DATE_MAX 0x7FFFFFFF

#include "mgd_internal.h"
#include "mgd_oop.h"

#ifdef PHP_MIDGARD_LEGACY_API

MGD_FUNCTION(ret_type, list_topic_calendar_all, (type param))
{
    int *topics;
    int typev = 0, startv = 0, stopv = 0;
    const char *sortv = NULL;
    zval **id, **startn, **stopn, **sortn, **typen;

    RETVAL_FALSE;
	CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
	case 5:
		if (zend_get_parameters_ex(5,
						  &id, &startn, &stopn, &sortn, &typen) == SUCCESS) {
			convert_to_long_ex(id);
			convert_to_long_ex(startn);
			convert_to_long_ex(stopn);
			convert_to_string_ex(sortn);
			convert_to_long_ex(typen);
			typev = (*typen)->value.lval;
			sortv = article_sort((*sortn)->value.str.val);
			startv = (*startn)->value.lval;
			stopv = ((*stopn)->value.lval > 0) ? (*stopn)->value.lval : DATE_MAX;
			break;
		}
	case 4:
		if (zend_get_parameters_ex(4, &id, &startn, &stopn, &sortn) == SUCCESS) {
			convert_to_long_ex(id);
			convert_to_long_ex(startn);
			convert_to_long_ex(stopn);
			convert_to_string_ex(sortn);
			typev = -1;
			sortv = article_sort((*sortn)->value.str.val);
			startv = (*startn)->value.lval;
			stopv = ((*stopn)->value.lval > 0) ? (*stopn)->value.lval : DATE_MAX;
			break;
		}
	case 3:
		if (zend_get_parameters_ex(3, &id, &startn, &stopn) == SUCCESS) {
			convert_to_long_ex(id);
			convert_to_long_ex(startn);
			convert_to_long_ex(stopn);
			typev = -1;
			sortv = "article.calstart";
			startv = (*startn)->value.lval;
			stopv = ((*stopn)->value.lval > 0) ? (*stopn)->value.lval : DATE_MAX;
			break;
		}
    case 2:
		if (zend_get_parameters_ex(2, &id, &typen) == SUCCESS) {
			convert_to_long_ex(id);
			convert_to_long_ex(typen);
			typev = (*typen)->value.lval;
			sortv = "article.calstart";
			startv = time(NULL);
			stopv = DATE_MAX;
			break;
		}
    case 1:
		if (zend_get_parameters_ex(1, &id) == SUCCESS) {
			convert_to_long_ex(id);
			typev = -1;
			sortv = "article.calstart";
			startv = time(NULL);
			stopv = DATE_MAX;
			break;
		}
    default:
		WRONG_PARAM_COUNT;
    }
	
    topics = mgd_tree(mgd_handle(), "topic", "up", (*id)->value.lval, 0, NULL);
	
	if(topics) {
	    if (typev == -1)
		    php_midgard_select(&MidgardArticle, return_value,
						   ARTICLE_SELECT "," ARTICLE_CALENDAR, ARTICLE_FROM,
						   "article.topic IN $D AND article.author = person.id"
						   " AND article.id=article_i.sid AND article_i.lang IN ($d,$d)"
						   " AND article.up = 0"
						   " AND Unix_Timestamp(calstart) > 0"
						   " AND Unix_Timestamp(calstart) >= $d"
						   " AND 24*60*60-1+Unix_timestamp("
						   "     Date_Add(calstart, INTERVAL caldays DAY)) <= $d",
						   sortv, topics, 
						   mgd_get_default_lang(mgd_handle()),
						   mgd_lang(mgd_handle()), startv, stopv);
	    else
		    php_midgard_select(&MidgardArticle, return_value,
						   ARTICLE_SELECT "," ARTICLE_CALENDAR, ARTICLE_FROM,
						   "article.type=$d AND article.topic IN $D"
						   " AND article.id=article_i.sid AND article_i.lang IN ($d,$d)"
						   " AND article.author=person.id"
						   " AND article.up = 0"
						   " AND Unix_Timestamp(calstart) > 0"
						   " AND Unix_Timestamp(calstart) >= $d"
						   " AND 24*60*60-1+Unix_timestamp("
						   "     Date_Add(calstart, INTERVAL caldays DAY)) <= $d",
						   sortv, typev, topics, 
						   mgd_get_default_lang(mgd_handle()),
						   mgd_lang(mgd_handle()),startv, stopv);
		free(topics);
	}
}

MGD_FUNCTION(ret_type, list_topic_calendar_all_fast, (type param))
{
    int *topics;
    int typev = 0, startv = 0, stopv = 0;
    const char *sortv = NULL;
    zval **id, **startn, **stopn, **sortn, **typen;

    RETVAL_FALSE;
	CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
	case 5:
		if (zend_get_parameters_ex(5,
						  &id, &startn, &stopn, &sortn, &typen) == SUCCESS) {
			convert_to_long_ex(id);
			convert_to_long_ex(startn);
			convert_to_long_ex(stopn);
			convert_to_string_ex(sortn);
			convert_to_long_ex(typen);
			typev = (*typen)->value.lval;
			sortv = article_sort((*sortn)->value.str.val);
			startv = (*startn)->value.lval;
			stopv = ((*stopn)->value.lval > 0) ? (*stopn)->value.lval : DATE_MAX;
			break;
		}
	case 4:
		if (zend_get_parameters_ex(4, &id, &startn, &stopn, &sortn) == SUCCESS) {
			convert_to_long_ex(id);
			convert_to_long_ex(startn);
			convert_to_long_ex(stopn);
			convert_to_string_ex(sortn);
			typev = -1;
			sortv = article_sort((*sortn)->value.str.val);
			startv = (*startn)->value.lval;
			stopv = ((*stopn)->value.lval > 0) ? (*stopn)->value.lval : DATE_MAX;
			break;
		}
	case 3:
		if (zend_get_parameters_ex(3, &id, &startn, &stopn) == SUCCESS) {
			convert_to_long_ex(id);
			convert_to_long_ex(startn);
			convert_to_long_ex(stopn);
			typev = -1;
			sortv = "article.calstart";
			startv = (*startn)->value.lval;
			stopv = ((*stopn)->value.lval > 0) ? (*stopn)->value.lval : DATE_MAX;
			break;
		}
    case 2:
		if (zend_get_parameters_ex(2, &id, &typen) == SUCCESS) {
			convert_to_long_ex(id);
			convert_to_long_ex(typen);
			typev = (*typen)->value.lval;
			sortv = "article.calstart";
			startv = time(NULL);
			stopv = DATE_MAX;
			break;
		}
    case 1:
		if (zend_get_parameters_ex(1, &id) == SUCCESS) {
			convert_to_long_ex(id);
			typev = -1;
			sortv = "article.calstart";
			startv = time(NULL);
			stopv = DATE_MAX;
			break;
		}
    default:
		WRONG_PARAM_COUNT;
    }
	
    topics = mgd_tree(mgd_handle(), "topic", "up", (*id)->value.lval, 0, NULL);
	
	if(topics) {
	    if (typev == -1)
		    php_midgard_select(&MidgardArticle, return_value,
						   ARTICLE_SELECT_FAST "," ARTICLE_CALENDAR,
						   ARTICLE_FROM_FAST,
						   "article.topic IN $D"
						   " AND article.up=0 AND Unix_Timestamp(calstart)>=$d"
						   " AND 24*60*60-1+Unix_timestamp("
						   "     Date_Add(calstart, INTERVAL caldays DAY))<=$d",
						   sortv, topics, startv, stopv);
		else
			    php_midgard_select(&MidgardArticle, return_value,
						   ARTICLE_SELECT_FAST "," ARTICLE_CALENDAR,
						   ARTICLE_FROM_FAST,
						   "article.type=$d AND article.topic IN $D"
						   " AND article.up=0 AND Unix_Timestamp(calstart)>=$d"
						   " AND 24*60*60-1+Unix_timestamp("
						   "     Date_Add(calstart, INTERVAL caldays DAY))<=$d",
						   sortv, typev, topics, startv, stopv);
		free(topics);
	}
}

#endif
