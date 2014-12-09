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

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(string, preparse, (type param))
#else
PHP_FUNCTION(mgd_preparse)
#endif
{
	zval *phpcode;
	GByteArray *buffer;
	CHECK_MGD;   

	if (ZEND_NUM_ARGS() != 1 && ZEND_NUM_ARGS() != 2) { WRONG_PARAM_COUNT; }
	if (zend_parse_parameters(1 TSRMLS_CC, "z", &phpcode) != SUCCESS) { WRONG_PARAM_COUNT; }
	
	convert_to_string_ex(&phpcode);
	buffer = mgd_preparse_string(Z_STRVAL_P(phpcode));
	
	RETVAL_STRING((gchar *)buffer->data, 1);
	g_byte_array_free(buffer, TRUE);
}

#ifdef PHP_MIDGARD_LEGACY_API
MGD_FUNCTION(string, format, (string value, string formatspec))
#else
PHP_FUNCTION(mgd_format)
#endif
{
	midgard_pool *pool;
	zval *value, *formatter;
	char *fmt = "";
	char fmtspec[3];
	CHECK_MGD;
	
	switch (ZEND_NUM_ARGS()) {
		case 2:
			if (zend_parse_parameters(2 TSRMLS_CC, "zz", &value, &formatter) != SUCCESS) {
				WRONG_PARAM_COUNT;
			}
			break;
		case 1:
			if (zend_parse_parameters(1 TSRMLS_CC, "z", &value) != SUCCESS) {
				WRONG_PARAM_COUNT;
			}
			fmt = NULL;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	convert_to_string_ex(&value);
	if (fmt) {
		convert_to_string_ex(&formatter);
		fmt = Z_STRVAL_P(formatter);
	} else {
		fmt = "";
	}
	
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
	RETVAL_STRING(mgd_format(mgd_handle(), pool, fmtspec,
				Z_STRVAL_P(value)), 1);
	mgd_free_pool(pool);
}
