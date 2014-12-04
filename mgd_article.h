/* $Id: mgd_article.h 6995 2003-05-15 21:03:07Z david $
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

#ifndef MGD_ARTICLE_H
#define MGD_ARTICLE_H

extern MGD_FUNCTION(ret_type, is_article_owner, (type param));
extern MGD_FUNCTION(ret_type, is_article_in_topic_tree, (type param));
extern MGD_FUNCTION(ret_type, list_topic_articles, (type param));
extern MGD_FUNCTION(ret_type, list_reply_articles, (type param));
extern MGD_FUNCTION(ret_type, list_topic_articles_all, (type param));
extern MGD_FUNCTION(ret_type, list_topic_articles_all_fast, (type param));
extern MGD_FUNCTION(ret_type, list_topic_articles_all_of_person, (type param));
extern MGD_FUNCTION(ret_type, get_article, (type param));
extern MGD_FUNCTION(ret_type, get_reply_by_name, (type param));
extern MGD_FUNCTION(ret_type, create_article, (type param));
#if HAVE_MIDGARD_MULTILANG
extern MGD_FUNCTION(ret_type, create_article_content, (type param));
#endif /* HAVE_MIDGARD_MULTILANG */
extern MGD_FUNCTION(ret_type, update_article_score, (type param));
extern MGD_FUNCTION(ret_type, update_article_created, (type param));
extern MGD_FUNCTION(ret_type, update_article_replyto, (type param));
extern MGD_FUNCTION(ret_type, update_article_type, (type param));
extern MGD_FUNCTION(ret_type, toggle_article_lock, (type param));
extern MGD_FUNCTION(ret_type, approve_article, (type param));
extern MGD_FUNCTION(ret_type, update_article, (type param));
#if HAVE_MIDGARD_MULTILANG
extern MGD_FUNCTION(ret_type, update_article_content, (type param));
#endif /* HAVE_MIDGARD_MULTILANG */
extern MGD_FUNCTION(ret_type, delete_article, (type param));
#if HAVE_MIDGARD_MULTILANG
extern MGD_FUNCTION(ret_type, delete_article_content, (type param));
#endif /* HAVE_MIDGARD_MULTILANG */
extern MGD_FUNCTION(ret_type, copy_article, (type param));
extern MGD_FUNCTION(ret_type, move_article, (type param));
extern MGD_FUNCTION(ret_type, copy_reply, (type param));
extern MGD_FUNCTION(ret_type, move_reply, (type param));
extern MGD_FUNCTION(ret_type, delete_article_tree, (type param));
extern MGD_FUNCTION(ret_type, walk_article_tree, (type param));

#endif
