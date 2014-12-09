/* $Id: midgard.c 27410 2014-09-01 07:39:28Z piotras $
Copyright (C) 1999 Jukka Zitting <jukka.zitting@iki.fi>
Copyright (C) 2000 The Midgard Project ry
Copyright (C) 2000 Emile Heyns, Aurora SA <emile@iris-advies.com>
Copyright (C) 2003 David Schmitter, Dataflow Solutions GmbH <schmitt@dataflow.ch>
Copyright (C) 2005 Piotr Pokora, <pp@infoglob.pl>

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

#include <sys/file.h>
#include "php_midgard.h"
#include "php_midgard_gobject.h"

/* You should tweak config.m4 so this symbol (or some else suitable)
   gets defined.
*/
#if HAVE_MIDGARD

#include "mgd_article.h"
#include "mgd_topic.h"
#include "mgd_attachment.h"
#include "mgd_element.h"
#include "mgd_group.h"
#include "mgd_member.h"
#include "mgd_host.h"
#include "mgd_calendar.h"
#include "mgd_event.h"
#include "mgd_eventmember.h"
#include "mgd_page.h"
#include "mgd_pageelement.h"
#include "mgd_pagelink.h"
#include "mgd_person.h"
#include "mgd_sitegroup.h"
#include "mgd_snippet.h"
#include "mgd_snippetdir.h"
#include "mgd_language.h"
#include "mgd_quota.h"
#include "mgd_preparser.h"
#include "mgd_preparse.h"
#include "mgd_query_builder.h"
#include "mgd_php_reflection_property.h"
#include "mgd_style.h"

#include "zend_exceptions.h"
#include "zend_extensions.h"
#include <zend_ini.h>

#include <locale.h>

#include "SAPI.h"

/* True global resources - no need for thread safety here */
int le_midgard_list_fetch;
GHashTable * midgard_registered_types;
static MidgardSchema *midgard_global_schema = NULL;
static zend_class_entry *midgard_metadata_class;
static zend_class_entry *ce_midgard_error_exception;
zend_object_handlers php_midgard_gobject_handlers;
guint global_loghandler = 0;
/* End of true globals */

#ifndef PHP_MIDGARD_LEGACY_API
php_midgard_function_entry midgard_functions[] = {
PHP_FE(mgd_get_midgard, NULL)
PHP_FE(mgd_preparse, NULL)
PHP_FE(mgd_snippet, NULL)
PHP_FE(mgd_errstr, NULL)
PHP_FE(mgd_errno, NULL)
PHP_FE(mgd_set_errno, NULL)
PHP_FE(mgd_version, NULL)
PHP_FE(mgd_auth_midgard, NULL)
PHP_FE(mgd_unsetuid, NULL)
PHP_FE(mgd_issetuid, NULL)
PHP_FE(mgd_is_guid, NULL)
PHP_FE(mgd_format, NULL)
PHP_FE(mgd_set_lang, NULL)
PHP_FE(mgd_get_lang, NULL)
PHP_FE(mgd_set_default_lang, NULL)
PHP_FE(mgd_get_default_lang, NULL)
PHP_FE(mgd_debug_start, NULL)
PHP_FE(mgd_debug_stop, NULL)
PHP_FE(mgd_config_init, NULL)
PHP_FE(mgd_is_element_loaded, NULL)
PHP_FE(mgd_stat_attachment, NULL)
MGD_FE(cache_invalidate, NULL)
MGD_FE(get_sitegroup, NULL)
MGD_FE(template, NULL)
{NULL, NULL, NULL}  /* Must be the last line in midgard_functions[] */
};
#else
MGD_FUNCTION(ret_type, errno, (type param));
MGD_FUNCTION(ret_type, errstr, (type param));
MGD_FUNCTION(ret_type, version, (type param));
#if HAVE_MIDGARD_MULTILANG
MGD_FUNCTION(ret_type, get_lang, (type param));
MGD_FUNCTION(ret_type, set_lang, (type param));
MGD_FUNCTION(ret_type, set_default_lang, (type param));
MGD_FUNCTION(ret_type, get_default_lang, (type param));
MGD_FUNCTION(ret_type, set_lang_by_code, (type param));
MGD_FUNCTION(ret_type, get_parameters_defaultlang, (type param));
MGD_FUNCTION(ret_type, get_attachments_defaultlang, (type param));
MGD_FUNCTION(ret_type, set_parameters_defaultlang, (type param));
MGD_FUNCTION(ret_type, set_attachments_defaultlang, (type param));
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FUNCTION(ret_type, get_midgard, (type param));
MGD_FUNCTION(ret_type, auth_midgard, (type param));
MGD_FUNCTION(ret_type, unsetuid, (type param));
MGD_FUNCTION(ret_type, issetuid, (type param));
MGD_FUNCTION(ret_type, preparser_active, (type param));
MGD_FUNCTION(ret_type, debug_start, (type param));
MGD_FUNCTION(ret_type, debug_stop, (type param));
MGD_FUNCTION(ret_type, register_style, (type param));
MGD_FUNCTION(ret_type, register_style_from_dir, (type param));
MGD_FUNCTION(ret_type, set_style, (type param));
MGD_FUNCTION(ret_type, set_styledir, (type param));
MGD_FUNCTION(ret_type, config_init, (type_param));

MGD_FUNCTION(ret_type, is_guid, (type_param));

/* Undocumented */
MGD_FUNCTION(ret_type, cache_invalidate, (type_param));
MGD_FUNCTION(ret_type, set_errno, (type_param));
MGD_FUNCTION(ret_type, reset_errno, (type_param));

/* Every user visible function must have an entry in midgard_functions[].
*/
php_midgard_function_entry midgard_functions[] = {
MGD_FE(is_article_owner, NULL)
MGD_FE(is_article_in_topic_tree, NULL)
MGD_FE(list_topic_articles, NULL)
MGD_FE(list_reply_articles, NULL)
MGD_FE(list_topic_articles_all, NULL)
MGD_FE(list_topic_articles_all_fast, NULL)
MGD_FE(list_topic_articles_all_of_person, NULL)
MGD_FE(get_article, NULL)
MGD_FALIAS(get_article_by_name, get_article, NULL)
MGD_FE(get_reply_by_name, NULL)
MGD_FE(create_article, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(create_article_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(update_article_score, NULL)
MGD_FE(update_article_created, NULL)
MGD_FE(update_article_replyto, NULL)
MGD_FE(update_article_type, NULL)
MGD_FE(toggle_article_lock, NULL)
MGD_FE(approve_article, NULL)
MGD_FE(update_article, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(update_article_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(delete_article, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(delete_article_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(copy_article, NULL)
MGD_FE(move_article, NULL)
MGD_FE(move_reply, NULL)
MGD_FE(delete_article_tree, NULL)
MGD_FE(open_attachment, NULL)
MGD_FE(get_attachment, NULL)
MGD_FE(serve_attachment, NULL)
MGD_FE(stat_attachment, NULL)
MGD_FE(delete_attachment, NULL)
MGD_FE(update_attachment, NULL)
//MGD_FE(errno, NULL)
//MGD_FE(errstr, NULL)
//MGD_FE(version, NULL)
MGD_FE(is_topic_owner, NULL)
MGD_FE(list_topics, NULL)
MGD_FE(is_in_topic_tree, NULL)
MGD_FE(get_topic, NULL)
MGD_FALIAS(get_topic_by_name, get_topic, NULL)
MGD_FE(create_topic, NULL)
MGD_FE(update_topic, NULL)
MGD_FE(update_topic_score, NULL)
MGD_FE(delete_topic, NULL)
MGD_FE(copy_topic, NULL)
MGD_FE(move_topic, NULL)
MGD_FE(delete_topic_tree, NULL)
MGD_FE(get_topic_by_path, NULL)
MGD_FE(list_elements, NULL)
MGD_FE(get_element, NULL)
MGD_FALIAS(get_element_by_name, get_element, NULL)
MGD_FE(create_element, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(create_element_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(update_element, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(update_element_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(delete_element, NULL)
MGD_FE(delete_element_tree, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(delete_element_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(copy_element, NULL)
MGD_FE(move_element, NULL)
MGD_FE(errno, NULL)
MGD_FE(errstr, NULL)
MGD_FE(version, NULL)
MGD_FE(get_midgard, NULL)
MGD_FE(auth_midgard, NULL)
MGD_FE(unsetuid, NULL)
MGD_FE(issetuid, NULL)
MGD_FE(preparser_active, NULL)
MGD_FE(is_group_owner, NULL)
MGD_FE(list_groups, NULL)
MGD_FE(get_group, NULL)
MGD_FE(get_group_by_name, NULL)
MGD_FE(create_group, NULL)
MGD_FE(update_group, NULL)
MGD_FE(delete_group, NULL)
MGD_FE(list_members, NULL)
MGD_FE(list_memberships, NULL)
MGD_FE(get_member, NULL)
MGD_FE(create_member, NULL)
MGD_FE(update_member, NULL)
MGD_FE(delete_member, NULL)
MGD_FE(is_host_owner, NULL)
MGD_FE(list_hosts, NULL)
MGD_FE(get_host, NULL)
MGD_FE(get_host_by_name, NULL)
MGD_FE(create_host, NULL)
MGD_FE(update_host, NULL)
MGD_FE(delete_host, NULL)
MGD_FE(list_topic_calendar_all, NULL)
MGD_FE(list_events_by_group, NULL)
MGD_FE(list_events_between_by_group, NULL)
MGD_FE(list_events_by_person, NULL)
MGD_FE(list_events_between_by_person, NULL)
MGD_FE(list_events_between_by_member, NULL)
MGD_FE(list_topic_calendar_all_fast, NULL)
MGD_FE(is_event_owner, NULL)
MGD_FE(create_event, NULL)
MGD_FE(update_event, NULL)
MGD_FE(delete_event, NULL)
MGD_FE(delete_event_tree, NULL)
MGD_FE(get_event, NULL)
MGD_FE(list_events, NULL)
MGD_FE(list_events_between, NULL)
MGD_FE(list_events_all, NULL)
MGD_FE(list_events_all_between, NULL)
MGD_FE(count_events_in_period, NULL)
MGD_FE(count_events_in_month, NULL)
MGD_FE(copy_event, NULL)
MGD_FE(move_event, NULL)
MGD_FE(create_event_member, NULL)
MGD_FE(update_event_member, NULL)
MGD_FE(delete_event_member, NULL)
MGD_FE(get_event_member, NULL)
MGD_FE(list_event_members, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(create_page_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(count_event_members, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(update_page_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(is_page_owner, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(delete_page_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(copy_page, NULL)
MGD_FE(move_page, NULL)
MGD_FE(list_pages, NULL)
MGD_FE(is_in_page_tree, NULL)
MGD_FE(get_page, NULL)
MGD_FE(get_page_by_name, NULL)
MGD_FE(create_page, NULL)
MGD_FE(update_page, NULL)
MGD_FE(delete_page, NULL)
MGD_FE(page_has_children, NULL)
MGD_FE(delete_page_tree, NULL)
MGD_FE(list_page_elements, NULL)
MGD_FE(get_page_element, NULL)
MGD_FE(get_page_element_by_name, NULL)
MGD_FE(create_page_element, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(create_page_element_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(update_page_element, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(update_page_element_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(delete_page_element, NULL)
MGD_FE(delete_page_element_tree, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(delete_page_element_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(copy_page_element, NULL)
MGD_FE(move_page_element, NULL)
MGD_FE(has_pagelinks, NULL)
#if HAVE_MIDGARD_PAGELINKS
MGD_FE(is_pagelink_owner, NULL)
MGD_FE(list_pagelinks, NULL)
MGD_FE(list_pagelinks_targeted_at, NULL)
MGD_FE(get_pagelink, NULL)
MGD_FE(get_pagelink_by_name, NULL)
MGD_FE(create_pagelink, NULL)
MGD_FE(update_pagelink, NULL)
MGD_FE(delete_pagelink, NULL)
#endif
MGD_FE(is_person_owner, NULL)
MGD_FE(is_member, NULL)
MGD_FE(list_persons, NULL)
MGD_FE(list_persons_in_department, NULL)
MGD_FALIAS(list_topic_persons, list_persons_in_department, NULL)
MGD_FE(list_persons_in_department_all, NULL)
MGD_FALIAS(list_topic_persons_all, list_persons_in_department_all, NULL)
MGD_FE(list_persons_in_office, NULL)
MGD_FE(get_person, NULL)
MGD_FE(get_person_by_name, NULL)
MGD_FE(create_person, NULL)
MGD_FE(update_person, NULL)
MGD_FE(update_password, NULL)
MGD_FE(update_password_plain, NULL)
MGD_FE(update_public, NULL)
MGD_FE(delete_person, NULL)
MGD_FE(has_sitegroups, NULL)
MGD_FE(list_sitegroups, NULL)
MGD_FE(create_sitegroup, NULL)
MGD_FE(get_sitegroup, NULL)
MGD_FE(update_sitegroup, NULL)
MGD_FE(delete_sitegroup, NULL)
MGD_FE(snippet_exists, NULL)
MGD_FE(list_snippets, NULL)
MGD_FE(get_snippet, NULL)
MGD_FE(get_snippet_by_name, NULL)
MGD_FE(create_snippet, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(create_snippet_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(update_snippet, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(update_snippet_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(delete_snippet, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(delete_snippet_content, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
MGD_FE(copy_snippet, NULL)
MGD_FE(move_snippet, NULL)
MGD_FE(get_snippet_by_path, NULL)
MGD_FE(is_snippetdir_owner, NULL)
MGD_FE(list_snippetdirs, NULL)
MGD_FE(get_snippetdir, NULL)
MGD_FE(get_snippetdir_by_path, NULL)
MGD_FE(create_snippetdir, NULL)
MGD_FE(update_snippetdir, NULL)
MGD_FE(delete_snippetdir, NULL)
MGD_FE(delete_snippetdir_tree, NULL)
MGD_FE(copy_snippetdir, NULL)
MGD_FE(move_snippetdir, NULL)
MGD_FE(walk_style_tree, NULL)
MGD_FE(walk_topic_tree, NULL)
MGD_FE(walk_article_tree, NULL)
MGD_FE(walk_page_tree, NULL)
MGD_FE(walk_snippetdir_tree, NULL)
MGD_FE(walk_event_tree, NULL)
MGD_FE(walk_group_tree, NULL)
MGD_FE(is_style_owner, NULL)
MGD_FE(set_lang, NULL)
MGD_FE(set_parameters_defaultlang, NULL)
MGD_FE(set_attachments_defaultlang, NULL)
MGD_FE(set_lang_by_code, NULL)
MGD_FE(get_lang, NULL)
MGD_FE(get_default_lang, NULL)
MGD_FE(set_default_lang, NULL)
MGD_FE(get_parameters_defaultlang, NULL)
MGD_FE(get_attachments_defaultlang, NULL)
MGD_FE(list_styles, NULL)
MGD_FE(get_style, NULL)
MGD_FE(get_style_by_name, NULL)
MGD_FE(create_style, NULL)
MGD_FE(update_style, NULL)
MGD_FE(delete_style, NULL)
MGD_FE(copy_style, NULL)
MGD_FE(move_style, NULL)
MGD_FE(delete_style_tree, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(has_multilang, NULL)
MGD_FE(list_languages, NULL)
MGD_FE(get_language, NULL)
MGD_FE(get_language_by_code, NULL)
MGD_FE(create_language, NULL)
MGD_FE(update_language, NULL)
MGD_FE(delete_language, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */
#if HAVE_MIDGARD_QUOTA
MGD_FE(list_quotas, NULL)
MGD_FE(get_quota, NULL)
MGD_FE(get_quota_by_tablename, NULL)
MGD_FE(get_quota_info_by_tablename, NULL)
MGD_FE(create_quota, NULL)
MGD_FE(update_quota, NULL)
MGD_FE(delete_quota, NULL)
MGD_FE(get_sitegroup_size, NULL)
#endif /* HAVE_MIDGARD_QUOTA */
MGD_FE(get_object_by_guid, NULL)
#if HAVE_MIDGARD_MULTILANG
MGD_FE(get_object_by_guid_all_langs, NULL)
MGD_FE(is_owner_by_guid, NULL)
#endif /* HAVE_MIDGARD_MULTILANG */

/* preparser functions */
MGD_FE(snippet, NULL)
MGD_FE(ref, NULL)
MGD_FE(preparse, NULL)
MGD_FE(format, NULL)
MGD_FE(template, NULL)

#if MIDGARD_142MOD
MGD_FE(eval, NULL)
MGD_FE(variable, NULL)
#endif

MGD_FE(debug_start, NULL)
MGD_FE(debug_stop, NULL)
MGD_FE(register_style, NULL)
MGD_FE(register_style_from_dir, NULL)
MGD_FE(set_style, NULL)
MGD_FE(set_styledir, NULL)
MGD_FE(is_element_loaded, NULL)
MGD_FE(config_init, NULL)
MGD_FE(is_guid, NULL)

/* Undocumented */
MGD_FE(cache_invalidate, NULL)
MGD_FE(set_errno, NULL)
MGD_FE(reset_errno, NULL)
    {NULL, NULL, NULL}  /* Must be the last line in midgard_functions[] */
};
#endif

void php_midgard_log_errors(const gchar *domain, GLogLevelFlags level,
		const gchar *msg, gpointer userdata)
{
	MidgardConnection *mgd = (MidgardConnection*) userdata;
	guint mlevel = 0;
	if(mgd)
		mlevel = midgard_connection_get_loglevel(mgd);
	
	g_assert(msg != NULL);

	switch (level) {
				
		case G_LOG_LEVEL_ERROR:
			php_error(E_ERROR,
					"(pid:%ld): %s",
					(unsigned long)getpid(), msg);
			return;
			break;
			
		case G_LOG_LEVEL_CRITICAL:
			php_error(E_WARNING,
					"(pid:%ld): %s",
					(unsigned long)getpid(), msg);
			return;
			break;
			
		case G_LOG_LEVEL_WARNING: 
			php_error(E_WARNING,
					"(pid:%ld): %s",
					(unsigned long)getpid(), msg);					
			return;
			break;
					
		default:

			break;
	}	

	if (mlevel >= level && mgd != NULL) {

		midgard_error_default_log(
				domain, level, msg, MIDGARD_CONNECTION(mgd));
	}
}

void php_glib_log_errors(const gchar *domain, GLogLevelFlags level,
		const gchar *_msg, gpointer userdata)
{
	if(_msg == NULL)
		_msg = "";

	 zval **data;
	 gchar *ruri;
	 gchar *rhost;
	TSRMLS_FETCH();

	 if(zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]),
	 	"REQUEST_URI", sizeof("REQUEST_URI"), (void **) &data) == FAILURE) {
		
		ruri = "Empty request uri";

	} else {
		
		ruri = Z_STRVAL_PP(data);
	}

	zval **hdata;
	
	if(zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]),
		"HTTP_HOST", sizeof("HTTP_HOST"), (void **) &hdata) == FAILURE) {
		
		rhost = "Empty hostname";
	
	} else {

	        rhost = Z_STRVAL_PP(hdata);
	}


	GString *mstr = g_string_new("");
	g_string_append_printf(mstr, "%s (%s %s)", _msg, rhost, ruri);

	gchar *msg = mstr->str;

	switch (level) {
		
		case G_LOG_FLAG_RECURSION:
			/* Do nothing */
			break;
		
		case G_LOG_LEVEL_CRITICAL:
		case G_LOG_FLAG_FATAL:
		case G_LOG_LEVEL_ERROR:
			php_error(E_WARNING,
					"(pid:%ld): %s",
					(unsigned long)getpid(), msg);
			break;
			
		case G_LOG_LEVEL_MESSAGE:
		case G_LOG_LEVEL_WARNING: 	
			php_error(E_WARNING,
					"(pid:%ld): %s",
					(unsigned long)getpid(), msg);					
			break;
					
		case G_LOG_LEVEL_INFO:
			php_error(E_NOTICE, "(pid:%ld): %s", (unsigned long)getpid(), msg);
			break;
			
		case G_LOG_LEVEL_DEBUG:
			/* Do nothing */
			break;
			
		default:
			break;
	}	

	g_string_free(mstr, TRUE);
}

gboolean php_midgard_error_exception_throw(midgard *mgd)
{
	MidgardConnection *_mgd = mgd->_mgd;

	if(_mgd->errnum != MGD_ERR_OK) {
		TSRMLS_FETCH();
		zend_throw_exception_ex(ce_midgard_error_exception,
				0 TSRMLS_CC, _mgd->errstr);
		return TRUE;
	}
	
	return FALSE;
}

void php_midgard_error_exception_force_throw(midgard *mgd, gint errcode)
{
	midgard_connection_set_error(mgd->_mgd, errcode);
	php_midgard_error_exception_throw(mgd);

	return;
}

ZEND_DECLARE_MODULE_GLOBALS(midgard) 

zend_module_entry midgard_module_entry = {
	STANDARD_MODULE_HEADER,
    	"midgard",
    	midgard_functions,
    	PHP_MINIT(midgard),
    	PHP_MSHUTDOWN(midgard),
    	PHP_RINIT(midgard),     /* Replace with NULL if there's nothing to do at request start */
    	PHP_RSHUTDOWN(midgard), /* Replace with NULL if there's nothing to do at request end */
    	PHP_MINFO(midgard),
    	MIDGARD_LIB_VERSION,          /* extension version number (string) */
    	STANDARD_MODULE_PROPERTIES
};

ZEND_GET_MODULE(midgard)

ZEND_INI_BEGIN()
ZEND_INI_ENTRY("midgard.engine", "On", ZEND_INI_ALL, NULL)
ZEND_INI_END()

static gboolean php_midgard_engine_is_enabled()
{
	gchar *engine = zend_ini_string("midgard.engine", sizeof("midgard.engine"), 0);
	gchar *lengine = g_ascii_strdown(engine, strlen(engine));
	
	gboolean enabled = FALSE;
	
	if(g_str_equal(lengine, "on"))
		enabled = TRUE;
		
	g_free(lengine);
	
	return enabled;
}

static void php_midgard_init_globals(zend_midgard_globals *midgard_globals)
{
    midgard_globals->rcfg = NULL;
    midgard_globals->dcfg = NULL;
    midgard_globals->mgd = NULL;
    midgard_globals->mgd_errno = MGD_ERR_OK;
}

static void _midgard_list_fetch_dtor(zend_rsrc_list_entry * rsrc TSRMLS_DC)
{
   midgard_res * res = (midgard_res*)rsrc->ptr;
   if(rsrc->type == IS_RESOURCE){  
      if (res != NULL) mgd_release(res);
    }
}

static zend_bool create_global_zval(const char *name, uint name_len TSRMLS_DC)
{
	zval *globals;
	ALLOC_ZVAL(globals);
	zend_hash_update(&EG(symbol_table), name, name_len + 1, &globals, sizeof(zval *), NULL);
	return 0;
}

static zend_bool create_global_array(const char *name, uint name_len TSRMLS_DC)
{
	zval *globals;
	ALLOC_ZVAL(globals);
	array_init(globals);
	INIT_PZVAL(globals);
	zend_hash_update(&EG(symbol_table), name, name_len + 1, &globals, sizeof(zval *), NULL);
	return 0;
}

void php_midgard_register_auto_globals(void)
{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	zend_register_auto_global(MIDGARD_GLOBAL_MIDGARD, sizeof(MIDGARD_GLOBAL_MIDGARD)-1, 0, NULL TSRMLS_CC);
	zend_register_auto_global(MIDGARD_GLOBAL_MIDCOM, sizeof(MIDGARD_GLOBAL_MIDCOM)-1, 0, NULL TSRMLS_CC);
	zend_register_auto_global(MIDGARD_GLOBAL_MIDGARD_CONNECTION, sizeof(MIDGARD_GLOBAL_MIDGARD_CONNECTION)-1, 0, NULL TSRMLS_CC);
#else
	zend_register_auto_global(MIDGARD_GLOBAL_MIDGARD, sizeof(MIDGARD_GLOBAL_MIDGARD)-1, NULL TSRMLS_CC);
	zend_register_auto_global(MIDGARD_GLOBAL_MIDCOM, sizeof(MIDGARD_GLOBAL_MIDCOM)-1, NULL TSRMLS_CC);
	zend_register_auto_global(MIDGARD_GLOBAL_MIDGARD_CONNECTION, sizeof(MIDGARD_GLOBAL_MIDGARD_CONNECTION)-1, NULL TSRMLS_CC);
#endif
	return;
}

PHP_MINIT_FUNCTION(midgard)
{
	/* Return success if there's module midgard already loaded.
	 * Internal classes won't be (de)registered, and dl() should be safe in usage */
	zend_extension *ze = zend_get_extension("midgard");
	if(ze != NULL) {
		php_error(E_NOTICE, "Module midgard (1.9) already loaded");
		return SUCCESS;
	}

	global_loghandler =
		g_log_set_handler("midgard-core", G_LOG_LEVEL_MASK,
				midgard_error_default_log, NULL); 

	/* Set loghandler for GLib and GLib-GObject */
	g_log_set_handler("GLib-GObject", G_LOG_LEVEL_MASK, php_glib_log_errors, NULL); 
	g_log_set_handler("GLib", G_LOG_LEVEL_MASK, php_glib_log_errors, NULL);  

	MidgardClassPtr *midgard_class;
	
	ZEND_INIT_MODULE_GLOBALS(midgard, php_midgard_init_globals, NULL);
	php_midgard_register_auto_globals();
	
	/* EEH: I have 0 clue as to what these params mean, and the PHP
	 * docs on these are non-existant
	 */
	le_midgard_list_fetch =
		zend_register_list_destructors_ex(_midgard_list_fetch_dtor, NULL,
				"midgard list fetch", module_number);
	
//#ifdef PHP_MIDGARD_LEGACY_API
	for (midgard_class = MidgardClasses; midgard_class &&
			*midgard_class; midgard_class++) {
		if(*midgard_class && (*midgard_class)->name) {
			MGD_INIT_OVERLOADED_CLASS_ENTRY(
					(*midgard_class)->class_entry,
					(*midgard_class)->name,
					(*midgard_class)->methods,
					NULL,
					NULL,
					NULL);
			(*midgard_class)->entry_ptr =
				zend_register_internal_class(
						&((*midgard_class)->class_entry) TSRMLS_CC);
			assert((*midgard_class)->entry_ptr);
		}
	}
//#endif
	
	midgard_init();

	/* Initialize handlers */
	memcpy(&php_midgard_gobject_handlers,
			zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	/* Custom handlers hooks */
	php_midgard_gobject_handlers.clone_obj = NULL;
	php_midgard_gobject_handlers.read_property = php_midgard_gobject_read_property;
	php_midgard_gobject_handlers.write_property = php_midgard_gobject_write_property;
	php_midgard_gobject_handlers.has_property = php_midgard_gobject_has_property;
	php_midgard_gobject_handlers.get_properties = php_midgard_zendobject_get_properties;
	/* php_midgard_gobject_handlers.unset_property = php_midgard_gobject_unset_property;  */

	/* register Gtype types from schemas */
	midgard_global_schema = g_object_new(MIDGARD_TYPE_SCHEMA, NULL);
	midgard_schema_init((MidgardSchema *) midgard_global_schema);
	midgard_schema_read_dir((MidgardSchema *) midgard_global_schema, MIDGARD_LSCHEMA_DIR);

	/* Register midgard_error_exception class */
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "midgard_error_exception", NULL);
	ce_midgard_error_exception = zend_register_internal_class_ex(&ce,
			zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);

	php_midgard_query_builder_init(module_number);
	php_midgard_reflection_property_init(module_number);
	php_midgard_collector_init(module_number);
	php_midgard_replicator_init(module_number);
	php_midgard_object_class_init(module_number);
	php_midgard_blob_init(module_number);
	php_midgard_connection_init(module_number);
	php_midgard_config_init(module_number);	
	php_midgard_object_init(module_number);
	php_midgard_user_init(module_number);
	php_midgard_dbus_init(module_number);
	php_midgard_sitegroup_init(module_number);

	/* Register midgard_metadata class */
	static zend_class_entry midgard_metadata_class_entry;
	INIT_CLASS_ENTRY(midgard_metadata_class_entry, "midgard_metadata", NULL);
	midgard_metadata_class = zend_register_internal_class(&midgard_metadata_class_entry TSRMLS_CC);
	midgard_metadata_class->create_object = php_midgard_gobject_new;

	/* Register auth type constants */
	REGISTER_LONG_CONSTANT("MIDGARD_USER_HASH_LEGACY", 
			MIDGARD_USER_HASH_LEGACY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MIDGARD_USER_HASH_LEGACY_PLAIN", 
			MIDGARD_USER_HASH_LEGACY_PLAIN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MIDGARD_USER_HASH_CRYPT", 
			MIDGARD_USER_HASH_CRYPT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MIDGARD_USER_HASH_MD5", 
			MIDGARD_USER_HASH_MD5, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MIDGARD_USER_HASH_PLAIN", 
			MIDGARD_USER_HASH_PLAIN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MIDGARD_USER_HASH_SHA1", 
			MIDGARD_USER_HASH_SHA1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MIDGARD_USER_HASH_PAM", 
			MIDGARD_USER_HASH_PAM, CONST_CS | CONST_PERSISTENT);

	/* Register properties' midgard types */
	REGISTER_LONG_CONSTANT("MGD_TYPE_NONE", MGD_TYPE_NONE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_TYPE_STRING", MGD_TYPE_STRING, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_TYPE_INT", MGD_TYPE_INT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_TYPE_UINT", MGD_TYPE_UINT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_TYPE_FLOAT", MGD_TYPE_FLOAT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_TYPE_BOOLEAN", MGD_TYPE_BOOLEAN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_TYPE_TIMESTAMP", MGD_TYPE_TIMESTAMP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_TYPE_LONGTEXT", MGD_TYPE_LONGTEXT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_TYPE_GUID", MGD_TYPE_GUID, CONST_CS | CONST_PERSISTENT);
	
	/* Register errcode constants */
	REGISTER_LONG_CONSTANT("MGD_ERR_OK", MGD_ERR_OK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_ERROR", MGD_ERR_ERROR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_ACCESS_DENIED", MGD_ERR_ACCESS_DENIED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_SITEGROUP_VIOLATION", MGD_ERR_SITEGROUP_VIOLATION, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_NOT_OBJECT", MGD_ERR_NOT_OBJECT, CONST_CS | CONST_PERSISTENT);	
	REGISTER_LONG_CONSTANT("MGD_ERR_NOT_EXISTS", MGD_ERR_NOT_EXISTS, CONST_CS | CONST_PERSISTENT);	
	REGISTER_LONG_CONSTANT("MGD_ERR_INVALID_PROPERTY", MGD_ERR_INVALID_PROPERTY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_INVALID_NAME", MGD_ERR_INVALID_NAME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_DUPLICATE", MGD_ERR_DUPLICATE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_HAS_DEPENDANTS", MGD_ERR_HAS_DEPENDANTS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_RANGE", MGD_ERR_RANGE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_NOT_CONNECTED", MGD_ERR_NOT_CONNECTED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_SG_NOTFOUND", MGD_ERR_SG_NOTFOUND, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_INVALID_OBJECT", MGD_ERR_INVALID_OBJECT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_QUOTA", MGD_ERR_QUOTA, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_INTERNAL", MGD_ERR_INTERNAL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_OBJECT_NAME_EXISTS", MGD_ERR_OBJECT_NAME_EXISTS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_OBJECT_NO_STORAGE", MGD_ERR_OBJECT_NO_STORAGE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_OBJECT_NO_PARENT", MGD_ERR_OBJECT_NO_PARENT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_INVALID_PROPERTY_VALUE", MGD_ERR_INVALID_PROPERTY_VALUE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_USER_DATA", MGD_ERR_USER_DATA, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_OBJECT_DELETED", MGD_ERR_OBJECT_DELETED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_OBJECT_PURGED", MGD_ERR_OBJECT_PURGED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_OBJECT_EXPORTED", MGD_ERR_OBJECT_EXPORTED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_OBJECT_IMPORTED", MGD_ERR_OBJECT_IMPORTED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_MISSED_DEPENDENCE", MGD_ERR_MISSED_DEPENDENCE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_TREE_IS_CIRCULAR", MGD_ERR_TREE_IS_CIRCULAR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MGD_ERR_OBJECT_IS_LOCKED", MGD_ERR_OBJECT_IS_LOCKED, CONST_CS | CONST_PERSISTENT);

	REGISTER_INI_ENTRIES();

	g_log_remove_handler("midgard-core", global_loghandler);
	global_loghandler = 0;

	g_log_set_always_fatal(G_LOG_LEVEL_ERROR);
	/* g_log_set_fatal_mask("GLib-GObject", G_LOG_LEVEL_WARNING); 
	g_log_set_fatal_mask("GLib", G_LOG_LEVEL_WARNING);  */

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(midgard)
{
	UNREGISTER_INI_ENTRIES(); 
	return SUCCESS;
}

static gboolean __rcfg_is_valid(midgard_request_config *rcfg)
{
	if(rcfg == NULL)
		return FALSE;

	if(rcfg->uri == NULL) {
		g_log("GLib", G_LOG_LEVEL_WARNING, "NULL uri for given rcfg");
		return FALSE;
	}

	return TRUE;
}

PHP_RINIT_FUNCTION(midgard)
{
	/* g_log_set_always_fatal(G_LOG_LEVEL_WARNING);  */

	global_loghandler = 0;
	MGDG(mgd) = NULL;
	MGDG(rcfg) = NULL;
	MGDG(dcfg) = NULL;

	//if(!php_midgard_engine_is_enabled())
	//	return SUCCESS;

	if (SG(server_context)) {

		MGDG(rcfg) = mgd_php_bug_workaround_get_rcfg();
		MGDG(dcfg) = mgd_php_bug_workaround_get_dcfg();
		
		if(!__rcfg_is_valid(MGDG(rcfg)))
			return SUCCESS;
	}

	if (MGDG(rcfg)) {	

		MGDG(mgd) = MGDG(rcfg)->mgd;

	} else {

		MGDG(mgd) = NULL; 
	}

	if(SG(server_context) && MGDG(mgd) == NULL)
	{
		return SUCCESS;
	}

	/* MGDG(dcfg) is Apache directory config , so we initialize classes only when 
	 * php is loaded as Apache module. For command line PHP we need to call another 
	 * function which initialize all internal midgard data.
	 * Glib types are already initialized at this moment.  */ 
	
	if (MGDG(dcfg) && MGDG(mgd)) { 	
		MGDG(mgd)->loglevel = MGDG(dcfg)->loglevel;
		MGDG(mgd)->logfile = g_strdup(MGDG(dcfg)->logfile);
	}

	if (MGDG(rcfg)) 
		_make_midgard_global();
	
	if(MGDG(mgd) != NULL) {
		
		/* Initialize _MIDGARD_CONNECTION singleton */
		zval *_mcg = NULL;
		MAKE_STD_ZVAL(_mcg);
		php_midgard_gobject_init(_mcg, "midgard_connection",
				G_OBJECT(mgd_handle()->_mgd), FALSE);
		/* PHP object constructor hack.
		 * I have no idea how to "trigger" php constructor here.
		 * And I didn't try ( yet ) call_user_function */
		zend_hash_add(&EG(symbol_table), "_MIDGARD_CONNECTION",
				sizeof("_MIDGARD_CONNECTION"),
				&_mcg, sizeof(zval **), NULL);

		/* Set MidgardConnection pointer */
		mgd_handle_singleton_set(mgd_handle()->_mgd);

		/* Connect to default callbacks, required for $_MIDGARD superglobal */
		php_midgard_connection_connect_callbacks(mgd_handle()->_mgd);

		global_loghandler =
			g_log_set_handler("midgard-core",
					G_LOG_LEVEL_MASK,
					php_midgard_log_errors,
					(gpointer)mgd_handle_singleton_get());

		midgard_connection_set_loghandler(
				mgd_handle_singleton_get(), global_loghandler);
	}

	if (MGDG(mgd) == NULL)
	{
		global_loghandler =
			g_log_set_handler("midgard-core", G_LOG_LEVEL_MASK,
					midgard_error_default_log, NULL);	
	}
	
	/* Initialize closure hash */
	php_midgard_gobject_closure_hash_new();

	return SUCCESS;	
}

/* Remove if there's nothing to do at request end */
PHP_RSHUTDOWN_FUNCTION(midgard)
{
	if(!php_midgard_engine_is_enabled())
		return SUCCESS;

	if(SG(server_context) && MGDG(mgd) == NULL)
	{
		return SUCCESS;
	}

	midgard *mgd = mgd_handle();

	/* Remove midgard loghandler. This might be valid in web environment, 
	 * and might be not valid in cli. Midgard connection object might be 
	 * already unrefed here. */
	if(global_loghandler) {
		if (mgd && (mgd->_mgd && MIDGARD_IS_CONNECTION (mgd->_mgd))) {
		       	if (midgard_connection_get_loghandler (mgd->_mgd) == global_loghandler)
				g_log_remove_handler("midgard-core", global_loghandler);
		}
	}

	/* FIXME, at least MidgardTypeHolder should be passed here instead of NULL */
	global_loghandler =
		g_log_set_handler("midgard-core",
				G_LOG_LEVEL_MASK,
				php_midgard_log_errors, NULL);

	g_debug("MIDGARD REQUEST END");

	midgard_request_config *rcfg = mgd_rcfg();

	/* Disconnect all callbacks registered for connection */
	if(mgd && mgd->_mgd)
		php_midgard_connection_disconnect_callbacks(mgd->_mgd);

	if(mgd != NULL){ 
		mgd_internal_set_lang(mgd_handle(), 0);
	}
	
	/* Destroy rcfg->elements hash table */
	if (rcfg && mgd != NULL) {
		/* Glib will destroy hash keys and values.
		 * * Table initilized with midgard_hash_strings_hash */
		if(rcfg->elements != NULL)
			g_hash_table_destroy (rcfg->elements);
	}
	
	if (mgd_rcfg() == NULL && mgd_handle() != NULL) {
		mgd_close(mgd_handle());
		/* EEH: not safe! mgd_done(); */
	}

	/* Free all closures 
	 * We can not keep them persistant as all data changes per request. */
	php_midgard_gobject_closure_hash_free();

	g_debug("MIDGARD REQUEST END - SUCCESS");

	if(global_loghandler)
		g_log_remove_handler("midgard-core", global_loghandler);

	/* Enable it for debugging purpose */
	/*zend_module_entry *module;
	int rv = zend_hash_find(&module_registry, "midgard", strlen("midgard")+1, (void**)&module);
	
	if(rv == SUCCESS)       
		module->handle = 0; */

	return SUCCESS;
}

PHP_MINFO_FUNCTION(midgard)
{
    int i = 0;
    php_info_print_table_start();
    php_info_print_table_header(2, "Midgard Support", "enabled");
    php_info_print_table_row(2, "Midgard version", mgd_version());
    
    /* TODO: pretify output by arranging functions according object classes */
    while (midgard_module_entry.functions[i].fname) {
        php_info_print_table_row(2, "", midgard_module_entry.functions[i].fname);
        i++;
    }
    php_info_print_table_end();

    i = 0;
    php_info_print_table_start();
    php_info_print_table_header(2, "MgdSchema technology support", "enabled");
    php_info_print_table_row(2, "Midgard version", mgd_version());

    /* FIXME
    while (__midgard_php_type_functions[i].fname) {
        php_info_print_table_row(2, "", __midgard_php_type_functions[i].fname);
        i++;
    }
    */
    php_info_print_table_end();
        
    php_info_print_box_start(0);
    PUTS("<h3><a href=\"http://www.midgard-project.org/\">");
    PUTS("The Midgard Project</a></h3>\n");

    php_printf("This program makes use of the Midgard Content Management engine:<br />");
    php_printf("&copy; 1998-2001 The Midgard Project Ry - 2000-2001 Aurora Linux<br />\n");
    php_printf("&copy; 2002-2005 The Midgard Community<br />\n");
    php_info_print_box_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, config_init, (type_param))
#else
PHP_FUNCTION(mgd_config_init)
#endif
{
	gchar *conf;
	gint conf_length;
	MidgardConnection *mgd = mgd_handle_singleton_get();
	RETVAL_FALSE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&conf, &conf_length) == FAILURE)
		return;
	
	/* Check if connection is established */
	if(mgd != NULL) {
		
		php_error(E_NOTICE,
				"mgd_config_init is deprecated. Use midgard_connection class");
		RETURN_TRUE;
	}
	
	/* TODO , open connection here if really needed for some backward compatibility */
}

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, errno, (type param))
#else
PHP_FUNCTION(mgd_errno)
#endif
{
	RETVAL_FALSE;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") != SUCCESS) {
		WRONG_PARAM_COUNT;
	}
	RETVAL_TRUE;
	midgard *mgd = mgd_handle();
	if(!mgd) {
		
		php_error(E_WARNING, "midgard_connection not established");
		return;
	}

	RETURN_LONG(mgd->_mgd->errnum);
}

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, errstr, (type param))
#else
PHP_FUNCTION(mgd_errstr)
#endif
{
	RETVAL_FALSE;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") != SUCCESS) {
		WRONG_PARAM_COUNT;
	}

	RETVAL_TRUE;
	midgard *mgd = mgd_handle();	

	if(!mgd) {
		
		php_error(E_WARNING, "midgard_connection not established");
		return;
	}

	RETURN_STRING(mgd->_mgd->errstr, 1);
}

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, version, (type param))
#else
PHP_FUNCTION(mgd_version)
#endif
{
   RETURN_STRING((char*)mgd_version(), 1);
}

/* Fetch static globals. Unfortunately these need to be here since the
   module globals are declared static by the Zend macros
*/
midgard_request_config *mgd_rcfg()
{ 
   TSRMLS_FETCH(); 
   return MGDG(rcfg);
}

midgard_directory_config *mgd_dcfg()
{
   TSRMLS_FETCH();
   return MGDG(dcfg);
}

midgard *mgd_handle()
{
   TSRMLS_FETCH();
   return MGDG(mgd);
}

void mgd_handle_set(midgard *mgd)
{
	g_assert(mgd != NULL);
	
	TSRMLS_FETCH();
	MGDG(mgd) = mgd;
}

void mgd_handle_singleton_set(MidgardConnection *mgd)
{
	g_assert(mgd != NULL);
	
	TSRMLS_FETCH();
	MGDG(midgard_singleton) = mgd;
}

MidgardConnection *mgd_handle_singleton_get()
{
	TSRMLS_FETCH();
	return MGDG(midgard_singleton);
}

void mgd_set_errno(MgdErrorGeneric mgd_errno)
{
	MIDGARD_ERRNO_SET(mgd_handle(), mgd_errno);  
}

void mgd_reset_errno(void)
{
	MIDGARD_ERRNO_SET(mgd_handle(), MGD_ERR_OK);
}

int mgd_get_errno()
{
	return mgd_handle()->_mgd->errnum;
}

int is_table_multilang(const char *table) {
    if (!strcmp(table, "page") || !strcmp(table, "article") || !strcmp(table, "element") || !strcmp(table, "pageelement") || !strcmp(table, "snippet")) {
        return TRUE;
    } else {
        return FALSE;
    }
}

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, get_midgard, (type param))
#else
PHP_FUNCTION(mgd_get_midgard)
#endif
{
    int i;
    char *ah_prefix;
    zval *argv, *types, *confv;
    char **mm_argv;
    long mm_argc;
    midgard *mgd = mgd_handle();
    midgard_request_config *rcfg = mgd_rcfg();

  CHECK_MGD;
    if (rcfg == NULL) {
        RETURN_FALSE_BECAUSE(MGD_ERR_NOT_CONNECTED);
    }
   
  object_init(return_value);
 
    add_property_long(return_value, "host", rcfg->host);
  add_property_long(return_value, "style", rcfg->style);
  add_property_long(return_value, "page", rcfg->resource.id);
  add_property_long(return_value, "cookieauth", rcfg->auth.cookie);
  add_property_long(return_value, "auth", rcfg->auth.required);
  add_property_long(return_value, "author", rcfg->author);
  add_property_long(return_value, "user", mgd_user(mgd));
    add_property_long(return_value, "admin", mgd_isadmin(mgd));
  
#if HAVE_MIDGARD_QUOTA
    add_property_long(return_value, "quota", mgd_get_quota(mgd));
#endif /* HAVE_MIDGARD_QUOTA */
#if HAVE_MIDGARD_MULTILANG
    add_property_long(return_value, "lang", mgd_lang(mgd));
#endif /* HAVE_MIDGARD_MULTILANG */

    add_property_long(return_value, "root", mgd_isroot(mgd));
    add_property_long(return_value, "sitegroup", mgd_sitegroup(mgd));
    
    /* PP: Add midgard-config values ( prefix, multilang, quota) */
  MAKE_STD_ZVAL(confv);
  array_init(confv);
  add_assoc_string(confv, "prefix", MIDGARD_LIB_PREFIX, 1);
  add_assoc_long(confv, "multilang", HAVE_MIDGARD_MULTILANG);
  add_assoc_long(confv, "quota", HAVE_MIDGARD_QUOTA);
  add_property_zval(return_value, "config" , confv);

    mm_argv = (char**)rcfg->elts;
    mm_argc = rcfg->nelts;
    add_property_long(return_value, "argc", mm_argc);

    MAKE_STD_ZVAL(argv);
    array_init(argv);
    for (i = 0; i < mm_argc; i++) {
    if (mm_argv[i]) {
      add_index_string(argv, i, mm_argv[i], 1);
    }
    }

  add_property_zval(return_value, "argv", argv);
    add_property_string(return_value, "uri", rcfg->uri, 1);
    add_property_string(return_value, "ah_prefix", ((ah_prefix = mgd_get_ah_prefix(mgd)) == NULL)?"":ah_prefix, 1);
    add_property_stringl(return_value, "prefix", rcfg->uri, rcfg->prelen, 1);
    add_property_stringl(return_value, "self", rcfg->uri, rcfg->self_len, 1);

    MAKE_STD_ZVAL(types);
    array_init(types);
    for (i = 1; i < MIDGARD_OBJECT_COUNT; i++) {
        add_assoc_long(types, (char *)mgd_table_extname[i], i);
    }

    add_property_zval(return_value, "types", types);
    
}
#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, auth_midgard, (type param))
#else 
PHP_FUNCTION(mgd_auth_midgard)
#endif
{
	gchar *username, *password;
	guint namel, passl;
	midgard *mgd = mgd_handle();
	midgard_request_config *rcfg = mgd_rcfg();
	guint oldcookie;
	
	CHECK_MGD;

	/* Third parameter oldcookie is used only for backward compatibility
	 * It does nothing now */
	switch(ZEND_NUM_ARGS()){

		case 2:
			if (zend_parse_parameters(2 TSRMLS_CC, "ss",
						&username, &namel, 
						&password, &passl) == FAILURE) {
				
				WRONG_PARAM_COUNT;
				RETURN_FALSE;
			}
			break;
	
		case 3:
			if (zend_parse_parameters(3 TSRMLS_CC, "ssl",
						&username, &namel,
						&password, &passl,
						&oldcookie) == FAILURE) {
				
				WRONG_PARAM_COUNT;
				RETURN_FALSE;
			}
			break;

		default:
			WRONG_PARAM_COUNT;
			RETURN_FALSE;
	}
			
	zval **hash, **vkey; /* Used in _MIDGARD_UPDATE_LONG */
	
	int rv = mgd_auth(mgd, username, password, 1);	
	if (rcfg){
		_MIDGARD_UPDATE_LONG("auth", rcfg->auth.required);
	}
	
	_MIDGARD_UPDATE_LONG("user", mgd_user(mgd));
	_MIDGARD_UPDATE_LONG("admin", mgd_isadmin(mgd));
	_MIDGARD_UPDATE_LONG("root", mgd_isroot(mgd));
 
	switch (rv) {
		case MGD_AUTH_NOT_CONNECTED:
			RETURN_FALSE_BECAUSE(MGD_ERR_NOT_CONNECTED);
		
		case MGD_AUTH_INVALID_NAME:
			RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_NAME);

		case MGD_AUTH_SG_NOTFOUND:
			RETURN_FALSE_BECAUSE(MGD_ERR_SG_NOTFOUND);
		
		case MGD_AUTH_DUPLICATE:
			RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);
		
		case MGD_AUTH_NOTFOUND:
			RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
		
		case MGD_AUTH_REAUTH:
		case MGD_AUTH_INVALID_PWD:
			RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
		
		default:
			if (rv >= 0) {
				RETVAL_LONG(rv);
			} else {
				RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
			}
	}
}

MGD_FUNCTION(ret_type, connect, (type param))
{
zval *hostname, *database, *username, *password;

  if (MGDG(mgd) != NULL) RETURN_TRUE;

    if (ZEND_NUM_ARGS() != 4) { WRONG_PARAM_COUNT; }

   if (zend_parse_parameters(4 TSRMLS_CC, "zzzz", &hostname, &database,
            &username, &password) == FAILURE) {
      WRONG_PARAM_COUNT;
   }
    convert_to_string_ex(&hostname);
    convert_to_string_ex(&database);
    convert_to_string_ex(&username);
    convert_to_string_ex(&password);

   /*[eeh] this can safely be called multiple times. After the first
    * it'll return without doing anything
    */
   mgd_init();

   MGDG(mgd) = mgd_connect(Z_STRVAL_P(hostname), Z_STRVAL_P(database), Z_STRVAL_P(username), Z_STRVAL_P(password));
}
#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, unsetuid, (type param))
#else
PHP_FUNCTION(mgd_unsetuid)
#endif
{
    midgard_request_config *rcfg = mgd_rcfg();
    midgard *mgd = mgd_handle();
    midgard_res *res;
    long int rsg = 0;
    int mua;
    zval **hash, **vkey; /* Used in _MIDGARD_UPDATE_LONG */
  
    if (ZEND_NUM_ARGS() != 0) { WRONG_PARAM_COUNT; }

    CHECK_MGD;

    if (rcfg) {
        
        res = mgd_ungrouped_select(mgd, "sitegroup","host", "host.id=$d", NULL, rcfg->host);

        if (res) {
            mgd_fetch(res);
            rsg = atoi(mgd_colvalue(res,0));
            mgd_release(res);
        }
    }

    mua = mgd_auth_unsetuid(mgd, rsg);

    if (MGDG(rcfg)){
        _MIDGARD_UPDATE_LONG("auth", MGDG(rcfg)->auth.required);
    }
     
    _MIDGARD_UPDATE_LONG("user", mgd_user(mgd_handle()));
    _MIDGARD_UPDATE_LONG("admin", mgd_isadmin(mgd_handle()));
    _MIDGARD_UPDATE_LONG("root", mgd_isroot(mgd_handle()));
    
    RETURN_LONG(mua);
}

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, issetuid, (type param))
#else
PHP_FUNCTION(mgd_issetuid)
#endif
{
    if (ZEND_NUM_ARGS() != 0) { WRONG_PARAM_COUNT; }

    CHECK_MGD;

   RETURN_LONG(mgd_auth_is_setuid(mgd_handle()));
}

MGD_FUNCTION(ret_type, preparser_active, (type param))
{
    if (ZEND_NUM_ARGS() != 0) { WRONG_PARAM_COUNT; }

    CHECK_MGD;

   RETURN_LONG(mgd_rcfg()->preparser_active);
}

int midgard_user_call_func(midgard *mgd, int id, int level, void *xparam)
{
    zval *args[3];
    zval ** xp = (zval **)xparam;
    zval *return_value;
    int retval;
  TSRMLS_FETCH();

    if(!PZVAL_IS_REF(xp[0])) {
        /* DG: Do we force the user to pass it by reference ? */
        php_error(E_WARNING,"You must pass the fourth parameter by reference.");
        return 0;
    }

    MAKE_STD_ZVAL(return_value);    ZVAL_NULL(return_value);
    MAKE_STD_ZVAL(args[0]);        ZVAL_LONG(args[0], id);
    MAKE_STD_ZVAL(args[1]);        ZVAL_LONG(args[1], level);
    args[2] = xp[0];        // DG: is this needed ? ->zval_copy_ctor(args[2]);

    if(call_user_function(CG(function_table), NULL,
                  xp[1], return_value, 3,
                  args TSRMLS_CC) != SUCCESS) {
        php_error(E_WARNING,"Unable to call %s() - function does not exist",
                  (xp[1])->value.str.val);
        zval_dtor(return_value);
        zval_dtor(args[0]); zval_dtor(args[1]);
        return 0;
    }
    if(return_value->type == IS_NULL)
        retval = 1;
    else {
        convert_to_long_ex(&return_value);
        retval = return_value->value.lval;
    }
    zval_dtor(return_value);
    zval_dtor(args[0]); zval_dtor(args[1]);
    return retval;
}

#if HAVE_MIDGARD_MULTILANG

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, set_lang, (type param))
#else
PHP_FUNCTION(mgd_set_lang)
#endif
{
    zval *lang;
    zval **hash, **vkey; /* Used in _MIDGARD_UPDATE_LONG */

    switch (ZEND_NUM_ARGS()) {
    case 1:
        if (zend_parse_parameters(1 TSRMLS_CC, "z", &lang) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        break;
    default:
        WRONG_PARAM_COUNT;
    }
    convert_to_long_ex(&lang);
    mgd_internal_set_lang(mgd_handle(), Z_LVAL_P(lang));
    
    _MIDGARD_UPDATE_LONG("lang", Z_LVAL_P(lang));

    RETURN_TRUE;
}

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, set_default_lang, (type_param))
#else
PHP_FUNCTION(mgd_set_default_lang)
#endif
{
	CHECK_MGD;
	RETVAL_FALSE;
	guint lang;

	if (zend_parse_parameters(1 TSRMLS_CC, "l", &lang)
			== FAILURE) {
		WRONG_PARAM_COUNT;
	}

	mgd_set_default_lang(mgd_handle(), lang);
	RETVAL_TRUE;
}

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, get_default_lang, (type_param))
#else
PHP_FUNCTION(mgd_get_default_lang)
#endif
{
	CHECK_MGD;
	RETVAL_FALSE;

	if(ZEND_NUM_ARGS() > 0)
		WRONG_PARAM_COUNT;

	RETVAL_TRUE;
	RETURN_LONG(mgd_get_default_lang(mgd_handle()));
}

MGD_FUNCTION(ret_type, set_lang_by_code, (type param))
{
    zval *lang;
    midgard_res *res;
    int id;

    switch (ZEND_NUM_ARGS()) {
    case 1:
        if (zend_parse_parameters(1 TSRMLS_CC, "z", &lang) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        break;
    default:
        WRONG_PARAM_COUNT;
    }
    convert_to_string_ex(&lang);
    res = mgd_ungrouped_select(mgd_handle(), "id", "language", "code=$q", NULL, Z_STRVAL_P(lang));

    if (res && mgd_fetch(res)) {
      id = atol(mgd_colvalue(res, 0));
      mgd_release(res);
    } else {
      RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
    }
    mgd_internal_set_lang(mgd_handle(), id);
    RETURN_TRUE;
}

MGD_FUNCTION(ret_type, set_parameters_defaultlang, (type param))
{
    zval *lang;

    switch (ZEND_NUM_ARGS()) {
    case 1:
        if (zend_parse_parameters(1 TSRMLS_CC, "z", &lang) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        break;
    default:
        WRONG_PARAM_COUNT;
    }
    convert_to_long_ex(&lang);
    mgd_internal_set_parameters_defaultlang(mgd_handle(), Z_LVAL_P(lang));
    RETURN_TRUE;
}

MGD_FUNCTION(ret_type, set_attachments_defaultlang, (type param))
{
    zval *lang;

    switch (ZEND_NUM_ARGS()) {
    case 1:
        if (zend_parse_parameters(1 TSRMLS_CC, "z", &lang) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        break;
    default:
        WRONG_PARAM_COUNT;
    }
    convert_to_long_ex(&lang);
    mgd_internal_set_attachments_defaultlang(mgd_handle(), Z_LVAL_P(lang));
    RETURN_TRUE;
}

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, get_lang, (type param))
#else
PHP_FUNCTION(mgd_get_lang)
#endif
{
    RETURN_LONG(mgd_lang(mgd_handle()));
}


MGD_FUNCTION(ret_type, get_parameters_defaultlang, (type param))
{
    RETURN_LONG(mgd_parameters_defaultlang(mgd_handle()));
}


MGD_FUNCTION(ret_type, get_attachments_defaultlang, (type param))
{
    RETURN_LONG(mgd_attachments_defaultlang(mgd_handle()));
}

#endif /* HAVE_MIDGARD_MULTILANG */

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, debug_start, (type param))
#else
PHP_FUNCTION(mgd_debug_start)
#endif
{	
	MidgardConnection *mgd = mgd_handle_singleton_get();

	if (ZEND_NUM_ARGS() != 0) { WRONG_PARAM_COUNT; }
  
	if(!mgd)
		php_error(E_ERROR, 
				"Can not start debug level. Midgard is not initialized");

	midgard_connection_set_loglevel(mgd, "debug", php_midgard_log_errors);
}

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, debug_stop, (type param))
#else
PHP_FUNCTION(mgd_debug_stop)
#endif

{
	midgard *mgd = mgd_handle();
	
	if (ZEND_NUM_ARGS() != 0) { WRONG_PARAM_COUNT; }

	if(!mgd)
		php_error(E_ERROR, 
				"Can not stop debug level. Midgard is not initialized");	

	midgard_connection_set_loglevel(mgd->_mgd, "warn", php_midgard_log_errors);
}

/* Create $_MIDGARD['schema']['types'] array.
 * All types defined in schema are included here */
static void _make_schema_types(zval *schema_array)
{
    zval *types;
    
    MAKE_STD_ZVAL(types);
    array_init(types);
           
    guint n_types, i;
    const gchar *typename; 
    GType *all_types = g_type_children(MIDGARD_TYPE_OBJECT, &n_types);
    
    for (i = 0; i < n_types; i++) {
	    typename = g_type_name(all_types[i]);
	    add_assoc_string(types, (gchar *)typename, "", 1);         
    }
    
    g_free(all_types);

    add_assoc_zval(schema_array, "types", types);
}

void _make_midgard_global()
{
	int i;
	char *ah_prefix;
	zval *argv, *confv, *mgd_php_globals, *schema_array;
	char **mm_argv;
	long mm_argc;
	midgard *mgd = mgd_handle();
	midgard_request_config *rcfg = mgd_rcfg();

	TSRMLS_FETCH();
	
	/* PP: We need to init array as it will be added to global $_MIDGARD */
	MAKE_STD_ZVAL(mgd_php_globals);
	array_init(mgd_php_globals);
	
	if(rcfg) {
		add_assoc_long(mgd_php_globals, "host", rcfg->host);
		add_assoc_long(mgd_php_globals, "style", rcfg->style);
		add_assoc_long(mgd_php_globals, "page", rcfg->resource.id);
		add_assoc_long(mgd_php_globals, "auth", rcfg->auth.required);
		add_assoc_long(mgd_php_globals, "author", rcfg->author); 
		add_assoc_string(mgd_php_globals, "uri", rcfg->uri, 1);
		add_assoc_stringl(mgd_php_globals, "prefix", rcfg->uri, rcfg->prelen, 1);
		add_assoc_stringl(mgd_php_globals, "self", rcfg->uri, rcfg->self_len, 1);
	}

	add_assoc_long(mgd_php_globals, "user", mgd_user(mgd));
	add_assoc_long(mgd_php_globals, "admin", mgd_isadmin(mgd));
	add_assoc_long(mgd_php_globals, "lang", mgd_lang(mgd));
	add_assoc_long(mgd_php_globals, "root", mgd_isroot(mgd));
	add_assoc_long(mgd_php_globals, "sitegroup", mgd_sitegroup(mgd));

	/* PP: Add midgard-config values ( prefix, multilang, quota) */
	MAKE_STD_ZVAL(confv);
	array_init(confv);
	add_assoc_string(confv, "prefix", MIDGARD_LIB_PREFIX, 1);
	add_assoc_zval(mgd_php_globals, "config", confv);
  
	MAKE_STD_ZVAL(argv);
	array_init(argv);
	
	if (rcfg) {
		
		mm_argv = (char**)rcfg->elts;
		mm_argc = rcfg->nelts;
		
		add_assoc_long(mgd_php_globals, "argc", mm_argc);
		
		for (i = 0; i < mm_argc; i++)
			add_index_string(argv, i, mm_argv[i], 1);
	}

	add_assoc_zval(mgd_php_globals, "argv", argv);
	add_assoc_string(mgd_php_globals, "ah_prefix", ((ah_prefix = mgd_get_ah_prefix(mgd)) == NULL)?"":ah_prefix, 1);
	
	/* Create $_MIDGARD['schema']['types'] */
	MAKE_STD_ZVAL(schema_array);
	array_init(schema_array);
	_make_schema_types(schema_array);
	add_assoc_zval(mgd_php_globals, "schema", schema_array);
	
	zend_hash_add(&EG(symbol_table), 
			MIDGARD_GLOBAL_MIDGARD, sizeof(MIDGARD_GLOBAL_MIDGARD), 
			&mgd_php_globals, sizeof(zval *), NULL);  
}

MGD_FUNCTION(ret_type, set_style, (type param))
{
  zval *style;
  midgard_request_config *rcfg = mgd_rcfg(); 
    
  CHECK_MGD;
  
  switch (ZEND_NUM_ARGS()) {
    case 0:
      WRONG_PARAM_COUNT;
      break;

    case 1:
      if (zend_parse_parameters(1 TSRMLS_CC, "z", &style) != SUCCESS) { WRONG_PARAM_COUNT; }
      convert_to_long_ex(&style);
            
      midgard_style_get_elements(MGDG(mgd), Z_LVAL_P(style), 
      midgard_pc_set_element, rcfg->elements);
     
      break;
    
    default:
      WRONG_PARAM_COUNT;
      break;
  }
      
}

MGD_FUNCTION(ret_type, set_styledir, (type param))
{
    gchar *path;
    int pathl;
    midgard_request_config *rcfg = mgd_rcfg();
    midgard *mgd = mgd_handle();
    
    CHECK_MGD;

    if (zend_parse_parameters(1  TSRMLS_CC, "s",
                &path, &pathl ) == FAILURE ){
        RETVAL_FALSE;
        WRONG_PARAM_COUNT;
    }
     
    if (midgard_style_get_elements_from_dir(mgd, path,
                midgard_pc_set_element, rcfg->elements)){
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
    
}

MGD_FUNCTION(ret_type, register_style, (type param))
{
	php_error(E_WARNING, "Obsolete call mgd_register_style");
       
}

MGD_FUNCTION(ret_type, register_style_from_dir, (type param))
{
      php_error(E_WARNING, "Obsolete call mgd_style_from_dir");
}

/* Piotras: zend_parse_parameters_ex with first 'quiet' parameter
 * is used to return false without any warning being set on PHP level.
 * This is midcom related. 
 * I define quiet parameter as 2 , in any other case it's not quiet.
 * I have no idea if it's hack like, but it works as expected */
#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(ret_type, is_guid, (type param))
#else
PHP_FUNCTION(mgd_is_guid)
#endif
{
	gchar *guid;
	guint guidl;
	CHECK_MGD;

	if(ZEND_NUM_ARGS() != 1) {
		WRONG_PARAM_COUNT;
		RETURN_FALSE;
	}
	
	if (zend_parse_parameters_ex(2, 1  TSRMLS_CC, "s",
				&guid, &guidl) == FAILURE ){
		RETURN_FALSE;
	}
	
	if (midgard_is_guid(guid))
		RETURN_TRUE;

	RETURN_FALSE;
}

MGD_FUNCTION(ret_type, cache_invalidate, (type param))
{
	CHECK_MGD;
	
	if(ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
		RETURN_FALSE;
	}
	
	mgd_cache_touch(mgd_handle(), 0);
	RETURN_TRUE;
}

MGD_FUNCTION(ret_type, set_errno, (type param))
{
	gint errid;
	CHECK_MGD;
	RETVAL_FALSE;
	
	if (zend_parse_parameters(1 TSRMLS_CC, "l", &errid)  == FAILURE) {
		WRONG_PARAM_COUNT;
	}
      
	mgd_set_errno(errid);
	RETURN_TRUE;
}

MGD_FUNCTION(ret_type, reset_errno, (type param))
{
	CHECK_MGD;
	RETVAL_FALSE;

	if(ZEND_NUM_ARGS() != 0) 
		WRONG_PARAM_COUNT;

	mgd_reset_errno();
	RETURN_TRUE;
}

zend_class_entry *midgard_php_register_internal_class(const gchar *class_name, GType class_type, zend_class_entry ce)
{
	g_assert(class_name != NULL);

	if(class_type == 0){
		php_error(E_ERROR, 
				"'%s' class  is not registered in GType system!", 
				class_name);
		return NULL;
	}

	/* Object's class can be uninitialized , so let's do it */
	g_object_new(class_type, NULL);

	if(G_TYPE_IS_DERIVED(class_type)){
		
		GObjectClass *object_class = g_type_class_peek(class_type);
		
		if(object_class){
			GObjectClass *parent_class = 
				g_type_class_peek_parent(object_class);
			
			if(parent_class) {
				TSRMLS_FETCH();
				GType parent_type = G_TYPE_FROM_CLASS(parent_class);
				gchar *parent_name = (gchar *) g_type_name(parent_type);
				guint classname_length = strlen(parent_name);
				gchar *_classname = g_ascii_strdown(
						parent_name, classname_length);
#if  (PHP_MAJOR_VERSION < 5)
				zend_class_entry *pce, *registered_class = NULL;
#else
				zend_class_entry **pce, *registered_class = NULL;
#endif
				if (zend_hash_find(CG(class_table),
							_classname,
							classname_length+1,
							(void **) &pce) == SUCCESS) {
#if  (PHP_MAJOR_VERSION < 5)	
					registered_class = 
						zend_register_internal_class_ex(&ce , pce, 
								parent_name TSRMLS_CC);
#else
					registered_class =
						zend_register_internal_class_ex(&ce , *pce,
								parent_name TSRMLS_CC);
#endif
				}
				g_free(_classname);
				return registered_class;
			}
		}
	}
	g_warning("Return NULL %s ", class_name);
	return NULL;
}

#endif  /* HAVE_MIDGARD */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
