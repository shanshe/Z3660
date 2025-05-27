/*--------------------------------------------------------------------------

  exFAT filesystem for MorphOS:
  Copyright � 2014-2015 Rupert Hausberger

  FAT12/16/32 filesystem handler:
  Copyright � 2006 Marek Szyprowski
  Copyright � 2007-2008 The AROS Development Team

  exFAT implementation library:
  Copyright � 2010-2013 Andrew Nayenko


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

#include <exec/types.h>

#include <math.h>
#include <stdarg.h>

#include "string.h"

/*------------------------------------------------------------------------*/

#define MAX_PRECI 0xfffffffful
#define DBL_DIG 100
#define MIN_FLT (DBL_DIG + 1)
#define MIN_INT (sizeof(unsigned long long) * 8 / 3 + 1)
#define MIN_PTR (sizeof(void *) * 8 / 4 + 1)

#define SIZE_BUFFER (MIN_INT > MIN_PTR ? \
	(MIN_INT > MIN_FLT ? MIN_INT : MIN_FLT): \
	(MIN_PTR > MIN_FLT ? MIN_PTR : MIN_FLT))

#define F_ALT		1  /* '#' is set */
#define F_ZERO		2  /* '0' is set */
#define F_ALIGN	4  /* '-' is set */
#define F_SPACE	8  /* ' ' is set */
#define F_SIGN		16 /* '+' is set */

#define EOF -1
#define OUT(c) \
	do { \
		if ((*outc)((c),data) == EOF) \
			return outcount; \
		outcount++; \
	} while (0)

const char *const __decimalpoint = ".";

/*------------------------------------------------------------------------*/

#ifdef __MORPHOS__
	#pragma pack(2)
#endif

typedef union {
	double value;
	struct {
		unsigned long msw;
		unsigned long lsw;
	} parts;
} ieee_double_shape_type;

#ifdef __MORPHOS__
	#pragma pack()
#endif

#define GET_HIGH_WORD(i,d) \
	do { \
		ieee_double_shape_type gh_u; \
		gh_u.value = (d);	\
		(i) = gh_u.parts.msw; \
	} while (0)

#define GET_LOW_WORD(i,d) \
	do { \
		ieee_double_shape_type gl_u; \
		gl_u.value = (d); \
		(i) = gl_u.parts.lsw; \
	} while (0)

static long _isinf(double val)
{
	long hx, lx;

	GET_HIGH_WORD(hx, val);
	GET_LOW_WORD(lx, val);
	hx &= 0x7fffffff;
	hx ^= 0x7ff00000;
	hx |= lx;

	return (hx == 0);
}

/*------------------------------------------------------------------------*/

#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define tolower(c) ((c) - 'A' + 'a')
#define toupper(c) ((c) - 'a' + 'A')

/*static unsigned long strlen(const char *s)
{
	const char *ptr;

	for (ptr = s; *ptr; ++ptr);
	return ((unsigned long)(ptr - s));
}*/

/*------------------------------------------------------------------------*/

static long vcformat(void *data, long (*outc)(long, void *), const char *format, va_list args)
{
	unsigned long outcount = 0;

	while (*format)
	{
		if (*format == '%')
		{
			static const char flagc[] = {'#','0','-',' ','+'};
			static const char lowertable[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
			static const char uppertable[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
			const char *ptr = (char *)format + 1;

			unsigned long width = 0, flags = 0, preci = MAX_PRECI;
			char type, subtype = 'i', subtype2 = '\0';

			char buffer1[2];
			char buffer[SIZE_BUFFER];
			char *buffer2 = buffer;

			unsigned long size1 = 0, size2 = 0;
			unsigned long i, pad;

			do {
				for (i = 0; i < sizeof(flagc); i++) {
					if (flagc[i] == *ptr) {
						flags |= 1 << i;
						ptr++;
						break;
					}
				}
			} while (i < sizeof(flagc));

			if (*ptr == '*') {
				signed int a;

				ptr++;
				a = va_arg(args, signed int);
				if (a < 0) {
					flags |= F_ALIGN;
					width = -a;
				} else
					width = a;
			} else {
				while (isdigit(*ptr))
					width = width * 10 + (*ptr++ - '0');
			}
			if (*ptr == '.') {
				ptr++;
				if (*ptr == '*')
				{
					signed int a;

					ptr++;
					a = va_arg(args, signed int);
					if (a >= 0)
						preci = a;
				} else {
					preci = 0;
					while (isdigit(*ptr))
						preci = preci * 10 + (*ptr++ - '0');
				}
			}

			if (*ptr == 'h' || *ptr == 'l' || *ptr == 'L') subtype = *ptr++;
			if (*ptr == 'h' || *ptr == 'l' || *ptr == 'L') subtype2 = *ptr++;
			type = *ptr++;
			switch (type)
			{
				case 'd':
				case 'i':
				case 'o':
				case 'p':
				case 'u':
				case 'x':
				case 'X':
				{
					unsigned long long v;
					const char *table;
					int base;

					if (type == 'p') {
						subtype = 'l';
						type = 'x';
						flags |= F_ALT;
					}
					if (type == 'd' || type == 'i') {
						signed long long v2;

						if (subtype2 == 'l')
							v2 = va_arg(args, signed long long);
						else if (subtype=='l')
							v2 = va_arg(args, signed long);
						else
							v2 = va_arg(args, signed int);

						if (v2 < 0) {
							buffer1[size1++] = '-';
							v = -v2;
						} else {
							if (flags & F_SIGN)
								buffer1[size1++] = '+';
							else if (flags & F_SPACE)
								buffer1[size1++] = ' ';

							v = v2;
						}
					} else {
						if (subtype2 == 'l')
							v = va_arg(args, unsigned long long);
						else if (subtype == 'l')
							v = va_arg(args, unsigned long);
						else
							v = va_arg(args, unsigned int);

						if (flags & F_ALT) {
							if (type == 'o' && (preci && v))
								buffer1[size1++] = '0';
							if ((type == 'x' || type == 'X') && v)
							{
								buffer1[size1++] = '0';
								buffer1[size1++] = type;
							}
						}
					}
					buffer2 = &buffer[sizeof(buffer)];
					base = type == 'x' || type == 'X' ? 16 : (type == 'o' ? 8 : 10);
					table = type != 'X' ? lowertable : uppertable;
					do {
						*--buffer2 = table[v % base];
						v = v / base;
						size2++;
					} while (v);

					if (preci == MAX_PRECI)
						preci = 0;
					else
						flags &= ~F_ZERO;

					break;
				}
				case 'c':
				{
					if (subtype == 'l')
						*buffer2 = va_arg(args, long);
					else
						*buffer2 = va_arg(args, int);

					size2 = 1;
					preci = 0;
					break;
				}
				case 's':
				{
					buffer2 = va_arg(args,char *);
					if (!buffer2)
						buffer2 = "(null)";

					size2 = strlen((const char *)buffer2);
					size2 = size2 <= preci ? size2 : preci;
					preci = 0;
					break;
				}
				case 'f':
				case 'e':
				case 'E':
				case 'g':
				case 'G':
				{
					double v;
					char killzeros = 0, sign = 0;
					int ex1, ex2;
					unsigned long size, dnum, dreq;
					char *udstr = NULL;

					v = va_arg(args, double);
					if (_isinf(v)) {
						if (v > 0)
							udstr = "+inf";
						else
							udstr = "-inf";
					} else if (isnan(v))
						udstr = "NaN";

					if (udstr != NULL) {
						size2 = strlen((const char *)udstr);
						preci = 0;
						buffer2 = udstr;
						break;
					}
					if (preci == MAX_PRECI)
						preci = 6;

					if (v < 0.0) {
						sign = '-';
						v = -v;
					} else {
						if (flags & F_SIGN)
							sign = '+';
						else if (flags & F_SPACE)
							sign = ' ';
					}
					ex1 = 0;
					if (v != 0.0) {
						ex1 = log10(v);
						if (v < 1.0)
							v = v * pow(10,- --ex1);
						else
							v = v / pow(10, ex1);

						if (v < 1.0) {
							v *= 10.0;
							ex1--;
						}
					}
					ex2 = preci;
					if (type == 'f')
						ex2 += ex1;
					if (tolower(type) == 'g')
						ex2--;

					v += .5 / pow(10, ex2 < MIN_FLT ? ex2 : MIN_FLT);
					if (v >= 10.0) {
						v /= 10.0;
						ex1++;
					}
					if (tolower(type) == 'g') {
						if (ex1 < (signed long)preci && ex1 >= -4) {
							type = 'f';
							preci -= ex1;
						} else
							type = type == 'g' ? 'e' : 'E';

						preci--;
						if (!(flags & F_ALT))
							killzeros = 1;
					}
					dreq = preci + 1;
					if (type == 'f')
						dreq += ex1;

					dnum = 0;
					while (dnum < dreq && dnum < MIN_FLT) {
						buffer[dnum++] = (char)v + '0';
						v = (v - (double)(char)v) * 10.0;
					}
					if (killzeros) {
						while (preci && (dreq-- > dnum || buffer[dreq] == '0'))
							preci--;
					}
					if (type == 'f') {
						size = preci + 1;
						if (ex1 > 0)
							size += ex1;
						if (preci || flags & F_ALT)
							size++;
					} else {
						size = preci + 5;
						if (preci || flags & F_ALT)
							size++;
						if (ex1 > 99 || ex1 < -99)
							size++;
					}
					pad = size + (sign != 0);
					pad = pad >= width ? 0 : width - pad;
					if (sign && flags & F_ZERO)
						OUT(sign);

					if (!(flags & F_ALIGN)) {
						for (i = 0; i < pad; i++)
							OUT(flags & F_ZERO ? '0' : ' ');
					}
					if (sign && !(flags & F_ZERO))
						OUT(sign);

					dreq = 0;
					if (type == 'f')
					{
						if (ex1 < 0)
							OUT('0');
						else {
							while (ex1 >= 0) {
								OUT(dreq < dnum ? buffer[dreq++] : '0');
								ex1--;
							}
						}
						if (preci || flags & F_ALT) {
							OUT(__decimalpoint[0]);
							while (preci--) {
								if (++ex1 < 0)
									OUT('0');
								else
									OUT(dreq < dnum ? buffer[dreq++] : '0');
							}
						}
					} else {
						OUT(buffer[dreq++]);
						if (preci || flags & F_ALT) {
							OUT(__decimalpoint[0]);
							while (preci--)
								OUT(dreq < dnum ? buffer[dreq++] : '0');
						}
						OUT(type);
						if (ex1<0) {
							OUT('-');
							ex1 = -ex1;
						} else
							OUT('+');

						if (ex1 > 99)
							OUT(ex1 / 100 + '0');

						OUT(ex1 / 10 % 10 + '0');
						OUT(ex1 % 10 + '0');
					}
					if (flags & F_ALIGN) {
						for (i = 0; i < pad; i++)
							OUT(' ');
					}
					width = preci = 0;
					break;
				}
				case '%':
				{
					buffer2 = "%";
					size2 = 1;
					preci = 0;
					break;
				}
				case 'n':
				{
					*va_arg(args, int *) = outcount;
					width = preci = 0;
					break;
				}
				default:
				{
					if (!type)
						ptr--;

					buffer2 = (char *)format;
					size2 = ptr - (char *)format;
					width = preci = 0;
					break;
				}
			}

			pad = size1 + (size2 >= preci ? size2 : preci);
			pad = pad >= width ? 0 : width - pad;
			if (flags & F_ZERO) {
				for (i = 0; i < size1; i++)
					OUT(buffer1[i]);
			}
			if (!(flags & F_ALIGN)) {
				for (i = 0; i < pad; i++)
					OUT(flags & F_ZERO ? '0' : ' ');
			}
			if (!(flags & F_ZERO)) {
				for (i = 0; i < size1; i++)
					OUT(buffer1[i]);
			}
			for (i = size2; i < preci; i++) {
				OUT('0');
			}
			for (i = 0; i < size2; i++) {
				OUT(buffer2[i]);
			}
			if (flags & F_ALIGN) {
				for (i = 0; i < pad; i++)
					OUT(' ');
			}
			format = (char *)ptr;
		} else
			OUT(*format++);
	}
	return (long)outcount;
}

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

static long vscountf_uc(long c, void *str)
{
	return 1;
}

int vscountf(const char *format, va_list args)
{
	return vcformat(NULL, (APTR)vscountf_uc, (const char *)format, args);
}

int scountf(const char *format, ...)
{
	int size;
	va_list args;

	va_start(args, format);
	size = vscountf(format, args);
	va_end(args);

	return size;
}

/*------------------------------------------------------------------------*/

static long vsprintf_uc(long c, char **str)
{
	*(*str)++ = c;
	return 1;
}

int vsprintf(char *str, const char *format, va_list args)
{
	int size;

	size = vcformat(&str, (APTR)vsprintf_uc, format, args);
	*str = 0;

	return size;
}

int sprintf(char *str, const char *format, ...)
{
	int size;
	va_list args;

	va_start(args, format);
	size = vsprintf(str, format, args);
	va_end(args);

	return size;
}

/*------------------------------------------------------------------------*/

#ifdef __MORPHOS__
	#pragma pack(2)
#endif

struct data {
	char *str;
	unsigned int n;
};

#ifdef __MORPHOS__
	#pragma pack(2)
#endif

static long vsnprintf_uc(long c, struct data *data)
{
	if (data->n) {
		*(data->str)++ = c;
		data->n--;
		return 1;
	}
	return -1;
}

int vsnprintf(char *str, unsigned int n, const char *format, va_list args)
{
	struct data data = { str, n };

	vcformat(&data, (APTR)vsnprintf_uc, format, args);

	if (data.n) {
		*(data.str) = 0;
		return (n - data.n);
	}
	return -1;
}

int snprintf(char *str, unsigned int n, const char *format, ...)
{
	int size;
	va_list args;

	va_start(args, format);
	size = vsnprintf(str, n, format, args);
	va_end(args);

	return size;
}

/*------------------------------------------------------------------------*/

