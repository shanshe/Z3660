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
#include <time.h>

long exfat_timezone;

#define SEC_IN_MIN 60ll
#define SEC_IN_HOUR (60 * SEC_IN_MIN)
#define SEC_IN_DAY (24 * SEC_IN_HOUR)
#define SEC_IN_YEAR (365 * SEC_IN_DAY) /* not leap year */

#define UNIX_EPOCH_YEAR  1970 /*  Unix epoch started at 0:00:00 UTC 1 January 1970 */
#define AMIGA_EPOCH_YEAR 1978 /* Amiga epoch started at 0:00:00 UTC 1 January 1978 */
#define EXFAT_EPOCH_YEAR 1980 /* exFAT epoch started at 0:00:00 UTC 1 January 1980 */

#define EPOCH_DIFF_YEAR (EXFAT_EPOCH_YEAR - AMIGA_EPOCH_YEAR)
#define EPOCH_DIFF_DAYS (EPOCH_DIFF_YEAR * 365 + EPOCH_DIFF_YEAR / 4)
#define EPOCH_DIFF_SEC (EPOCH_DIFF_DAYS * SEC_IN_DAY)

#define LEAP_YEARS(year) ((EXFAT_EPOCH_YEAR + (year) - 1) / 4 - (EXFAT_EPOCH_YEAR - 1) / 4)
#define IS_LEAP_YEAR(year) ((EXFAT_EPOCH_YEAR + (year)) % 4 == 0)

static const time_t days_in_year[] = {
	/* Jan  Feb  Mar  Apr  May  Jun  Jul  Aug  Sep  Oct  Nov  Dec */
	0,   0,  31,  59,  90, 120, 151, 181, 212, 243, 273, 304, 334
};

/*------------------------------------------------------------------------*/

void exfat_tzset(void)
{
	struct Library *LocaleBase;

	exfat_timezone = 0l;

	if ((LocaleBase = OpenLibrary("locale.library", 37))) {
		struct Locale *loc = OpenLocale(NULL);

		if (loc) {
			exfat_timezone = loc->loc_GMTOffset * SEC_IN_MIN;
			CloseLocale(loc);
		}
		CloseLibrary(LocaleBase);
	}
}

/*------------------------------------------------------------------------*/

void exfat_copy_ds(struct DateStamp *s, struct DateStamp *d)
{
	d->ds_Days = s->ds_Days;
	d->ds_Minute = s->ds_Minute;
	d->ds_Tick = s->ds_Tick;
}

/*------------------------------------------------------------------------*/

static ULONG exfat_exfat2amiga_secs(le16_t date, le16_t time, UBYTE centisec)
{
	ULONG s;
	UWORD ndate = le16_to_cpu(date);
	UWORD ntime = le16_to_cpu(time);

	UWORD day    = ndate & 0x1f;
	UWORD month  = ndate >> 5 & 0xf;
	UWORD year   = ndate >> 9;
	UWORD twosec = ntime & 0x1f;
	UWORD min    = ntime >> 5 & 0x3f;
	UWORD hour   = ntime >> 11;

	if (day == 0 || month == 0 || month > 12) {
		exfat_error("bad date %u-%02u-%02u", year + EXFAT_EPOCH_YEAR, month, day);
		return 0;
	}
	if (hour > 23 || min > 59 || twosec > 29) {
		exfat_error("bad time %u:%02u:%02u", hour, min, twosec * 2);
		return 0;
	}
	if (centisec > 199) {
		exfat_error("bad centiseconds count %u", centisec);
		return 0;
	}

	s = EPOCH_DIFF_SEC;
	s += year * SEC_IN_YEAR + LEAP_YEARS(year) * SEC_IN_DAY;
	s += days_in_year[month] * SEC_IN_DAY;
	if ((EXFAT_EPOCH_YEAR + year) % 4 == 0 && month > 2)
		s += SEC_IN_DAY;
	s += (day - 1) * SEC_IN_DAY;

	s += hour * SEC_IN_HOUR;
	s += min * SEC_IN_MIN;
	s += twosec * 2;
	s += centisec / 100;

	s += exfat_timezone;
	return s;
}

void exfat_exfat2amiga(le16_t date, le16_t time, UBYTE centisec, struct DateStamp *ds)
{
	ULONG days, secs;
	ULONG s = exfat_exfat2amiga_secs(date, time, centisec);

	s -= exfat_timezone;

	days = (ULONG)s / SEC_IN_DAY;
	secs = (ULONG)s % SEC_IN_DAY;

	ds->ds_Days = (LONG)days;
	ds->ds_Minute = (LONG)(secs / 60);
	ds->ds_Tick = (LONG)((secs % 60) * TICKS_PER_SECOND);
}

/*------------------------------------------------------------------------*/

static void exfat_amiga2exfat_secs(ULONG s, le16_t *date, le16_t *time, UBYTE *centisec)
{
	ULONG shift = EPOCH_DIFF_SEC + exfat_timezone;
	UWORD day, month, year;
	UWORD twosec, min, hour;
	int days;
	int i;

	if (s < shift)
		s = shift;

	s -= shift;

	days = s / SEC_IN_DAY;
	year = (4 * days) / (4 * 365 + 1);
	days -= year * 365 + LEAP_YEARS(year);
	month = 0;
	for (i = 1; i <= 12; i++) {
		int leap_day = (IS_LEAP_YEAR(year) && i == 2);
		int leap_sub = (IS_LEAP_YEAR(year) && i >= 3);

		if (i == 12 || days - leap_sub < days_in_year[i + 1] + leap_day) {
			month = i;
			days -= days_in_year[i] + leap_sub;
			break;
		}
	}
	day = days + 1;

	hour = (s % SEC_IN_DAY) / SEC_IN_HOUR;
	min = (s % SEC_IN_HOUR) / SEC_IN_MIN;
	twosec = (s % SEC_IN_MIN) / 2;

	*date = cpu_to_le16(day | (month << 5) | (year << 9));
	*time = cpu_to_le16(twosec | (min << 5) | (hour << 11));
	if (centisec)
		*centisec = (s % 2) * 100;
}

void exfat_amiga2exfat(struct DateStamp *ds, le16_t *date, le16_t *time, UBYTE *centisec)
{
	ULONG s = ds->ds_Days * 60 * 60 * 24 + ds->ds_Minute * 60 + ds->ds_Tick / TICKS_PER_SECOND;

	s += exfat_timezone;

	exfat_amiga2exfat_secs(s, date, time, centisec);
}

