/* $Id: mgd_snippetdir.h 6491 2001-04-01 00:23:49Z emile $
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

#ifndef MGD_SNIPPETDIR_H
#define MGD_SNIPPETDIR_H

extern MGD_FUNCTION(ret_type, is_snippetdir_owner, (type param));
extern MGD_FUNCTION(ret_type, list_snippetdirs, (type param));
extern MGD_FUNCTION(ret_type, get_snippetdir, (type param));
extern MGD_FUNCTION(ret_type, get_snippetdir_by_path, (type param));
extern MGD_FUNCTION(ret_type, create_snippetdir, (type param));
extern MGD_FUNCTION(ret_type, update_snippetdir, (type param));
extern MGD_FUNCTION(ret_type, delete_snippetdir, (type param));
extern MGD_FUNCTION(ret_type, delete_snippetdir_tree, (type param));
extern MGD_FUNCTION(ret_type, walk_snippetdir_tree, (type param));
extern MGD_FUNCTION(ret_type, copy_snippetdir, (type param));
extern MGD_FUNCTION(ret_type, move_snippetdir, (type param));

#endif
