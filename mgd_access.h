/* $Id: mgd_access.h 6995 2003-05-15 21:03:07Z david $
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

#ifndef MGD_ACCESS_H
#define MGD_ACCESS_H

#include <midgard/midgard.h>

#define istopicreader(topic) mgd_istopicreader(mgd_handle(), (topic))
#define istopicowner(topic) mgd_istopicowner(mgd_handle(),(topic))
#define issnippetdirowner(snippetdir) mgd_issnippetdirowner(mgd_handle(),(snippetdir))
#define isgroupowner(group) mgd_isgroupowner(mgd_handle(),(group))
#define isuserowner(user) mgd_isuserowner(mgd_handle(),(user))
#define ispageelementowner(pageelement) mgd_ispageelementowner(mgd_handle(),(pageelement))
#define isarticlereader(article) mgd_isarticlereader(mgd_handle(),(article))
#define isarticleowner(article) mgd_isarticleowner(mgd_handle(),(article))
#define iseventowner(event) mgd_iseventowner(mgd_handle(),(event))
#define ishostowner(host) mgd_ishostowner(mgd_handle(),(host))
#define ispageowner(page) mgd_ispageowner(mgd_handle(),(page))
#if HAVE_MIDGARD_MULTILANG
#define ispagecontentowner(page,lang) mgd_ispagecontentowner(mgd_handle(),(page),(lang))
#endif /* HAVE_MIDGARD_MULTILANG */
#define isstyleowner(style) mgd_isstyleowner(mgd_handle(),(style))
#define isglobalowner(table,id) mgd_global_is_owner(mgd_handle(),(table),(id))
#if HAVE_MIDGARD_PAGELINKS
#define ispagelinkowner(pagelink) mgd_ispagelinkowner(mgd_handle(),(pagelink))
#endif

#endif
