/* $Id: attachment.c 27410 2014-09-01 07:39:28Z piotras $
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

#include "php_globals.h"
#include <main/SAPI.h>
#include "ext/standard/file.h"

#include "mgd_oop.h"
#include "mgd_access.h"

#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

//#ifdef PHP_MIDGARD_LEGACY_API

enum { MGD_BLOB_CALL_FUNC, MGD_BLOB_CALL_SELF, MGD_BLOB_CALL_OTHER };

static long mgd_get_blob_id(zval *self, zval **name, int *calltype)
{
	zval **table, **id;
	TSRMLS_FETCH();

#if HAVE_MIDGARD_MULTILANG
	zval **zv_lang;
        int lang = mgd_get_default_lang(mgd_handle());
	int myId;
#endif /* HAVE_MIDGARD_MULTILANG */

   if (self == NULL) {
      if (calltype) *calltype = MGD_BLOB_CALL_FUNC;
      if (name == NULL) return MGD_ERR_INVALID_NAME;
      convert_to_long_ex(name);
      return (*name)->value.lval;
   }

   if (!MGD_PROPFIND(self, "__table__", table)) {
      return MGD_ERR_NOT_OBJECT;
   }

   if (!MGD_PROPFIND(self, "id", id)) {
      return MGD_ERR_NOT_OBJECT;
   }
   else
   convert_to_long_ex(id);

   if ((*table)->type != IS_STRING) {
      return MGD_ERR_NOT_OBJECT;
   }

   if (is_table_multilang((*table)->value.str.val)) {
       if (!MGD_PROPFIND(self, "lang", zv_lang)) {
           return MGD_ERR_NOT_OBJECT;
       }
       convert_to_long_ex(zv_lang);
       lang = (*zv_lang)->value.lval;
   }


   if (strcmp((*table)->value.str.val, "blobs") == 0) {
      if (calltype) *calltype = MGD_BLOB_CALL_SELF;
      convert_to_long_ex(id);
      return (*id)->value.lval;
   }

   if (calltype) *calltype = MGD_BLOB_CALL_OTHER;
   if (name == NULL) return MGD_ERR_INVALID_NAME;
   convert_to_string_ex(name);

   myId = mgd_exists_id(mgd_handle(), "blobs",
			"ptable=$q AND pid=$d AND name=$q AND lang=$d",
			(*table)->value.str.val, (*id)->value.lval,
			(*name)->value.str.val, lang);
   if (myId) {
     return myId;
   } else {
     return mgd_exists_id(mgd_handle(), "blobs", "ptable=$q AND pid=$d AND name=$q AND lang=$d",
			  (*table)->value.str.val, (*id)->value.lval,
			  (*name)->value.str.val), mgd_get_default_lang(mgd_handle());
   }

}

MGD_FUNCTION(int, oop_attachment_create, (string name, string title, string mimetype, [int score]))
{
	zval *self;
	zval **zv_table, **zv_id;
	zval **zv_name, **zv_title, **zv_mimetype, **zv_score, **sitegroup_property;
	int id = 0, current_sitegroup;
	int table;
	int score;
	char location[MIDGARD_REPLIGARD_TAG_LENGTH + 5];
	char *path;
	struct stat statbuf;
	midgard_pool *pool;
	FILE *fp;
	const char *blobdir;
	zval **zv_lang;
    	int lang = mgd_get_default_lang(mgd_handle());
	zend_class_entry *ce;
    
	CHECK_MGD;

	blobdir = mgd_get_blobdir(mgd_handle());
	if (!blobdir || (*blobdir != '/')
	    || (stat(blobdir, &statbuf) != 0)
	    || !S_ISDIR(statbuf.st_mode)) {
         php_log_err("Midgard: Invalid blobdir or blobdir not set" TSRMLS_CC);
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	}

	self = getThis();
	if (self == NULL) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
	}

	if (!MGD_PROPFIND(self, "__table__", zv_table)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
   }
	if (!MGD_PROPFIND(self, "id", zv_id)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
   }

	convert_to_string_ex(zv_table);
	convert_to_long_ex(zv_id);

	if (is_table_multilang((*zv_table)->value.str.val)) {
        if (!MGD_PROPFIND(self, "lang", zv_lang)) {
            RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
        }
        convert_to_long_ex(zv_lang);
        lang = mgd_attachments_defaultlang(mgd_handle()) ? mgd_get_default_lang(mgd_handle())  :(*zv_lang)->value.lval;
	}

    switch (ZEND_NUM_ARGS()) {
		case 3:
			if (zend_parse_parameters
			    (3 TSRMLS_CC, "zzz", &zv_name, &zv_title, &zv_mimetype) != SUCCESS) {
				WRONG_PARAM_COUNT;
			}
			score = 0;
			break;
		case 4:
			if (zend_parse_parameters
			    (4 TSRMLS_CC, "zzzz", &zv_name, &zv_title, &zv_mimetype, &zv_score)
			    != SUCCESS) {
				WRONG_PARAM_COUNT;
			}
			convert_to_long_ex(zv_score);
			score = (*zv_score)->value.lval;
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	convert_to_string_ex(zv_name);
	convert_to_string_ex(zv_title);
	convert_to_string_ex(zv_mimetype);


	if (mgd_exists_id
	    (mgd_handle(), "blobs", "ptable=$q AND pid=$d AND name=$q AND lang=$d",
	     (*zv_table)->value.str.val, (*zv_id)->value.lval,
	     (*zv_name)->value.str.
	     val, lang
	     ))  {
        RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);
    }

    id = (*zv_id)->value.lval;    

    /* Switch to anonymous mode for MgdSchema objects */
    MGD_PHP_PCLASS_NAME;
    if (g_hash_table_lookup(mgd_handle()->schema->types, ce->name) == NULL) {

        table = mgd_lookup_table_id((*zv_table)->value.str.val);
       
	    if (!isglobalowner(table, id)) {
		    RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
        }
    }

    do {
		strcpy(location + 4,
		       mgd_create_guid(mgd_handle(),
				       (*zv_table)->value.str.val, id));
	} while (strstr(location + 4, ".."));

	/* some basic hashing to aid in dirsize balancing. This probably
	   needs refinement since the md5 checksum we use to generate the names
	   returns all readable chars which don't distribute well with the
	   simple method below
	   AB: the algorithm has been changed to the selection of two first
	   characters from GUID. It adds more uniform distribution because older one
	   was weak: it counted only 1st character from GUID and since it can be only\
	   from [0-9a-f] set, only '3' and '6' top-level directories were selected
	   because 0-9 are in 0x30-0x39 and a-f are in 0x61-0x66.
	   Note that Repligard already uses this algorithm from its birth.
	 */
	location[1] = location[3] = '/';
	location[0] = toupper(location[4]);
	location[2] = toupper(location[5]);

	if (!(pool = mgd_alloc_pool())) {
		php_log_err("Midgard: out of memory" TSRMLS_CC);
		RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
	}

	path = mgd_format(mgd_handle(), pool, "$s/$s", blobdir, location);

	fp = fopen(path, "w");
	if (fp == NULL) {
		php_log_err("Midgard: Can't create blob" TSRMLS_CC);
		RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
	}
	else {
		fclose(fp);

		_MGD_SITEGROUP_FORCE();
		MgdObject *gobj = midgard_object_new(mgd_handle(), "midgard_attachment", NULL);
		
		if(!midgard_quota_create(gobj)){
			g_object_unref(gobj);
			mgd_free_pool(pool);
			RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
		}
		g_object_unref(gobj);
			
		php_midgard_create(return_value, NULL, "blobs",
				   "ptable,pid,name,title,location,mimetype,author,created,score,lang",
				   "$q,$d,$q,$q,$q,$q,$d,Now(),$d,$d",
				   (*zv_table)->value.str.val, id,
				   (*zv_name)->value.str.val,
				   (*zv_title)->value.str.val, location,
				   (*zv_mimetype)->value.str.val,
				   mgd_user(mgd_handle()), score, lang);

		gobj = midgard_object_new(mgd_handle(), 
				"midgard_attachment", NULL);
		if(midgard_object_get_by_id(gobj, 
					(guint) return_value->value.lval)) {
			midgard_quota_update(gobj);
			g_object_unref(gobj);
		} else {
			g_warning("Failed to update attachment quota. ID:%d", (int)return_value->value.lval);
		}
			
		PHP_CREATE_REPLIGARD("blobs", return_value->value.lval);
		_MGD_SITEGROUP_FORCE_REVERT();
	}

	mgd_free_pool(pool);
}

MGD_FUNCTION(mixed, oop_attachment_list, ([string sort]))
{
	zval *self, **zv_table, **zv_id, **zv_order;
	char *order = NULL;
	zval **zv_lang;
	zend_class_entry *ce;


	CHECK_MGD;

	self = getThis();
	if (self == NULL) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
	}

    MGD_PHP_PCLASS_NAME;
    
	if (!MGD_PROPFIND(self, "__table__", zv_table)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
   }
	if (!MGD_PROPFIND(self, "id", zv_id)) {
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
   }

	convert_to_string_ex(zv_table);
	convert_to_long_ex(zv_id);

#if HAVE_MIDGARD_MULTILANG
	if (is_table_multilang((*zv_table)->value.str.val)) {
        if (!MGD_PROPFIND(self, "lang", zv_lang)) {
            RETURN_FALSE_BECAUSE(MGD_ERR_NOT_OBJECT);
        }
        convert_to_long_ex(zv_lang);
	}
#endif /* HAVE_MIDGARD_MULTILANG */


	if (ZEND_NUM_ARGS() == 1) {
		if (zend_parse_parameters(1 TSRMLS_CC, "z", &zv_order) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		else {
			convert_to_string_ex(zv_order);
			order = (*zv_order)->value.str.val;
		}
	}
	php_midgard_select(&MidgardAttachment, return_value,
			   "id,name,title,mimetype,score,author,created" SITEGROUP_SELECT,
			   "blobs", "ptable=$q AND pid=$d", order,
			   (*zv_table)->value.str.val, (*zv_id)->value.lval);
}

MGD_FUNCTION(int, open_attachment, ([int id, [string mode]]))
{
	int pid;
	int ptable;
#if ! (PHP_MAJOR_VERSION > 4 || (PHP_MAJOR_VERSION == 4 && PHP_MINOR_VERSION >= 3))
	FILE *fp;
#endif
	const char *location;
	char *path;
	midgard_pool *pool;
	midgard_res *res, *qres;
	const char *blobdir;
	zval **zv_id, **zv_mode;
    zval *self;
    long aid;
    zend_class_entry *ce;

#if HAVE_MIDGARD_QUOTA
   int spacelim = 0, length = 0;
#endif

#if PHP_MAJOR_VERSION > 4 || (PHP_MAJOR_VERSION == 4 && PHP_MINOR_VERSION >= 3)
   php_stream *stream;
#endif

	CHECK_MGD;
	RETVAL_FALSE;

	blobdir = mgd_get_blobdir(mgd_handle());
	if (!blobdir || *blobdir != '/') {
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }

	switch (ZEND_NUM_ARGS()) {
        
        case 0:
            zv_id = NULL;
            zv_mode = NULL;
            break;
		            
        case 1:
            if (zend_parse_parameters(1 TSRMLS_CC, "z", &zv_id) != SUCCESS) {
                WRONG_PARAM_COUNT;
            }
			zv_mode = NULL;
			break;
            
		case 2:
			if (zend_parse_parameters(2 TSRMLS_CC, "z", &zv_id, &zv_mode) !=
			    SUCCESS) {
				WRONG_PARAM_COUNT;
			}
			break;
            
		default:
			WRONG_PARAM_COUNT;
	}

   
   aid = mgd_get_blob_id((self = getThis()), zv_id, NULL);
   if (aid == 0) { RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS); }
   else if (aid == MGD_ERR_INVALID_NAME) { WRONG_PARAM_COUNT; }
   else if (aid < 0) { RETURN_FALSE_BECAUSE(aid); }

   if(zv_mode)
       convert_to_string_ex(zv_mode);

	res = mgd_sitegroup_record(mgd_handle(), "ptable,pid,location",
				   "blobs", aid);

	if (!res || !mgd_fetch(res)) {
		if (res) {
			mgd_release(res);
      }

		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}

	ptable = mgd_lookup_table_id(mgd_colvalue(res, 0));
	pid = mgd_sql2id(res, 1);
	location = mgd_colvalue(res, 2);

	/* Check for relative location - not allowed */
	if (strstr(location, "..")) {
		mgd_release(res);
		php_log_err("Midgard: blobdir location relative" TSRMLS_CC);

		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	}

    /* Switch to anonymous mode for MgdSchema objects */
  
    if (getThis()) { 
        MGD_PHP_PCLASS_NAME;
    
    if (g_hash_table_lookup(mgd_handle()->schema->types, ce->name) == NULL) {

        /* Check access permissions: Write-Access requires ownership, */
	    /* Read access is allowed by default */
    	if (zv_mode)
	    	if (strcmp((*zv_mode)->value.str.val, "r") != 0
	        	   && !isglobalowner(ptable, pid)) {
			    mgd_release(res);
    			php_log_err("Midgard: Attachment Write Access only for owner" TSRMLS_CC);

	    		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
		    } 
    }
    }

	pool = mgd_alloc_pool();
#if HAVE_MIDGARD_QUOTA
	if (mgd_get_quota(mgd_handle())) {
	  qres = mgd_query(mgd_handle(), "SELECT space FROM quota WHERE sg = $d and tablename = 'blobsdata'", mgd_sitegroup(mgd_handle()));
	  if (qres) {
	    if (mgd_fetch(qres)) {
	      spacelim = mgd_sql2id(qres, 0);
	    }
	    mgd_release(qres);
	  }
	  if (spacelim) {
	    length = mgd_get_quota_space(mgd_handle(), "blobsdata", NULL, 0);
	    if (length >= spacelim) {
	      g_log("midgard-lib", G_LOG_LEVEL_DEBUG,
		    "Quota exceeded: length = %d", length);
	      mgd_free_pool(pool);
	      RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
	    }
	  }
	  spacelim = 0;
	  qres = mgd_query(mgd_handle(), "SELECT space FROM quota WHERE sg = $d and tablename = 'wholesg'", mgd_sitegroup(mgd_handle()));
	  if (qres) {
	    if (mgd_fetch(qres)) {
	      spacelim = mgd_sql2id(qres, 0);
	    }
	    mgd_release(qres);
	  }
	  if (spacelim) {
	    length = mgd_get_quota_space(mgd_handle(), "wholesg", NULL, 0);
	    if (length >= spacelim) {
	      g_log("midgard-lib", G_LOG_LEVEL_DEBUG,
		    "Quota exceeded: length = %d", length);
	      mgd_free_pool(pool);
	      RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
	    }
	  }
	}
	mgd_touch_recorded_quota(mgd_handle(), "blobsdata", mgd_sitegroup(mgd_handle()));
	mgd_touch_recorded_quota(mgd_handle(), "wholesg", mgd_sitegroup(mgd_handle()));
#endif
	path = mgd_format(mgd_handle(), pool, "$s/$s", blobdir, location);

#if PHP_MAJOR_VERSION > 4 || (PHP_MAJOR_VERSION == 4 && PHP_MINOR_VERSION >= 3)
   stream = php_stream_open_wrapper_ex(path, zv_mode ? (*zv_mode)->value.str.val : "w", IGNORE_PATH | IGNORE_URL | STREAM_DISABLE_OPEN_BASEDIR, NULL, NULL);
   if (stream == NULL) {
       RETVAL_FALSE_BECAUSE(MGD_ERR_INTERNAL);
   } else {
       
       /* Update repligard's entry  without any knowledge if PHP's fputs or fwrite
        * functions didn't return FALSE. Not ellegant solution to update guid here
        * but resolve problem of updating every Midgard application which needs to update
        * guid.
        */ 
       if (zv_mode) {
           if (strcmp((*zv_mode)->value.str.val, "w") == 0) {
               PHP_UPDATE_REPLIGARD("blobs", aid);
           }
       } else {
           PHP_UPDATE_REPLIGARD("blobs", aid);
       }
                  
       php_stream_to_zval(stream, return_value);
   }

#else
	fp = fopen(path, zv_mode ? (*zv_mode)->value.str.val : "w");

	if (!fp) {
        RETVAL_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    } else {
        
        if (mgd_update
                (mgd_handle(), "blobs", aid,
                 "author=$d,created=Now()", mgd_user(mgd_handle()))) {
            
            ZEND_REGISTER_RESOURCE(return_value, fp, php_file_le_fopen());

            /* Update repligard's entry  without any knowledge if PHP's fputs or fwrite
             * functions didn't return FALSE. Not ellegant solution to update guid here
             * but resolve problem of updating every Midgard application which needs to update
             * guid.
             */ 
            if (strcmp((zv_mode)->value.str.val, "w") == 0) {
                PHP_UPDATE_REPLIGARD("blobs", aid);
            }

        } else {
            fclose(fp);
			fp = NULL;
        }
    }

	if (!fp) {
		RETVAL_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
		php_log_err("Midgard: failed to open blob" TSRMLS_CC);
	}
#endif
	mgd_release(res);
	mgd_free_pool(pool);

}

MGD_FUNCTION(mixed, get_attachment, (int id))
{
	zval **id;
   zval *self;
   long aid;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
		case 1:
			if (zend_parse_parameters(1 TSRMLS_CC, "z", &id) != SUCCESS) WRONG_PARAM_COUNT;
         break;

		default:
			WRONG_PARAM_COUNT;
	}

   aid = mgd_get_blob_id((self = getThis()), id, NULL);
   if (aid == 0) { RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS); }
   else if (aid == MGD_ERR_INVALID_NAME) { WRONG_PARAM_COUNT; }
   else if (aid < 0) { RETURN_FALSE_BECAUSE(aid); }

   php_midgard_get_object(return_value, MIDGARD_OBJECT_BLOBS, aid);
}

MGD_FUNCTION(bool, serve_attachment, (int id))
{
	midgard_res *res;
	const char *location, *mimetype;
	midgard_pool *pool;
	char *path;
	FILE *fp;
	int b;
	char buf[1024];
	const char *blobdir;
   int id;
   zval *self, **zv_id;
   char *content_type;

	CHECK_MGD;
	RETVAL_FALSE;

	switch (ZEND_NUM_ARGS()) {
		case 0:
         zv_id = NULL;
         break;

		case 1:
			if (zend_parse_parameters(1 TSRMLS_CC, "z", &zv_id) != SUCCESS) WRONG_PARAM_COUNT;
         break;

		default:
			WRONG_PARAM_COUNT;
	}

   id = mgd_get_blob_id((self = getThis()), zv_id, NULL);
   if (id == 0) { RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS); }
   else if (id == MGD_ERR_INVALID_NAME) { WRONG_PARAM_COUNT; }
   else if (id < 0) { RETURN_FALSE_BECAUSE(id); }

	blobdir = mgd_get_blobdir(mgd_handle());
	if (!blobdir || *blobdir != '/') {
		RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
   }

	res =
	   mgd_sitegroup_record(mgd_handle(), "location,mimetype", "blobs",
				id);

	if (!res || !mgd_fetch(res)) {
		if (res) { mgd_release(res); }
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}

	location = mgd_colvalue(res, 0);
	mimetype = mgd_colvalue(res, 1);
	if (!mimetype || !*mimetype) {
		mimetype = "application/binary";
	}

	if (!location || !*location || strstr(location, "..")) {
		mgd_release(res);

		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}

	pool = mgd_alloc_pool();
	path = mgd_format(mgd_handle(), pool, "$s/$s", blobdir, location);

	if (!(fp = fopen(path, "r"))) {
		mgd_free_pool(pool);
		mgd_release(res);

		RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
	}

   content_type
      = mgd_format (mgd_handle(), pool, "Content-type: $s", mimetype);
   sapi_add_header(content_type, strlen(content_type), 1);

	if (sapi_send_headers(TSRMLS_C) != SUCCESS) {
		mgd_free_pool(pool);
		mgd_release(res);
      fclose(fp);

		RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
	}

	while ((b = fread(buf, 1, sizeof(buf), fp)) > 0) {
		PHPWRITE(buf, b);
	}

   fclose(fp);
	mgd_free_pool(pool);
	mgd_release(res);
	RETVAL_TRUE;
}

MGD_FUNCTION(mixed, stat_attachment, (int id))
{
	midgard_res *res;
	const char *location;
	midgard_pool *pool;
	char *path;
	struct stat blobstat;
   long id;
   zval **zv_id, *self;
   midgard_directory_config *dcfg;

	CHECK_MGD;
	RETVAL_FALSE;

	switch (ZEND_NUM_ARGS()) {
		case 0:
         zv_id = NULL;
         break;

		case 1:
			if (zend_parse_parameters(1 TSRMLS_CC, "z", &zv_id) != SUCCESS) WRONG_PARAM_COUNT;
         break;

		default:
			WRONG_PARAM_COUNT;
	}

	id = mgd_get_blob_id((self = getThis()), zv_id, NULL);
	if (id == 0) { RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS); }
	else if (id == MGD_ERR_INVALID_NAME) { WRONG_PARAM_COUNT; }
	else if (id < 0) { RETURN_FALSE_BECAUSE(id); }

	dcfg = mgd_dcfg();
	if (!dcfg || !dcfg->blobdir
			|| *(dcfg->blobdir) != '/') {
		RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
	}
	
	res = mgd_sitegroup_record(mgd_handle(), "location", "blobs", id);

	if (!res || !mgd_fetch(res)) {
		if (res) { mgd_release(res); }
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}

	location = mgd_colvalue(res, 0);

	if (!location || !*location || strstr(location, "..")) {
		mgd_release(res);

		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}

	pool = mgd_alloc_pool();
	path =
	   mgd_format(mgd_handle(), pool, "$s/$s", dcfg->blobdir,
		      location);
	stat(path, &blobstat);	

	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
	}
	add_next_index_long(return_value, blobstat.st_dev);
	add_next_index_long(return_value, blobstat.st_ino);
	add_next_index_long(return_value, blobstat.st_mode);
	add_next_index_long(return_value, blobstat.st_nlink);
	add_next_index_long(return_value, blobstat.st_uid);
	add_next_index_long(return_value, blobstat.st_gid);
#ifdef HAVE_ST_BLKSIZE
	add_next_index_long(return_value, blobstat.st_rdev);
#else
	add_next_index_long(return_value, -1);
#endif
	add_next_index_long(return_value, blobstat.st_size);
	add_next_index_long(return_value, blobstat.st_atime);
	add_next_index_long(return_value, blobstat.st_mtime);
	add_next_index_long(return_value, blobstat.st_ctime);
#ifdef HAVE_ST_BLKSIZE
	add_next_index_long(return_value, blobstat.st_blksize);
#else
	add_next_index_long(return_value, -1);
#endif
#ifdef HAVE_ST_BLOCKS
	add_next_index_long(return_value, blobstat.st_blocks);
#else
	add_next_index_long(return_value, -1);
#endif
}

MGD_FUNCTION(bool, delete_attachment, (int id))
{
	int pid;
	int ptable;
	const char *location;
	char *path;
	midgard_pool *pool;
	midgard_res *res;
	const char *blobdir;
   long id;
   zval *self, **zv_id;

	CHECK_MGD;
	RETVAL_FALSE;

	switch (ZEND_NUM_ARGS()) {
		case 0:
         zv_id = NULL;
         break;

		case 1:
			if (zend_parse_parameters(1 TSRMLS_CC, "z", &zv_id) != SUCCESS) WRONG_PARAM_COUNT;
         break;

		default:
			WRONG_PARAM_COUNT;
	}

   id = mgd_get_blob_id((self = getThis()), zv_id, NULL);
   if (id == 0) { RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS); }
   else if (id == MGD_ERR_INVALID_NAME) { WRONG_PARAM_COUNT; }
   else if (id < 0) { RETURN_FALSE_BECAUSE(id); }

	if (mgd_has_dependants(mgd_handle(), id, "blobs")) {
		RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);
   }

	blobdir = mgd_get_blobdir(mgd_handle());
	if (!blobdir || *blobdir != '/') {
		RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);
   }

	res =
	   mgd_sitegroup_record(mgd_handle(), "ptable,pid,location", "blobs",
				id);

	if (!res || !mgd_fetch(res)) {
		if (res) { mgd_release(res); }
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}

	ptable = mgd_lookup_table_id(mgd_colvalue(res, 0));
	pid = mgd_sql2id(res, 1);
	location = mgd_colvalue(res, 2);

	if (strstr(location, "..") || !isglobalowner(ptable, pid)) {
		mgd_release(res);

		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	}

	pool = mgd_alloc_pool();
	path = mgd_format(mgd_handle(), pool, "$s/$s", blobdir, location);

	if (unlink(path) == 0 || errno == ENOENT) {
		php_midgard_delete(return_value, "blobs", id);
		PHP_DELETE_REPLIGARD("blobs", id);
	}

#if HAVE_MIDGARD_QUOTA
	mgd_touch_recorded_quota(mgd_handle(), "blobsdata", mgd_sitegroup(mgd_handle()));
	mgd_touch_recorded_quota(mgd_handle(), "wholesg", mgd_sitegroup(mgd_handle()));
#endif

	mgd_free_pool(pool);
	mgd_release(res);
}

MGD_FUNCTION(bool, update_attachment, (int id, string name, string title, string mimetype, [int score, [int author]]))
{
	zval **id, **name, **title, **mimetype, **score, **author, *self;
	int ptable, pid;
	midgard_res *res = NULL;
	int sc, auth, calltype;
    long aid;
    zend_class_entry *ce;

	RETVAL_FALSE;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
		case 6:
			if (zend_parse_parameters
			    (6 TSRMLS_CC, "zzzzzz", &id, &name, &title, &mimetype, &score,
			     &author) != SUCCESS) {
				WRONG_PARAM_COUNT;
			}
			break;

		case 5:
			if (zend_parse_parameters
			    (5 TSRMLS_CC, "zzzzz", &id, &name, &title, &mimetype,
			     &score) != SUCCESS) {WRONG_PARAM_COUNT;
			}
			author = NULL;
			break;

		case 4:
			if (zend_parse_parameters
			    (4 TSRMLS_CC, "zzzz", &id, &name, &title, &mimetype) != SUCCESS) {
				WRONG_PARAM_COUNT;
			}
			author = NULL;
			score = NULL;
			break;

      case 0:
         id = NULL;
         break;

		default:
			WRONG_PARAM_COUNT;
			break;
	}

   aid = mgd_get_blob_id((self = getThis()), id, &calltype);
   if (aid == 0) { RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS); }
   else if (aid == MGD_ERR_INVALID_NAME) { WRONG_PARAM_COUNT; }
   else if (aid < 0) { RETURN_FALSE_BECAUSE(aid); }

   if (calltype == MGD_BLOB_CALL_SELF) {
      if (!MGD_PROPFIND(self, "name", name)
            || !MGD_PROPFIND(self, "title", title)
            || !MGD_PROPFIND(self, "mimetype", mimetype)) {
         RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
      }

      if (!MGD_PROPFIND(self, "author", author)) { author = NULL; }
      if (!MGD_PROPFIND(self, "score", score)) { score = NULL; }
   }

	convert_to_string_ex(name);
	convert_to_string_ex(title);
	convert_to_string_ex(mimetype);

	if (mgd_exists_bool(mgd_handle(), "blobs me, blobs other",
		       "me.id=$d AND me.id<>other.id"
		       " AND me.ptable=other.ptable AND me.pid=other.pid"
		       " AND other.name=$q",
		       aid, (*name)->value.str.val)) {
      mgd_release(res);
      RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);
	}

	res =
	   mgd_sitegroup_record(mgd_handle(), "ptable,pid,location", "blobs",
				aid);

	if (!res || !mgd_fetch(res)) {
		if (res) { mgd_release(res); }
		RETURN_FALSE_BECAUSE(MGD_ERR_NOT_EXISTS);
	}

	ptable = mgd_lookup_table_id(mgd_colvalue(res, 0));
	pid = mgd_sql2id(res, 1);


    /* Switch to anonymous mode for MgdSchema objects */
    MGD_PHP_PCLASS_NAME;
    if (g_hash_table_lookup(mgd_handle()->schema->types, ce->name) == NULL) {
        
        if (!isglobalowner(ptable, pid)) {
            mgd_release(res);
            RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED); 
        } 
    }
            

	if (score) {
		convert_to_long_ex(score);
		sc = (*score)->value.lval;
	}
	else
		sc = 0;
	if (author) {
		convert_to_long_ex(author);
		auth = (*author)->value.lval;
	}
	else
		auth = 0;

	MgdObject *gobj = midgard_object_new_by_id(mgd_handle(),
			"midgard_attachment", (gchar *)aid);
	
	if(midgard_quota_size_is_reached(gobj,
				midgard_object_get_size(gobj))){
		g_object_unref(gobj);
		mgd_release(res);
		RETURN_FALSE_BECAUSE(MGD_ERR_QUOTA);
	}
	g_object_unref(gobj);

	php_midgard_update(return_value, "blobs",
			   (author ?
			    "name=$q,title=$q,mimetype=$q,score=$d,author=$d"
			    : (score ? "name=$q,title=$q,mimetype=$q,score=$d" :
			       "name=$q,title=$q,mimetype=$q")),
			   aid, (*name)->value.str.val,
			   (*title)->value.str.val, (*mimetype)->value.str.val,
			   sc, auth);
	PHP_UPDATE_REPLIGARD("blobs", aid);
	mgd_release(res);
}

static zend_function_entry MidgardAttachmentMethods[] =
   {
      PHP_FALIAS(update,            mgd_update_attachment,  NULL)
      PHP_FALIAS(delete,            mgd_delete_attachment,  NULL)
      PHP_FALIAS(stat,              mgd_stat_attachment,    NULL)
#ifdef PHP_MIDGARD_LEGACY_API
      PHP_FALIAS(fetch,             mgd_oop_list_fetch,     NULL)
      MIDGARD_OOP_PARAMETER_METHODS
      MIDGARD_OOP_SITEGROUP_METHODS
#else
      PHP_FALIAS(open,              mgd_open_attachment,     NULL)
      PHP_FALIAS(serve,             mgd_serve_attachment,    NULL)
#endif
      { NULL, NULL, NULL }
   };
MidgardClass MidgardAttachment = {
   "MidgardAttachment",
   "blobs",
   MidgardAttachmentMethods,
   {},
   NULL
};

//#endif
