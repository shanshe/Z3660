/*--------------------------------------------------------------------------

  exFAT filesystem for MorphOS:
  Copyright © 2014-2015 Rupert Hausberger

  FAT12/16/32 filesystem handler:
  Copyright © 2006 Marek Szyprowski
  Copyright © 2007-2008 The AROS Development Team

  exFAT implementation library:
  Copyright © 2010-2013 Andrew Nayenko


  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

--------------------------------------------------------------------------*/

#include "include.h"

/*------------------------------------------------------------------------*/

typedef int wchar_t;

static char* wchar_to_utf8(char* output, wchar_t wc, size_t outsize)
{
	if (wc <= 0x7f)
	{
		if (outsize < 1)
			return NULL;
		*output++ = (char) wc;
	}
	else if (wc <= 0x7ff)
	{
		if (outsize < 2)
			return NULL;
		*output++ = 0xc0 | (wc >> 6);
		*output++ = 0x80 | (wc & 0x3f);
	}
	else if (wc <= 0xffff)
	{
		if (outsize < 3)
			return NULL;
		*output++ = 0xe0 | (wc >> 12);
		*output++ = 0x80 | ((wc >> 6) & 0x3f);
		*output++ = 0x80 | (wc & 0x3f);
	}
	else if (wc <= 0x1fffff)
	{
		if (outsize < 4)
			return NULL;
		*output++ = 0xf0 | (wc >> 18);
		*output++ = 0x80 | ((wc >> 12) & 0x3f);
		*output++ = 0x80 | ((wc >> 6) & 0x3f);
		*output++ = 0x80 | (wc & 0x3f);
	}
	else if (wc <= 0x3ffffff)
	{
		if (outsize < 5)
			return NULL;
		*output++ = 0xf8 | (wc >> 24);
		*output++ = 0x80 | ((wc >> 18) & 0x3f);
		*output++ = 0x80 | ((wc >> 12) & 0x3f);
		*output++ = 0x80 | ((wc >> 6) & 0x3f);
		*output++ = 0x80 | (wc & 0x3f);
	}
	else if (wc <= 0x7fffffff)
	{
		if (outsize < 6)
			return NULL;
		*output++ = 0xfc | (wc >> 30);
		*output++ = 0x80 | ((wc >> 24) & 0x3f);
		*output++ = 0x80 | ((wc >> 18) & 0x3f);
		*output++ = 0x80 | ((wc >> 12) & 0x3f);
		*output++ = 0x80 | ((wc >> 6) & 0x3f);
		*output++ = 0x80 | (wc & 0x3f);
	}
	else
		return NULL;

	return output;
}

static const le16_t* utf16_to_wchar(const le16_t* input, wchar_t* wc, size_t insize)
{
	if ((le16_to_cpu(input[0]) & 0xfc00) == 0xd800)
	{
		if (insize < 2 || (le16_to_cpu(input[1]) & 0xfc00) != 0xdc00)
			return NULL;
		*wc = ((wchar_t) (le16_to_cpu(input[0]) & 0x3ff) << 10);
		*wc |= (le16_to_cpu(input[1]) & 0x3ff);
		*wc += 0x10000;
		return input + 2;
	}
	else
	{
		*wc = le16_to_cpu(*input);
		return input + 1;
	}
}

int utf16_to_utf8(char* output, const le16_t* input, size_t outsize, size_t insize)
{
	const le16_t* inp = input;
	char* outp = output;
	wchar_t wc;

	while (inp - input < insize && le16_to_cpu(*inp))
	{
		inp = utf16_to_wchar(inp, &wc, insize - (inp - input));
		if (inp == NULL)
		{
			exfat_error("illegal UTF-16 sequence");
			return -ERROR_BAD_NUMBER; //EILSEQ;
		}
		outp = wchar_to_utf8(outp, wc, outsize - (outp - output));
		if (outp == NULL)
		{
			exfat_error("name is too long");
			return -ERROR_LINE_TOO_LONG; //ENAMETOOLONG;
		}
	}
	*outp = '\0';
	return 0;
}

static const char* utf8_to_wchar(const char* input, wchar_t* wc, size_t insize)
{
	if ((input[0] & 0x80) == 0 && insize >= 1)
	{
		*wc = (wchar_t) input[0];
		return input + 1;
	}
	if ((input[0] & 0xe0) == 0xc0 && insize >= 2)
	{
		*wc = (((wchar_t) input[0] & 0x1f) << 6) |
		((wchar_t) input[1] & 0x3f);
		return input + 2;
	}
	if ((input[0] & 0xf0) == 0xe0 && insize >= 3)
	{
		*wc = (((wchar_t) input[0] & 0x0f) << 12) |
		(((wchar_t) input[1] & 0x3f) << 6) |
		((wchar_t) input[2] & 0x3f);
		return input + 3;
	}
	if ((input[0] & 0xf8) == 0xf0 && insize >= 4)
	{
		*wc = (((wchar_t) input[0] & 0x07) << 18) |
		(((wchar_t) input[1] & 0x3f) << 12) |
		(((wchar_t) input[2] & 0x3f) << 6) |
		((wchar_t) input[3] & 0x3f);
		return input + 4;
	}
	if ((input[0] & 0xfc) == 0xf8 && insize >= 5)
	{
		*wc = (((wchar_t) input[0] & 0x03) << 24) |
		(((wchar_t) input[1] & 0x3f) << 18) |
		(((wchar_t) input[2] & 0x3f) << 12) |
		(((wchar_t) input[3] & 0x3f) << 6) |
		((wchar_t) input[4] & 0x3f);
		return input + 5;
	}
	if ((input[0] & 0xfe) == 0xfc && insize >= 6)
	{
		*wc = (((wchar_t) input[0] & 0x01) << 30) |
		(((wchar_t) input[1] & 0x3f) << 24) |
		(((wchar_t) input[2] & 0x3f) << 18) |
		(((wchar_t) input[3] & 0x3f) << 12) |
		(((wchar_t) input[4] & 0x3f) << 6) |
		((wchar_t) input[5] & 0x3f);
		return input + 6;
	}
	return NULL;
}

static le16_t* wchar_to_utf16(le16_t* output, wchar_t wc, size_t outsize)
{
	if (wc <= 0xffff) /* if character is from BMP */
	{
		if (outsize == 0)
			return NULL;
		output[0] = cpu_to_le16(wc);
		return output + 1;
	}
	if (outsize < 2)
		return NULL;
	wc -= 0x10000;
	output[0] = cpu_to_le16(0xd800 | ((wc >> 10) & 0x3ff));
	output[1] = cpu_to_le16(0xdc00 | (wc & 0x3ff));
	return output + 2;
}

int utf8_to_utf16(le16_t* output, const char* input, size_t outsize, size_t insize)
{
	const char* inp = input;
	le16_t* outp = output;
	wchar_t wc;

	while (inp - input < insize && *inp)
	{
		inp = utf8_to_wchar(inp, &wc, insize - (inp - input));
		if (inp == NULL)
		{
			exfat_error("illegal UTF-8 sequence '%s' (%ld)", input, (LONG)strlen(input));
			return -ERROR_BAD_NUMBER; //EILSEQ;
		}
		outp = wchar_to_utf16(outp, wc, outsize - (outp - output));
		if (outp == NULL)
		{
			exfat_error("name is too long");
			return -ERROR_LINE_TOO_LONG; //ENAMETOOLONG;
		}
	}
	*outp = cpu_to_le16(0);
	return 0;
}

size_t utf16_length(const le16_t* str)
{
	size_t i = 0;

	while (le16_to_cpu(str[i]))
		i++;
	return i;
}

/*------------------------------------------------------------------------*/

static struct {
	UBYTE iso; //ISO-8859-1
	UWORD utf; //UTF-8
	UWORD uni; //Unicode
} table[] = {
	{ 0x80, 0xC280, 0x0080 },
	{ 0x81, 0xC281, 0x0081 },
	{ 0x82, 0xC282, 0x0082 },
	{ 0x83, 0xC283, 0x0083 },
	{ 0x84, 0xC284, 0x0084 },
	{ 0x85, 0xC285, 0x0085 },
	{ 0x86, 0xC286, 0x0086 },
	{ 0x87, 0xC287, 0x0087 },
	{ 0x88, 0xC288, 0x0088 },
	{ 0x89, 0xC289, 0x0089 },
	{ 0x8A, 0xC28A, 0x008A },
	{ 0x8B, 0xC28B, 0x008B },
	{ 0x8C, 0xC28C, 0x008C },
	{ 0x8D, 0xC28D, 0x008D },
	{ 0x8E, 0xC28E, 0x008E },
	{ 0x8F, 0xC28F, 0x008F },
	{ 0x90, 0xC290, 0x0090 },
	{ 0x91, 0xC291, 0x0091 },
	{ 0x92, 0xC292, 0x0092 },
	{ 0x93, 0xC293, 0x0093 },
	{ 0x94, 0xC294, 0x0094 },
	{ 0x95, 0xC295, 0x0095 },
	{ 0x96, 0xC296, 0x0096 },
	{ 0x97, 0xC297, 0x0097 },
	{ 0x98, 0xC298, 0x0098 },
	{ 0x99, 0xC299, 0x0099 },
	{ 0x9A, 0xC29A, 0x009A },
	{ 0x9B, 0xC29B, 0x009B },
	{ 0x9C, 0xC29C, 0x009C },
	{ 0x9D, 0xC29D, 0x009D },
	{ 0x9E, 0xC29E, 0x009E },
	{ 0x9F, 0xC29F, 0x009F },
	{ 0xA0, 0xC2A0, 0x00A0 },
	{ 0xA1, 0xC2A1, 0x00A1 },
	{ 0xA2, 0xC2A2, 0x00A2 },
	{ 0xA3, 0xC2A3, 0x00A3 },
	{ 0xA4, 0xC2A4, 0x00A4 },
	{ 0xA5, 0xC2A5, 0x00A5 },
	{ 0xA6, 0xC2A6, 0x00A6 },
	{ 0xA7, 0xC2A7, 0x00A7 },
	{ 0xA8, 0xC2A8, 0x00A8 },
	{ 0xA9, 0xC2A9, 0x00A9 },
	{ 0xAA, 0xC2AA, 0x00AA },
	{ 0xAB, 0xC2AB, 0x00AB },
	{ 0xAC, 0xC2AC, 0x00AC },
	{ 0xAD, 0xC2AD, 0x00AD },
	{ 0xAE, 0xC2AE, 0x00AE },
	{ 0xAF, 0xC2AF, 0x00AF },
	{ 0xB0, 0xC2B0, 0x00B0 },
	{ 0xB1, 0xC2B1, 0x00B1 },
	{ 0xB2, 0xC2B2, 0x00B2 },
	{ 0xB3, 0xC2B3, 0x00B3 },
	{ 0xB4, 0xC2B4, 0x00B4 },
	{ 0xB5, 0xC2B5, 0x00B5 },
	{ 0xB6, 0xC2B6, 0x00B6 },
	{ 0xB7, 0xC2B7, 0x00B7 },
	{ 0xB8, 0xC2B8, 0x00B8 },
	{ 0xB9, 0xC2B9, 0x00B9 },
	{ 0xBA, 0xC2BA, 0x00BA },
	{ 0xBB, 0xC2BB, 0x00BB },
	{ 0xBC, 0xC2BC, 0x00BC },
	{ 0xBD, 0xC2BD, 0x00BD },
	{ 0xBE, 0xC2BE, 0x00BE },
	{ 0xBF, 0xC2BF, 0x00BF },
	{ 0xC0, 0xC380, 0x00C0 },
	{ 0xC1, 0xC381, 0x00C1 },
	{ 0xC2, 0xC382, 0x00C2 },
	{ 0xC3, 0xC383, 0x00C3 },
	{ 0xC4, 0xC384, 0x00C4 },
	{ 0xC5, 0xC385, 0x00C5 },
	{ 0xC6, 0xC386, 0x00C6 },
	{ 0xC7, 0xC387, 0x00C7 },
	{ 0xC8, 0xC388, 0x00C8 },
	{ 0xC9, 0xC389, 0x00C9 },
	{ 0xCA, 0xC38A, 0x00CA },
	{ 0xCB, 0xC38B, 0x00CB },
	{ 0xCC, 0xC38C, 0x00CC },
	{ 0xCD, 0xC38D, 0x00CD },
	{ 0xCE, 0xC38E, 0x00CE },
	{ 0xCF, 0xC38F, 0x00CF },
	{ 0xD0, 0xC390, 0x00D0 },
	{ 0xD1, 0xC391, 0x00D1 },
	{ 0xD2, 0xC392, 0x00D2 },
	{ 0xD3, 0xC393, 0x00D3 },
	{ 0xD4, 0xC394, 0x00D4 },
	{ 0xD5, 0xC395, 0x00D5 },
	{ 0xD6, 0xC396, 0x00D6 },
	{ 0xD7, 0xC397, 0x00D7 },
	{ 0xD8, 0xC398, 0x00D8 },
	{ 0xD9, 0xC399, 0x00D9 },
	{ 0xDA, 0xC39A, 0x00DA },
	{ 0xDB, 0xC39B, 0x00DB },
	{ 0xDC, 0xC39C, 0x00DC },
	{ 0xDD, 0xC39D, 0x00DD },
	{ 0xDE, 0xC39E, 0x00DE },
	{ 0xDF, 0xC39F, 0x00DF },
	{ 0xE0, 0xC3A0, 0x00E0 },
	{ 0xE1, 0xC3A1, 0x00E1 },
	{ 0xE2, 0xC3A2, 0x00E2 },
	{ 0xE3, 0xC3A3, 0x00E3 },
	{ 0xE4, 0xC3A4, 0x00E4 },
	{ 0xE5, 0xC3A5, 0x00E5 },
	{ 0xE6, 0xC3A6, 0x00E6 },
	{ 0xE7, 0xC3A7, 0x00E7 },
	{ 0xE8, 0xC3A8, 0x00E8 },
	{ 0xE9, 0xC3A9, 0x00E9 },
	{ 0xEA, 0xC3AA, 0x00EA },
	{ 0xEB, 0xC3AB, 0x00EB },
	{ 0xEC, 0xC3AC, 0x00EC },
	{ 0xED, 0xC3AD, 0x00ED },
	{ 0xEE, 0xC3AE, 0x00EE },
	{ 0xEF, 0xC3AF, 0x00EF },
	{ 0xF0, 0xC3B0, 0x00F0 },
	{ 0xF1, 0xC3B1, 0x00F1 },
	{ 0xF2, 0xC3B2, 0x00F2 },
	{ 0xF3, 0xC3B3, 0x00F3 },
	{ 0xF4, 0xC3B4, 0x00F4 },
	{ 0xF5, 0xC3B5, 0x00F5 },
	{ 0xF6, 0xC3B6, 0x00F6 },
	{ 0xF7, 0xC3B7, 0x00F7 },
	{ 0xF8, 0xC3B8, 0x00F8 },
	{ 0xF9, 0xC3B9, 0x00F9 },
	{ 0xFA, 0xC3BA, 0x00FA },
	{ 0xFB, 0xC3BB, 0x00FB },
	{ 0xFC, 0xC3BC, 0x00FC },
	{ 0xFD, 0xC3BD, 0x00FD },
	{ 0xFE, 0xC3BE, 0x00FE },
	{ 0xFF, 0xC3BF, 0x00FF },
	{ 0, 0, 0 }
};

static UBYTE utf2iso(UWORD c)
{
	int i = 0;

	while (table[i].iso) {
		if (table[i].utf == c)
			return table[i].iso;

		i++;
	}
	return 0;
}

static UWORD iso2utf(UBYTE c)
{
	int i = 0;

	while (table[i].iso) {
		if (table[i].iso == c)
			return table[i].utf;

		i++;
	}
	return 0;
}

int utf8_to_iso8859(char *output, const char *input, size_t insize)
{
	UBYTE *in = (UBYTE *)input;
	UBYTE *out = (UBYTE *)output;
	LONG len = (LONG)insize;
	UBYTE c;

	while (len > 0) {
		c = *in++;

		if (c < 128)
			*out++ = c;
		else {
			UWORD cc = ((UWORD)c << 8) | (UWORD)(*in++);
			len--;

			c = utf2iso(cc);
			if (c != 0)
				*out++ = c;
			else {
				exfat_error("illegal UTF-8 sequence, unknown char 0x%02x == %d", c, c);
				return -ERROR_BAD_NUMBER;
			}
		} 
		len--;
	}
	*out++ = '\0';
	return 0;
}

int iso8859_to_utf8(char *output, const char *input, size_t insize)
{
	UBYTE *in = (UBYTE *)input;
	UBYTE *out = (UBYTE *)output;
	LONG len = (LONG)insize;
	UBYTE c;

	while (len > 0) {
		c = *in++;

		if (c < 128)
			*out++ = c;
		else {
			UWORD cc = iso2utf(c);

			if (cc != 0) {
				*out++ = (UBYTE)(cc >> 8);
				*out++ = (UBYTE)(cc & 0xff);
			} else {
				exfat_error("illegal ISO sequence, unknown char 0x%04x == %d", cc, cc);
				return -ERROR_BAD_NUMBER;
			}
		}
		len--;
	}
	*out++ = '\0';
	return 0;
}

