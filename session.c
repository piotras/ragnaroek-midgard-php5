/* $Id: session.c 7357 2004-10-27 12:56:29Z piotras $
Copyright (C) 1999 Jukka Zitting <jukka.zitting@iki.fi>
Copyright (C) 2000 The Midgard Project ry
Copyright (C) 2001 David Guerizec, Aurora SA <david@guerizec.net>

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

#include "php_midgard.h"

#if HAVE_MIDGARD
#if HAVE_MIDGARD_SESSION



#include "../session/php_session.h"
#include "mgd_session.h"

#define GET_MGD midgard * mgd = mgd_handle();

typedef struct {
	int id;
	int ready;
} ps_midgard;

ps_module ps_mod_midgard = {
	PS_MOD(midgard)
};

static int ps_midgard_valid_key(const char *key)
{
	size_t len;
	const char *p;
	char c;
	int ret = 1;

	for (p = key; (c = *p); p++) {
		/* valid characters are a..z,A..Z,0..9 */
		if (!((c >= 'a' && c <= 'z') ||
				(c >= 'A' && c <= 'Z') ||
				(c >= '0' && c <= '9'))) {
			ret = 0;
			break;
		}
	}

	len = p - key;
	
	if (len == 0)
		ret = 0;
	
	return ret;
}

static int ps_midgard_write(ps_midgard * data, const char * key, const char * val)
{
	GET_MGD;

	if(!data->ready)
		return FAILURE;
	if(data->id) {
		if(mgd_update(mgd, "session", data->id, 
						"sess_data=$q, "
						"expire=UNIX_TIMESTAMP() + 3600, "
						"changed=UNIX_TIMESTAMP()",
						val))
			return SUCCESS;
		assert(0);
	} else {
		if((data->id = mgd_create(mgd, "session",
						"sess_key, sess_data, person, expire, changed",
						"$q, $q, $d, UNIX_TIMESTAMP() + 3600, UNIX_TIMESTAMP()",
						key, val, mgd_user(mgd))))
			return SUCCESS;
		assert(0);
	}
	return FAILURE;
}

static void ps_midgard_open(ps_midgard *data, const char *key)
{
	GET_MGD;

	if(data->id) {
		php_error(E_WARNING, "Called twice ??");
	}

	data->id = 0;
	if(!mgd) {
		php_error(E_WARNING, "Midgard is not ready yet for session management,"
						" please come back later...");
		return;
	}
	if(!ps_midgard_valid_key(key))
		return;

	data->id = mgd_exists_id(mgd, "session", 
					"sess_key=$q AND person=$d", // AND expire>=UNIX_TIMESTAMP()",
					key, mgd_user(mgd));
	data->ready = 1;
	return;
}

#define PS_MIDGARD_DATA ps_midgard *data = PS_GET_MOD_DATA()

PS_OPEN_FUNC(midgard)
{
	ps_midgard *data;

	data = ecalloc(sizeof(*data), 1);
	PS_SET_MOD_DATA(data);

	data->id = 0;
	data->ready = 0;
	
	return SUCCESS;
}

PS_CLOSE_FUNC(midgard)
{
	PS_MIDGARD_DATA;

	efree(data);
	*mod_data = NULL;

	return SUCCESS;
}

PS_READ_FUNC(midgard)
{
	midgard_res * res;
	PS_MIDGARD_DATA;
	GET_MGD;

	ps_midgard_open(data, key);
	if(!data->id) return FAILURE;
	
	res = mgd_query(mgd, "SELECT sess_data FROM session WHERE id=$d",
					data->id);
	if(res) {
		if(mgd_fetch(res)) {
			*vallen = strlen(mgd_colvalue(res, 0));
			*val = estrdup(mgd_colvalue(res, 0));
			mgd_release(res);
			return SUCCESS;
		}
		mgd_release(res);
	}
	return FAILURE;
}

PS_WRITE_FUNC(midgard)
{
	PS_MIDGARD_DATA;
	GET_MGD;

	ps_midgard_open(data, key);
	if(!mgd) {
		php_error(E_WARNING, "Midgard is not ready yet for session management,"
						" please come back later...");
		return FAILURE;
	}
	return ps_midgard_write(data, key, val);
}

PS_DESTROY_FUNC(midgard)
{
	PS_MIDGARD_DATA;
	GET_MGD;

	ps_midgard_open(data, key);
	if(!data->id) return FAILURE;

	if(mgd_delete(mgd, "session", data->id))
		return SUCCESS;
	return FAILURE;
}

PS_GC_FUNC(midgard) 
{
	midgard_res * res;
	GET_MGD;
	
	if(!mgd) {
		php_error(E_WARNING, "Midgard is not ready yet for session management,"
						" please come back later...");
		return FAILURE;
	}
	res = mgd_query(mgd, "DELETE FROM session "
					"WHERE expire < UNIX_TIMESTAMP()");
	if(res) {
		mgd_release(res);
		return SUCCESS;
	}
	return FAILURE;
}
#endif // HAVE_MIDGARD_SESSION
#endif // HAVE_MIDGARD
