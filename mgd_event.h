/* $Id: mgd_event.h 6938 2002-11-25 16:54:07Z armand $
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

#ifndef MGD_EVENT_H
#define MGD_EVENT_H

extern MGD_FUNCTION(ret_type, is_event_owner, (type param));
extern MGD_FUNCTION(ret_type, create_event, (type param));
extern MGD_FUNCTION(ret_type, update_event, (type param));
extern MGD_FUNCTION(ret_type, delete_event, (type param));
extern MGD_FUNCTION(ret_type, delete_event_tree, (type param));
extern MGD_FUNCTION(ret_type, walk_event_tree, (type param));
extern MGD_FUNCTION(ret_type, get_event, (type param));
extern MGD_FUNCTION(ret_type, list_events, (type param));
extern MGD_FUNCTION(ret_type, list_events_between, (type param));
extern MGD_FUNCTION(ret_type, list_events_all, (type param));
extern MGD_FUNCTION(ret_type, list_events_all_between, (type param));
extern MGD_FUNCTION(ret_type, count_events_in_period, (type param));
extern MGD_FUNCTION(ret_type, count_events_in_month, (type param));
extern MGD_FUNCTION(ret_type, copy_event, (type param));
extern MGD_FUNCTION(ret_type, move_event, (type param));
extern MGD_FUNCTION(ret_type, list_events_by_group, (type param));
extern MGD_FUNCTION(ret_type, list_events_between_by_group, (type param));
extern MGD_FUNCTION(ret_type, list_events_by_person, (type param));
extern MGD_FUNCTION(ret_type, list_events_between_by_person, (type param));
extern MGD_FUNCTION(ret_type, list_events_between_by_member, (type param));

#endif
