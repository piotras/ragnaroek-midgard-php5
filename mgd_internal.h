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

#define php_rqst    ((request_rec *) SG(server_context))

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
   int id; zval *self, *id_zval; \
   if (!mgd_handle()) \
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_CONNECTED); \
   if ((self = getThis()) != NULL) { \
      if (! MGD_PROPFIND(self, "id", &id_zval)) { \
         RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT); \
      } \
   } else { \
      if (ZEND_NUM_ARGS() != 1 \
            || zend_parse_parameters(1 TSRMLS_CC, "l", &id_zval) != SUCCESS) \
      WRONG_PARAM_COUNT; \
   } \
   id = Z_LVAL_P(id_zval);

#define PHP_CREATE_REPLIGARD(table,id)
#define PHP_CREATE_REPLIGARD_VOID(table,id)

#define PHP_DELETE_REPLIGARD(table,id) \
   { \
      if(id != 0) DELETE_REPLIGARD(mgd_handle(), table, id) \
      else RETURN_FALSE_BECAUSE(MGD_ERR_ERROR); \
   }

#define PHP_UPDATE_REPLIGARD(table,id) \
   UPDATE_REPLIGARD(mgd_handle(), table, id)

#define MGD_MOVE_FUNCTION(table,roottable,name,rootname) \
   MGD_MOVE_AND_TOUCH(table,roottable,name,rootname,0)

#define MGD_MOVE_AND_TOUCH(table,roottable,name,rootname,touch) \
MGD_FUNCTION(int, move_##name, (int id, int root)) \
{ \
   RETVAL_FALSE; \
   CHECK_MGD; \
   long id, root; \
   if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &id, &root) != SUCCESS) { \
	   return; \
   } \
   if(mgd_move_object(mgd_handle(), #table, #rootname, id, root) RETVAL_TRUE; \
   PHP_UPDATE_REPLIGARD(#table, id); \
   PHP_UPDATE_REPLIGARD(#roottable, root); \
   if (touch) { TOUCH_CACHE; } \
}

extern int midgard_user_call_func(midgard *mgd, int id, int level, void * xparam);

/* Commonly used macros
*/

#define HOSTNAME_FIELD \
  "Concat('http', If(host.port=443, 's',''), '://', host.name, If(host.port=0 || host.port=443, '', Concat(':', host.port)),If(host.prefix='/', '/', Concat(host.prefix, '/'))) AS hostname"

#define PUBLIC_FIELD(n,name) "If(info&" #n "," #name ",'') AS " #name
#define PUBLIC_FIELDS \
  "info&2=2 AS addressp,info&4=4 as phonep,info&8=8 AS homepagep," \
  "info&16=16 AS emailp,info&32=32 AS extrap"

#define NAME_FIELD \
  "Concat(firstname,If(firstname=''||lastname='','',' '),lastname)"
#define RNAME_FIELD \
  "Concat(lastname,If(firstname=''||lastname='','',', '),firstname)"
#define NAME_FIELDS "firstname,lastname,username," \
  NAME_FIELD " AS name," RNAME_FIELD " AS rname"

#define ADDRESS_FIELD "Concat(street," \
  "If(street!=''&&(postcode!=''||city!=''),', ','')," \
  "postcode,If(postcode!=''&&city!='',' ',''),city)"
#define ADDRESS_FIELDS "street,postcode,city," ADDRESS_FIELD " AS address"

#define PHONE_FIELD "Concat(" \
  "handphone,If(handphone=''||(homephone=''&&workphone=''),'',', ')," \
  "homephone,If(homephone=''||workphone='','',', ')," \
  "workphone,If(workphone='','',' (ty�)'))"
#define PHONE_FIELDS "handphone,homephone,workphone," PHONE_FIELD " AS phone"

#define HOMEPAGE_FIELD "If(homepage='',''," \
  "Concat('<a href=\\\"',homepage,'\\\" title=\\\"',firstname,' ',lastname," \
  "'\\\">',homepage,'</a>'))"
#define HOMEPAGE_FIELDS "homepage," HOMEPAGE_FIELD " AS homepagelink"

#define EMAIL_FIELD "If(email='',''," \
  "Concat('<a href=\\\"mailto:',email,'\\\" title=\\\"',firstname,' '," \
  "lastname,'\\\">',email,'</a>'))"
#define EMAIL_FIELDS  "email," EMAIL_FIELD " AS emaillink"

#define GROUP_HOMEPAGE_FIELD "If(homepage='',''," \
  "Concat('<a href=\\\"',homepage,'\\\" title=\\\"',name," \
  "'\\\">',homepage,'</a>'))"
#define GROUP_HOMEPAGE_FIELDS "homepage," \
  GROUP_HOMEPAGE_FIELD " AS homepagelink"

#define GROUP_EMAIL_FIELD "If(email='',''," \
  "Concat('<a href=\\\"mailto:',email,'\\\" title=\\\"',name," \
  "'\\\">',email,'</a>'))"
#define GROUP_EMAIL_FIELDS  "email," GROUP_EMAIL_FIELD " AS emaillink"

#define SITEGROUP_SELECT ",sitegroup"

/* Person macroses */
#define PERSON_SELECT \
  "person.guid AS guid,id,username," NAME_FIELD " AS name," RNAME_FIELD " AS rname,extra," \
  "topic,department,office,info&1 AS admin,info>1 AS public" SITEGROUP_SELECT

/* Article macroses */
#define CALENDAR_FIELD \
  "If(IsNull(calstart),'',If(caldays=0,Date_Format(calstart,'%d.%m.%Y')," \
  "Concat(Date_Format(calstart," \
  "If(Year(calstart)!=Year(From_Days(To_Days(calstart)+caldays)),'%d.%m.%Y'," \
  "If(Month(calstart)!=Month(From_Days(To_Days(calstart)+caldays)),'%d.%m.','%d.')))," \
  "Date_Format(From_Days(To_Days(calstart)+caldays),'-%d.%m.%Y'))))"
#define ACALENDAR_FIELD \
  "If(IsNull(calstart),'',If(caldays=0,Date_Format(calstart,'%D %b. %Y')," \
  "Concat(Date_Format(calstart," \
  "If(Year(calstart)!=Year(From_Days(To_Days(calstart)+caldays)),'%D %b. %Y'," \
  "If(Month(calstart)!=Month(From_Days(To_Days(calstart)+caldays)),'%D %b.','%D')))," \
  "Date_Format(From_Days(To_Days(calstart)+caldays),'-%D %b. %Y'))))"
#define ALCALENDAR_FIELD \
  "If(IsNull(calstart),'',If(caldays=0,Date_Format(calstart,'%D %M %Y')," \
  "Concat(Date_Format(calstart," \
  "If(Year(calstart)!=Year(From_Days(To_Days(calstart)+caldays)),'%D %M %Y'," \
  "If(Month(calstart)!=Month(From_Days(To_Days(calstart)+caldays)),'%D %M','%D')))," \
  "Date_Format(From_Days(To_Days(calstart)+caldays),'-%D %M %Y'))))"
#define CALENDAR_FIELDS \
  CALENDAR_FIELD " AS calendar," \
  ACALENDAR_FIELD " AS acalendar," \
  ALCALENDAR_FIELD " AS alcalendar," \
  "Unix_Timestamp(calstart) AS startdate," \
  "Unix_Timestamp(Date_Add(calstart, INTERVAL caldays DAY)) AS enddate," \
  "caldays,Date_Format(calstart,'%d.%m.%Y') AS calstart," \
  "Date_Format(From_Days(To_Days(calstart)+caldays),'%d.%m.%Y') As calstop"

#define ARTICLE_CALENDAR CALENDAR_FIELDS

#if ! HAVE_MIDGARD_MULTILANG
#define ARTICLE_SITEGROUP_SELECT ",article.sitegroup"
#else
#define ARTICLE_SITEGROUP_SELECT ",article.sitegroup as sitegroup"
#endif

#if ! HAVE_MIDGARD_MULTILANG
#define ARTICLE_SELECT \
  "article.guid AS guid,article.id AS id,article.name AS name,title,abstract,content,author," \
  NAME_FIELD " AS authorname,article.topic AS topic," \
  "Date_format(article.created,'%d.%m.%Y') AS date," \
  "Date_format(article.created,'%D %b. %Y') AS adate," \
  "Date_format(article.created,'%D %M %Y') AS aldate," \
  "url,icon,extra1,extra2,extra3,article.score AS score,type," \
  "Unix_Timestamp(article.created) AS created,article.creator AS creator," \
  "Unix_Timestamp(revised) AS revised,revisor,revision," \
  "Unix_Timestamp(locked) AS locked,locker," \
  "Unix_Timestamp(approved) AS approved,approver" ARTICLE_SITEGROUP_SELECT
#else
#define ARTICLE_SELECT \
  "article.guid AS guid,article.id AS id,up,article.name AS name,title,abstract,content,article.author AS author,article_i.author as contentauthor, " \
  NAME_FIELD " AS authorname,article.topic AS topic," \
  "Date_format(article.created,'%d.%m.%Y') AS date," \
  "Date_format(article.created,'%D %b. %Y') AS adate," \
  "Date_format(article.created,'%D %M %Y') AS aldate," \
  "url,icon,view,print,extra1,extra2,extra3,article.score AS score,type," \
  "Unix_Timestamp(article.created) AS created,article.creator AS creator," \
  "Unix_Timestamp(revised) AS revised,revisor,revision," \
  "Unix_Timestamp(locked) AS locked,locker," \
  "Unix_Timestamp(approved) AS approved,approver,lang" ARTICLE_SITEGROUP_SELECT
#endif

#if ! HAVE_MIDGARD_MULTILANG
#define ARTICLE_SELECT_FAST \
  "article.guid AS guid,id,name,title,abstract,content,author,topic," \
  "Date_format(article.created,'%d.%m.%Y') AS date," \
  "Date_format(article.created,'%D %b. %Y') AS adate," \
  "Date_format(article.created,'%D %M %Y') AS aldate," \
  "url,icon,extra1,extra2,extra3,article.score AS score,type," \
  "Unix_Timestamp(article.created) AS created,article.creator AS creator," \
  "Unix_Timestamp(revised) AS revised,revisor,revision," \
  "Unix_Timestamp(locked) AS locked,locker," \
  "Unix_Timestamp(approved) AS approved,approver" SITEGROUP_SELECT
#else
#define ARTICLE_SELECT_FAST \
  "article.guid AS guid,article.id AS id,up,name,title,abstract,content,article.author AS author,article_i.author as contentauthor,topic," \
  "Date_format(article.created,'%d.%m.%Y') AS date," \
  "Date_format(article.created,'%D %b. %Y') AS adate," \
  "Date_format(article.created,'%D %M %Y') AS aldate," \
  "url,icon,view,print,extra1,extra2,extra3,article.score AS score,type," \
  "Unix_Timestamp(article.created) AS created,article.creator AS creator," \
  "Unix_Timestamp(revised) AS revised,revisor,revision," \
  "Unix_Timestamp(locked) AS locked,locker," \
  "Unix_Timestamp(approved) AS approved,approver,lang" ARTICLE_SITEGROUP_SELECT
#endif

#if ! HAVE_MIDGARD_MULTILANG
#define ARTICLE_FROM "article,person"
#define ARTICLE_FROM_FAST "article"
#else /* HAVE_MIDGARD_MULTILANG */
#define ARTICLE_FROM "article,article_i,person"
#define ARTICLE_FROM_FAST "article,article_i"
#define ARTICLE_I_WHERE " AND article.id = article_i.sid AND article_i.lang = $d"
#endif /* HAVE_MIDGARD_MULTILANG */

/* Macroses for Events */
#define EVENT_SITEGROUP     " AND sitegroup in (0,$d)"
#define EVENT_SITEGROUP2    " AND event.sitegroup in (0,$d)"\
							" AND eventmember.sitegroup in (0,$d)"
#define EVENT_COUNT_WHERE_0 "start>=Unix_Timestamp(Now())" EVENT_SITEGROUP
#define EVENT_COUNT_WHERE_1 "start>=Unix_Timestamp(Now())"\
							" AND end<=$d" EVENT_SITEGROUP
#define EVENT_COUNT_WHERE_2 "start>=$d AND end<=$d" EVENT_SITEGROUP
#define EVENT_COUNT_WHERE_3 "start>=$d AND end<=$d"\
							" AND event.id=eventmember.eid"\
							" AND eventmember.uid=$d" EVENT_SITEGROUP
#define EVENT_COUNT_WHERE_4 "start>=$d AND end<=$d"\
							" AND event.id=eventmember.eid"\
							" AND eventmember.uid=$d"\
							" AND event.type=$d" EVENT_SITEGROUP2
#define EVENT_COUNT_WHERE_43 "start>=$d AND end<=$d"\
							" AND event.type=$d" EVENT_SITEGROUP
#define EVENT_COUNT_TABLE 	"event"
#define EVENT_COUNT_TABLE_2 "event,eventmember"

#define EVENT_START 0
#define EVENT_END   1

#define EVENT_MONTH_WHERE 	   "((start<$d AND end>$d)"\
						  	   " OR (start>$d AND start<$d)"\
						  	   " OR (end>$d AND end<$d))" EVENT_SITEGROUP
#define EVENT_MONTH_WHERE_TYPE "((start<$d AND end>$d)"\
						       " OR (start>$d AND start<$d)"\
							   " OR (end>$d AND end<$d))"\
							   " AND type=$d" EVENT_SITEGROUP
/* Macroses for Event Members */
#define EVENT_PUBLIC_FIELD(n,name) \
		"If(person.id<>$d,If(info&" #n "," #name ",'')," #name ") AS " #name

#define EVENT_EMAIL_FIELD \
		"If(person.id<>$d,If(Info&16,If(email='',''," \
  "Concat('<a href=\\\"mailto:',email,'\\\" title=\\\"',firstname,' '," \
  "lastname,'\\\">',email,'</a>')),'')," \
  "Concat('<a href=\\\"mailto:',email,'\\\" title=\\\"',firstname,' '," \
  "lastname,'\\\">',email,'</a>')) AS emaillink"

/* DG: fixing an incompatibility with a certain state of PHP's CVS...
 * not needed anymore, but who knows...
#ifdef add_property_unset
#undef add_property_unset
#define add_property_unset(__arg, __key) add_property_unset_ex(__arg, __key, strlen(__key) + 1)
#endif
*/

#define MGD_INIT_CLASS_ENTRY(class_container, class_name, functions) \
	{ \
		class_container.name = strdup(class_name); \
		class_container.name_length = strlen(class_name); \
		class_container.builtin_functions = functions; \
		class_container.handle_function_call = NULL; \
		class_container.handle_property_get = NULL; \
		class_container.handle_property_set = NULL; \
	}
#if  (PHP_MAJOR_VERSION < 5)

#define MGD_INIT_OVERLOADED_CLASS_ENTRY(class_container, class_name, functions, handle_fcall, handle_propget, handle_propset) \
	{															\
		class_container.name = strdup(class_name);				\
		class_container.name_length = strlen(class_name);		\
		class_container.builtin_functions = functions;			\
		class_container.handle_function_call = handle_fcall;	\
		class_container.handle_property_get = handle_propget;	\
		class_container.handle_property_set = handle_propset;	\
	}

#else 

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

#endif /* PHP_MAJOR_VERSION < 5 */


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
