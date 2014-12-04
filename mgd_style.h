/* $Id: mgd_style.h 6491 2001-04-01 00:23:49Z emile $
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

#ifndef MGD_STYLE_H
#define MGD_STYLE_H

extern MGD_FUNCTION(ret_type, walk_style_tree, (type param));
extern MGD_FUNCTION(ret_type, is_style_owner, (type param));
extern MGD_FUNCTION(ret_type, list_styles, (type param));
extern MGD_FUNCTION(ret_type, get_style, (type param));
extern MGD_FUNCTION(ret_type, get_style_by_name, (type param));
extern MGD_FUNCTION(ret_type, create_style, (type param));
extern MGD_FUNCTION(ret_type, update_style, (type param));
extern MGD_FUNCTION(ret_type, delete_style, (type param));
extern MGD_FUNCTION(ret_type, copy_style, (type param));
extern MGD_FUNCTION(ret_type, move_style, (type param));
extern MGD_FUNCTION(ret_type, delete_style_tree, (type param));

#endif
