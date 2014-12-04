/* Copyright (C) 2007 Piotr Pokora <piotrek.pokora@gmail.com>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "php_midgard.h"
#include "php_midgard_gobject.h"

#include "php_midgard__helpers.h"

static zend_class_entry *php_midgard_user_class;

#define _GET_USER_OBJECT \
	zval *zval_object = getThis(); \
	php_midgard_gobject *php_gobject = \
                (php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC); \
        MidgardUser *user = MIDGARD_USER(php_gobject->gobject); \
        if(!user) php_error(E_ERROR, "Can not find underlying user instance");


/* Object constructor */
static PHP_METHOD(midgard_user, __construct)
{	
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	zval *zobject = NULL;
	MgdObject *person = NULL;

	zend_class_entry *user_ce = 
		php_midgard_get_baseclass_ptr_by_name("midgard_person");

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|O",
				&zobject, user_ce) == FAILURE)
		return;
	
	if(zobject != NULL) {
		
		php_midgard_gobject *php_gobject_param =
			(php_midgard_gobject *)zend_object_store_get_object(zobject TSRMLS_CC);
		person = MIDGARD_OBJECT(php_gobject_param->gobject);	
	} 

	MidgardUser *user = midgard_user_new(person);

	if(!user) {

		php_midgard_error_exception_throw(mgd_handle());
		return;
	}

	php_midgard_gobject *php_gobject = 
		(php_midgard_gobject *)zend_object_store_get_object(zval_object TSRMLS_CC);
	php_gobject->gobject = G_OBJECT(user);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_user___construct, 0, 0, 1)
        ZEND_ARG_OBJ_INFO(0, person, midgard_person, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, auth)
{
	RETVAL_FALSE;
	CHECK_MGD;
	const gchar *name, *password, *sitegroup = NULL;
	gint name_length, password_length, sitegroup_length;
	zend_bool zbool = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|sb",
				&name, &name_length, 
				&password, &password_length,
				&sitegroup, &sitegroup_length,
				&zbool) == FAILURE) 
		return;

	MidgardConnection *mgd = mgd_handle_singleton_get();
	MidgardUser *user = midgard_user_auth(mgd, 
			name, password, sitegroup, zbool);
	
	if(user == NULL)
		RETURN_FALSE;

	php_midgard_gobject_new_with_gobject(return_value, php_midgard_user_class, G_OBJECT(user), TRUE);

	/* 1. Returned midgard_user is kind of reference, so mark it as reference.
	 * 2. Increase reference count, so object is not destroyed by zend before request end */
	Z_SET_ISREF_P(return_value);
	zval_add_ref(&return_value);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_user_auth, 0, 0, 2)
        ZEND_ARG_INFO(0, username)
        ZEND_ARG_INFO(0, password)
        ZEND_ARG_INFO(0, sitegroup)
        ZEND_ARG_INFO(0, trusted_auth)
ZEND_END_ARG_INFO()


static PHP_METHOD(midgard_user, password)
{
	RETVAL_FALSE;
	CHECK_MGD;
	const gchar *name, *password;
	guint name_length, password_length;
	guint hashtype = MIDGARD_USER_HASH_LEGACY;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|l",
				&name, &name_length,
				&password, &password_length, 
				&hashtype) == FAILURE) 
		return;

	_GET_USER_OBJECT;

	gboolean rv = midgard_user_password(user, name, password, hashtype);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_user_password, 0, 0, 2)
        ZEND_ARG_INFO(0, username)
        ZEND_ARG_INFO(0, password)
        ZEND_ARG_INFO(0, hashtype)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, get_person)
{
	RETVAL_FALSE;
	CHECK_MGD;

	zend_class_entry *person_ce =
		php_midgard_get_baseclass_ptr_by_name("midgard_person");
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") ==  FAILURE)
		return;
	
	_GET_USER_OBJECT;
	
	MgdObject *person = midgard_user_get_person(user);
	
	if(person == NULL)
		RETURN_NULL();

	php_midgard_gobject_new_with_gobject(return_value, person_ce, G_OBJECT(person), TRUE);

	Z_SET_ISREF_P(return_value);
	zval_add_ref(&return_value);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_user_get_person, 0, 1, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, is_user)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	_GET_USER_OBJECT;
	RETURN_BOOL(midgard_user_is_user(user));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_user_is_user, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, is_admin)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	_GET_USER_OBJECT;
	RETURN_BOOL(midgard_user_is_admin(user));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_user_is_admin, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, is_root)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE)
		return;

	_GET_USER_OBJECT;
	RETURN_BOOL(midgard_user_is_root(user));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_user_is_root, 0)
ZEND_END_ARG_INFO()

/* Initialize ZEND&PHP class */
void php_midgard_user_init(int module_number)
{
	static php_midgard_function_entry midgard_user_methods[] = {
		PHP_ME(midgard_user,	__construct,	arginfo_midgard_user___construct,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_user,	auth,		arginfo_midgard_user_auth,		ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_user,	password,	arginfo_midgard_user_password,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_user,    is_user,	arginfo_midgard_user_is_user,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_user,    is_admin,       arginfo_midgard_user_is_admin,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_user,    is_root,       	arginfo_midgard_user_is_root,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_user,    get_person,     arginfo_midgard_user_get_person,	ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_user_class_entry;
	TSRMLS_FETCH();

	INIT_CLASS_ENTRY(
			php_midgard_user_class_entry,
			"midgard_user", midgard_user_methods);

	php_midgard_user_class =
		zend_register_internal_class(
				&php_midgard_user_class_entry TSRMLS_CC);
	
	/* Set function to initialize underlying data */
	php_midgard_user_class->create_object = php_midgard_gobject_new;	
}
