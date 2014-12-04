/* $Id: mgd_quota.h 7029 2003-09-29 12:20:41Z david $
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

#ifndef MGD_QUOTA_H
#define MGD_QUOTA_H
#if HAVE_MIDGARD_QUOTA
extern MGD_FUNCTION(ret_type, list_quotas, (type param));
extern MGD_FUNCTION(ret_type, get_quota, (type param));
extern MGD_FUNCTION(ret_type, get_quota_by_tablename, (type param));
extern MGD_FUNCTION(ret_type, get_quota_info_by_tablename, (type param));
extern MGD_FUNCTION(ret_type, create_quota, (type param));
extern MGD_FUNCTION(ret_type, update_quota, (type param));
extern MGD_FUNCTION(ret_type, delete_quota, (type param));

#endif /* HAVE_MIDGARD_QUOTA */
#endif
