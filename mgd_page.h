/* $Id: mgd_page.h 6995 2003-05-15 21:03:07Z david $
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

#ifndef MGD_PAGE_H
#define MGD_PAGE_H

extern MGD_FUNCTION(ret_type, is_page_owner, (type param));
extern MGD_FUNCTION(ret_type, copy_page, (type param));
extern MGD_FUNCTION(ret_type, move_page, (type param));
extern MGD_FUNCTION(ret_type, list_pages, (type param));
extern MGD_FUNCTION(ret_type, is_in_page_tree, (type param));
extern MGD_FUNCTION(ret_type, get_page, (type param));
extern MGD_FUNCTION(ret_type, get_page_by_name, (type param));
extern MGD_FUNCTION(ret_type, create_page, (type param));
#if HAVE_MIDGARD_MULTILANG
extern MGD_FUNCTION(ret_type, create_page_content, (type param));
#endif /* HAVE_MIDGARD_MULTILANG */
extern MGD_FUNCTION(ret_type, update_page, (type param));
#if HAVE_MIDGARD_MULTILANG
extern MGD_FUNCTION(ret_type, update_page_content, (type param));
#endif /* HAVE_MIDGARD_MULTILANG */
extern MGD_FUNCTION(ret_type, delete_page, (type param));
#if HAVE_MIDGARD_MULTILANG
extern MGD_FUNCTION(ret_type, delete_page_content, (type param));
#endif /* HAVE_MIDGARD_MULTILANG */
extern MGD_FUNCTION(ret_type, page_has_children, (type param));
extern MGD_FUNCTION(ret_type, delete_page_tree, (type param));
extern MGD_FUNCTION(ret_type, walk_page_tree, (type param));

#endif
