/* $Id: person.c 15758 2008-03-18 12:52:39Z piotras $
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

#ifdef PHP_MIDGARD_LEGACY_API

int isowner()
{
	return mgd_isadmin(mgd_handle()) || mgd_exists_id(mgd_handle(), "grp",
   "owner IN $D", mgd_groups(mgd_handle()));
}

MGD_FUNCTION(ret_type, is_person_owner, (type param))
{
    IDINIT;

    CHECK_MGD;
    RETVAL_LONG(isuserowner(id));
}

MGD_FUNCTION(ret_type, is_member, (type param))
{
zval **zv_gid, **zv_uid;;

	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
		case 1:
			if (zend_get_parameters_ex(1, &zv_gid)==FAILURE) { WRONG_PARAM_COUNT; }
			convert_to_long_ex(zv_gid);
			RETVAL_LONG(mgd_ismember(mgd_handle(), (*zv_gid)->value.lval));
			break;

		case 2:
			if (zend_get_parameters_ex(2, &zv_gid, &zv_uid)==FAILURE) {
				WRONG_PARAM_COUNT;
			}
      convert_to_long_ex(zv_gid);
      convert_to_long_ex(zv_uid);
			RETVAL_LONG(mgd_exists_id(mgd_handle(),	"member",
															"uid=$d AND gid=$d",
															(*zv_uid)->value.lval,
															(*zv_gid)->value.lval))
			break;

		default:
			WRONG_PARAM_COUNT;
			break;
  }
}

MGD_FUNCTION(ret_type, list_persons, (type param))
{
    php_midgard_select(&MidgardPerson, return_value, PERSON_SELECT,
		   "person", NULL, "lastname,firstname");
}

static const char *person_sort(const char *order)
{
    static struct { const char *order, *sort; } sort[] = {
	{ "alpha", "lastname,firstname" },
	{ "created", "created ASC" },
	{ NULL, "created DESC" }
    };
    int i;

    for (i = 0; sort[i].order; i++) 
	if (strcmp(order, sort[i].order) == 0)
	    return sort[i].sort;

    return sort[i].sort;
}

MGD_FUNCTION(ret_type, list_persons_in_department, (type param))
{
    const char *sortv = NULL;
    zval **id, **sortn;

	CHECK_MGD;

    switch (ZEND_NUM_ARGS()) {
    case 2:
	if (zend_get_parameters_ex(2, &id, &sortn) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_string_ex(sortn);
	    sortv = person_sort((*sortn)->value.str.val);
	    break;
	}
    case 1:
	if (zend_get_parameters_ex(1, &id) == SUCCESS) {
	    convert_to_long_ex(id);
	    sortv = person_sort("");
	    break;
	}
    default:
	WRONG_PARAM_COUNT;
    }

    php_midgard_select(&MidgardPerson, return_value, PERSON_SELECT,
		   "person", "department=$d", sortv, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, list_persons_in_department_all, (type param))
{
    int *deps;
    const char *sortv = NULL;
    zval **id, **sortn;

	CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 2:
	if (zend_get_parameters_ex(2, &id, &sortn) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_string_ex(sortn);
	    sortv = person_sort((*sortn)->value.str.val);
	    break;
	}
    case 1:
	if (zend_get_parameters_ex(1, &id) == SUCCESS) {
	    convert_to_long_ex(id);
	    sortv = person_sort("");
	    break;
	}
    default:
	WRONG_PARAM_COUNT;
    }

    deps = mgd_tree(mgd_handle(), "topic", "up", (*id)->value.lval, 0, NULL);

	if(deps) {
	    php_midgard_select(&MidgardPerson, return_value, PERSON_SELECT,
			   "person", "department IN $D", sortv, deps);
		free(deps);
	}
}

MGD_FUNCTION(ret_type, list_persons_in_office, (type param))
{
    const char *sortv = NULL;
    zval **id, **sortn;

	CHECK_MGD;
    switch (ZEND_NUM_ARGS()) {
    case 2:
	if (zend_get_parameters_ex(2, &id, &sortn) == SUCCESS) {
	    convert_to_long_ex(id);
	    convert_to_string_ex(sortn);
	    sortv = person_sort((*sortn)->value.str.val);
	    break;
	}
    case 1:
	if (zend_get_parameters_ex(1, &id) == SUCCESS) {
	    convert_to_long_ex(id);
	    sortv = person_sort("");
	    break;
	}
    default:
	WRONG_PARAM_COUNT;
    }

    php_midgard_select(&MidgardPerson, return_value, PERSON_SELECT,
		   "person", "office=$d", sortv, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, get_person, (type param))
{
	zval **id;
	CHECK_MGD;

	switch (ZEND_NUM_ARGS()) {
	   case 0:
		   php_midgard_bless(return_value, &MidgardPerson);
		   mgd_object_init(return_value, "firstname", "lastname",
		      "birthdate", "street", "postcode", "city", "handphone",
		      "homephone", "workphone", "homepage", "email", "pgpkey",
            "topic", "department", "office", "extra",
            "subtopic", "img", "creator", "created",
            NULL);
		return;

	case 1:
		if (zend_get_parameters_ex(1, &id) != SUCCESS) { WRONG_PARAM_COUNT; }
    convert_to_long_ex(id);
    break;
      
	default:
		WRONG_PARAM_COUNT;
      break;
	}
  
  php_midgard_get_object(return_value, MIDGARD_OBJECT_PERSON, (*id)->value.lval);
}

MGD_FUNCTION(ret_type, get_person_by_name, (type param))
{
	zval **id;
   long uid;

	CHECK_MGD;

	if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &id) != SUCCESS) { WRONG_PARAM_COUNT; }

   convert_to_string_ex(id);
   uid = mgd_exists_id(mgd_handle(), "person", "username=$q", (*id)->value.str.val);

   php_midgard_get_object(return_value, MIDGARD_OBJECT_PERSON, uid);
}

MGD_FUNCTION(ret_type, create_person, (type param))
{
    zval **firstname, **lastname, **birthdate;
    zval **street, **postcode, **city, **handphone, **homephone, **workphone;
    zval **homepage, **email, **pgpkey = NULL, **topic, **department, **office, **extra, *self;
    char *pgp;
	 midgard_pool *pool = NULL;

    RETVAL_FALSE;
	CHECK_MGD;

   if ((self = getThis()) != NULL) {
      if (ZEND_NUM_ARGS() != 0) { WRONG_PARAM_COUNT; }

      if (!MGD_PROPFIND(self, "pgpkey", pgpkey)) { pgpkey = NULL; }

      if (!MGD_PROPFIND(self, "firstname", firstname)
            || !MGD_PROPFIND(self, "lastname", lastname)
            || !MGD_PROPFIND(self, "birthdate", birthdate)
            || !MGD_PROPFIND(self, "street", street)
            || !MGD_PROPFIND(self, "postcode", postcode)
            || !MGD_PROPFIND(self, "city", city)
            || !MGD_PROPFIND(self, "handphone", handphone)
            || !MGD_PROPFIND(self, "homephone", homephone)
            || !MGD_PROPFIND(self, "workphone", workphone)
            || !MGD_PROPFIND(self, "homepage", homepage)
            || !MGD_PROPFIND(self, "email", email)
            || !MGD_PROPFIND(self, "topic", topic)
            || !MGD_PROPFIND(self, "department", department)
            || !MGD_PROPFIND(self, "office", office)
            || !MGD_PROPFIND(self, "extra", extra)
            ) {
         RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
      }
   } else {
    if (ZEND_NUM_ARGS() != 15
		|| zend_get_parameters_ex(15, &firstname, &lastname, &birthdate, &street,
						 &postcode, &city, &handphone, &homephone, &workphone,
						 &homepage, &email, &topic, &department, &office,
						 &extra) != SUCCESS)
		WRONG_PARAM_COUNT;
   }

	pool = mgd_alloc_pool();
	if (pool == NULL) RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);

    convert_to_string_ex(firstname);
    convert_to_string_ex(lastname);
    convert_to_string_ex(birthdate);
    convert_to_string_ex(street);
    convert_to_string_ex(postcode);
    convert_to_string_ex(city);
    convert_to_string_ex(handphone);
    convert_to_string_ex(homephone);
    convert_to_string_ex(workphone);
    convert_to_string_ex(homepage);
    convert_to_string_ex(email);
    if (pgpkey == NULL) {
      pgp = "''";
    } else {
      convert_to_string_ex(pgpkey);
		pgp = mgd_format(mgd_handle(), pool, "$q", (*pgpkey)->value.str.val);
    }
    convert_to_long_ex(topic);
    convert_to_long_ex(department);
    convert_to_long_ex(office);
    convert_to_string_ex(extra);

/* armand: 
 * Commented out as part of the aegir/nadmin patches going into main release
#ifndef HAVE_AEGIR_PATCHES
	if (!isowner()) {
		mgd_free_pool(pool);
      RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
   }
#endif
*/

    php_midgard_create(return_value, self, "person", "firstname,lastname,"
				   "birthdate,street,postcode,city,handphone,"
				   "homephone,workphone,homepage,email,"
				   "topic,department,office,extra,created,creator,pgpkey",
				   "$q,$q,$t,$q,$q,$q,$q,$q,$q,$q,$q,$d,$d,$d,$q,Now(),$d,$s",
				   (*firstname)->value.str.val, (*lastname)->value.str.val,
				   (*birthdate)->value.str.val,
				   (*street)->value.str.val,    (*postcode)->value.str.val,
				   (*city)->value.str.val,      (*handphone)->value.str.val,
				   (*homephone)->value.str.val, (*workphone)->value.str.val,
				   (*homepage)->value.str.val,  (*email)->value.str.val,
				   (*topic)->value.lval,        (*department)->value.lval,
				   (*office)->value.lval,       (*extra)->value.str.val,
				   mgd_user(mgd_handle()),      pgp);

   mgd_free_pool(pool);

    PHP_CREATE_REPLIGARD("person",return_value->value.lval);
}

MGD_FUNCTION(ret_type, update_person, (type param))
{
    zval **id, **firstname, **lastname, **birthdate, **street, **postcode, **city;
    zval **handphone, **homephone, **workphone, **homepage, **email, **pgpkey = NULL;
    zval **topic, **department, **office, **extra, *self;
    char *pgp;
	 midgard_pool *pool = NULL;

	CHECK_MGD;
   if ((self = getThis()) != NULL) {
      if (ZEND_NUM_ARGS() != 0) { WRONG_PARAM_COUNT; }

      if (!MGD_PROPFIND(self, "pgpkey", pgpkey)) { pgpkey = NULL; }

      if (!MGD_PROPFIND(self, "id", id)
            || !MGD_PROPFIND(self, "firstname", firstname)
            || !MGD_PROPFIND(self, "lastname", lastname)
            || !MGD_PROPFIND(self, "birthdate", birthdate)
            || !MGD_PROPFIND(self, "street", street)
            || !MGD_PROPFIND(self, "postcode", postcode)
            || !MGD_PROPFIND(self, "city", city)
            || !MGD_PROPFIND(self, "handphone", handphone)
            || !MGD_PROPFIND(self, "homephone", homephone)
            || !MGD_PROPFIND(self, "workphone", workphone)
            || !MGD_PROPFIND(self, "homepage", homepage)
            || !MGD_PROPFIND(self, "email", email)
            || !MGD_PROPFIND(self, "topic", topic)
            || !MGD_PROPFIND(self, "department", department)
            || !MGD_PROPFIND(self, "office", office)
            || !MGD_PROPFIND(self, "extra", extra)
            ) {
         RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_OBJECT);
      }
   } else {
    if (ZEND_NUM_ARGS() != 16
	|| zend_get_parameters_ex(16, &id, &firstname, &lastname, &birthdate,
                         &street, &postcode, &city,
			 &handphone, &homephone, &workphone, &homepage, &email,
			 &topic, &department, &office, &extra) != SUCCESS)
	WRONG_PARAM_COUNT;
   }

	pool = mgd_alloc_pool();
	if (pool == NULL) RETURN_FALSE_BECAUSE(MGD_ERR_INTERNAL);

    convert_to_long_ex(id);
    convert_to_string_ex(firstname);
    convert_to_string_ex(lastname);
    convert_to_string_ex(birthdate);
    convert_to_string_ex(street);
    convert_to_string_ex(postcode);
    convert_to_string_ex(city);
    convert_to_string_ex(handphone);
    convert_to_string_ex(homephone);
    convert_to_string_ex(workphone);
    convert_to_string_ex(homepage);
    convert_to_string_ex(email);
    if (pgpkey == NULL) {
      pgp = "pgpkey";
    } else {
      convert_to_string_ex(pgpkey);
		pgp = mgd_format(mgd_handle(), pool, "$q", (*pgpkey)->value.str.val);
    }
    convert_to_long_ex(topic);
    convert_to_long_ex(department);
    convert_to_long_ex(office);
    convert_to_string_ex(extra);

    if (!isuserowner((*id)->value.lval)) {
      mgd_free_pool(pool);
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    }

    php_midgard_update(return_value, "person", "firstname=$q,lastname=$q,"
				   "birthdate=$t,street=$q,postcode=$q,city=$q,"
				   "handphone=$q,homephone=$q,workphone=$q,homepage=$q,"
				   "email=$q,topic=$d,department=$d,office=$d,extra=$q,pgpkey=$s",
				   (*id)->value.lval,
				   (*firstname)->value.str.val, (*lastname)->value.str.val,
				   (*birthdate)->value.str.val, (*street)->value.str.val,
				   (*postcode)->value.str.val,  (*city)->value.str.val,
				   (*handphone)->value.str.val, (*homephone)->value.str.val,
				   (*workphone)->value.str.val, (*homepage)->value.str.val,
				   (*email)->value.str.val,     (*topic)->value.lval,
				   (*department)->value.lval,   (*office)->value.lval,
				   (*extra)->value.str.val,     pgp);

   mgd_free_pool(pool);

    PHP_UPDATE_REPLIGARD("person",(*id)->value.lval);
}

MGD_FUNCTION(ret_type, update_password, (type param))
{
    zval **uid, **username, **password;

	CHECK_MGD;
    if (ZEND_NUM_ARGS() != 3
		|| zend_get_parameters_ex(3, &uid, &username, &password) != SUCCESS)
		WRONG_PARAM_COUNT;
    convert_to_long_ex(uid);
    convert_to_string_ex(username);
    convert_to_string_ex(password);
	
    if (!mgd_isadmin(mgd_handle()) && !mgd_isuser(mgd_handle(), (*uid)->value.lval))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	
    /* no 'magic' chars in username */
    if (strpbrk((*username)->value.str.val, MIDGARD_LOGIN_RESERVED_CHARS))
	 	RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_NAME);

    if ((*username)->value.str.len != 0
	 		&& mgd_exists_bool(mgd_handle(), "person this, person other",
	 									"this.id=$d"
										" AND other.id<>$d AND other.username=$q"
										" AND this.sitegroup=other.sitegroup"
										,
										(*uid)->value.lval, (*uid)->value.lval,
										(*username)->value.str.val)) RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

    php_midgard_update(return_value, "person", "username=$q,password=Encrypt($q)",
				   (*uid)->value.lval, (*username)->value.str.val,
				   (*password)->value.str.val);
    PHP_UPDATE_REPLIGARD("person",(*uid)->value.lval);
}

MGD_FUNCTION(ret_type, update_password_plain, (type param))
{
    zval **uid, **username, **password;

	CHECK_MGD;
    if (ZEND_NUM_ARGS() != 3
		|| zend_get_parameters_ex(3, &uid, &username, &password) != SUCCESS)
		WRONG_PARAM_COUNT;
    convert_to_long_ex(uid);
    convert_to_string_ex(username);
    convert_to_string_ex(password);
	
    if (!mgd_isadmin(mgd_handle()) && !mgd_isuser(mgd_handle(), (*uid)->value.lval)
		&& !(isuserowner((*uid)->value.lval)
			 && mgd_exists_id(mgd_handle(), "person",
						   "id=$d AND (password='' OR Left(password,2)='**')",
						   (*uid)->value.lval)))
		RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
	
    /* no 'magic' chars in username */
    if (strpbrk((*username)->value.str.val, MIDGARD_LOGIN_RESERVED_CHARS)) RETURN_FALSE_BECAUSE(MGD_ERR_INVALID_NAME);

    if ((*username)->value.str.len != 0
	 		&& mgd_exists_bool(mgd_handle(), "person this, person other",
	 									"this.id=$d"
										" AND other.id<>$d AND other.username=$q"
										" AND this.sitegroup=other.sitegroup",
										(*uid)->value.lval, (*uid)->value.lval,
										(*username)->value.str.val)) RETURN_FALSE_BECAUSE(MGD_ERR_DUPLICATE);

    php_midgard_update(return_value, "person",
				   "username=$q,password=Concat('**',$q)",
				   (*uid)->value.lval, (*username)->value.str.val,
				   (*password)->value.str.val);
    PHP_UPDATE_REPLIGARD("person",(*uid)->value.lval);
}

MGD_FUNCTION(ret_type, update_public, (type param))
{
    zval **uid, **addressp, **phonep, **homepagep, **emailp, **extrap;

    RETVAL_FALSE;
	CHECK_MGD;
    if (ZEND_NUM_ARGS() != 6
		|| zend_get_parameters_ex(6, &uid, &addressp, &phonep,
						 &homepagep, &emailp, &extrap) != SUCCESS)
	WRONG_PARAM_COUNT;
    convert_to_long_ex(uid);
    convert_to_long_ex(addressp);
    convert_to_long_ex(phonep);
    convert_to_long_ex(homepagep);
    convert_to_long_ex(emailp);
    convert_to_long_ex(extrap);

    if (!isuserowner((*uid)->value.lval)) RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);

    php_midgard_update(return_value, "person", "info=(info&1)|$d",
				   (*uid)->value.lval, 
				   ((*addressp)->value.lval != 0) << 1
				   | ((*phonep)->value.lval != 0) << 2
				   | ((*homepagep)->value.lval != 0) << 3
				   | ((*emailp)->value.lval != 0) << 4
				   | ((*extrap)->value.lval != 0) << 5);
    PHP_UPDATE_REPLIGARD("person",(*uid)->value.lval);
}

MGD_FUNCTION(ret_type, delete_person, (type param))
{
    IDINIT;
	CHECK_MGD;
    if(mgd_has_dependants(mgd_handle(),id,"person"))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

	if(mgd_exists_id(mgd_handle(), "article", "author=$d", id)
		|| mgd_exists_id(mgd_handle(), "member", "uid=$d", id)
		|| mgd_exists_id(mgd_handle(), "eventmember", "uid=$d", id)
		|| mgd_exists_id(mgd_handle(), "page", "author=$d", id))
	RETURN_FALSE_BECAUSE(MGD_ERR_HAS_DEPENDANTS);

    if (!isuserowner(id)) RETURN_FALSE_BECAUSE(MGD_ERR_ACCESS_DENIED);
    php_midgard_delete(return_value, "person", id);
    PHP_DELETE_REPLIGARD("person", id);
}

MIDGARD_CLASS(MidgardPerson, person, midgardperson, person)

#endif
