/* $Id: preparse.c 27410 2014-09-01 07:39:28Z piotras $
Copyright (C) 1999 Jukka Zitting <jukka.zitting@iki.fi>
Copyright (C) 2000 The Midgard Project ry
Copyright (C) 2000 Emile Heyns, Aurora SA <emile@iris-advies.com>
Copyright (C) 2001 Emile Heyns

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

#include "mgd_internal.h"
#include "mgd_oop.h"
#include <glib.h>
#include <midgard/midgard_apache.h>
#include <midgard/pageresolve.h>

static void mgd_preparse_buffer_add(char *text, int len, void *userdata)
{
	GByteArray *buffer = (GByteArray*)userdata;
	g_byte_array_append(buffer, (guint8*)text, len);
}

GByteArray *mgd_preparse_string(char *phpcode)
{
	mgd_parser_itf_t itf;
	GByteArray *buffer = g_byte_array_new();
	midgard_request_config *rcfg = mgd_rcfg();
	
	itf.output.func = mgd_preparse_buffer_add;
	itf.output.userdata = buffer;
	itf.get_element.func = midgard_pc_get_element;
	itf.get_element.userdata = rcfg->elements;
	mgd_preparse_buffer(phpcode, &itf);
	
	/*[eeh] add terminating '\0' */
	guint8 const terminator = 0;
	g_byte_array_append(buffer, &terminator , 1);
	
	return buffer;
}

PHP_FUNCTION(mgd_preparse)
{
	char *phpcode;
	int phpcode_length;
	GByteArray *buffer;
	CHECK_MGD;   

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &phpcode, &phpcode_length) != SUCCESS) { 
		return;
	}
	
	buffer = mgd_preparse_string(phpcode);
	RETVAL_STRING((gchar *)buffer->data, 1);
	g_byte_array_free(buffer, TRUE);
}

PHP_FUNCTION(mgd_format)
{
	midgard_pool *pool;
	char *value, *formatter = NULL;
	int value_length, formatter_length;
	char fmtspec[3];
	char *fmt = "";
	CHECK_MGD;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &value, &value_length, &formatter, &formatter_length) != SUCCESS) {
		return;
	}
	
	if (formatter != NULL)
		fmt = formatter;

	pool = mgd_alloc_pool();
	strcpy(fmtspec, "$?");
	fmtspec[1] = fmt[0];
	
	switch (fmtspec[1]) {
		case '\0': /* default */
		case 'T':
			fmtspec[1] = 'p';
			break;
		case 't':
			fmtspec[1] = 'h';
			break;
		case 'h':
			fmtspec[1] = 'H';
			break;
		case 'u':
		case 'f':
		case 'F':
			break;
		default:
			fprintf(stderr, "Unknown formatter '%c'\n", fmtspec[1]);
			fmtspec[1] = 'p';
			break;
	}
	/* We should duplicate string , but midgard pool is not freed anyway */
	RETVAL_STRING(mgd_format(mgd_handle(), pool, fmtspec, value), 1);
	mgd_free_pool(pool);
}
