/* $Id: mgd_person.h 6762 2002-01-17 07:33:37Z emile $
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

#ifndef MGD_PERSON_H
#define MGD_PERSON_H

extern MGD_FUNCTION(ret_type, is_person_owner, (type param));
extern MGD_FUNCTION(ret_type, is_member, (type param));
extern MGD_FUNCTION(ret_type, list_persons, (type param));
extern MGD_FUNCTION(ret_type, list_persons_in_department, (type param));
extern MGD_FUNCTION(ret_type, list_topic_persons, (type param));
extern MGD_FUNCTION(ret_type, list_persons_in_department_all, (type param));
extern MGD_FUNCTION(ret_type, list_topic_persons_all, (type param));
extern MGD_FUNCTION(ret_type, list_persons_in_office, (type param));
extern MGD_FUNCTION(ret_type, get_person, (type param));
extern MGD_FUNCTION(ret_type, get_person_by_name, (type param));
extern MGD_FUNCTION(ret_type, create_person, (type param));
extern MGD_FUNCTION(ret_type, update_person, (type param));
extern MGD_FUNCTION(ret_type, update_password, (type param));
extern MGD_FUNCTION(ret_type, update_password_plain, (type param));
extern MGD_FUNCTION(ret_type, update_public, (type param));
extern MGD_FUNCTION(ret_type, delete_person, (type param));

#endif
