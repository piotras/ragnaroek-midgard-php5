/* $Id: mgd_snippet.h 7296 2004-09-09 21:51:14Z piotras $
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

#ifndef MGD_SNIPPET_H
#define MGD_SNIPPET_H

extern MGD_FUNCTION(ret_type, snippet_exists, (type param));
extern MGD_FUNCTION(ret_type, list_snippets, (type param));
extern MGD_FUNCTION(ret_type, get_snippet, (type param));
extern MGD_FUNCTION(ret_type, get_snippet_by_name, (type param));
extern MGD_FUNCTION(ret_type, create_snippet, (type param));
#if HAVE_MIDGARD_MULTILANG
extern MGD_FUNCTION(ret_type, create_snippet_content, (type param));
#endif /* HAVE_MIDGARD_MULTILANG */
extern MGD_FUNCTION(ret_type, update_snippet, (type param));
#if HAVE_MIDGARD_MULTILANG
extern MGD_FUNCTION(ret_type, update_snippet_content, (type param));
#endif /* HAVE_MIDGARD_MULTILANG */
extern MGD_FUNCTION(ret_type, delete_snippet, (type param));
#if HAVE_MIDGARD_MULTILANG
extern MGD_FUNCTION(ret_type, delete_snippet_content, (type param));
#endif /* HAVE_MIDGARD_MULTILANG */
extern MGD_FUNCTION(ret_type, copy_snippet, (type param));
extern MGD_FUNCTION(ret_type, move_snippet, (type param));
extern MGD_FUNCTION(ret_type, get_snippet_by_path, (type param));

#endif
