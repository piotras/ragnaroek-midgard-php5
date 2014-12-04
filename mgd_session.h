/* $Id: mgd_session.h 6491 2001-04-01 00:23:49Z emile $
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

#if HAVE_MIDGARD_SESSION
#ifndef MGD_SESSION_H
#define MGD_SESSION_H

extern ps_module ps_mod_midgard;
#define ps_midgard_ptr &ps_mod_midgard

PS_FUNCS(midgard);

#endif
#endif // HAVE_MIDGARD_SESSION
