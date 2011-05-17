/*********************************************************************
 *                     Mihrab: Muslim Prayer Times
 * strings.c
 *
 * Copyright (C) 2011  Ahmad Khayyat
 *
 * This file is part of Mihrab.
 *
 * Mihrab is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#include "common.h"

static char *time_format_12_str;
static char *time_format_24_str;
static char *period_format_hms_str;
static char *period_format_ms_str;
static char *period_format_s_str;

static char *prayer_names_str[6];
static char *hijri_months_str[12];
static char *weekday_names_str[7];
static char *greg_months_str[12];

void init_strings(void)
{
    time_format_12_str = "%l:%M:%S %p";
    time_format_24_str = "%H:%M:%S";

    period_format_hms_str = _("%dh %dm %ds");
    period_format_ms_str = _("%dm %ds");
    period_format_s_str = _("%ds");

    prayer_names_str[0] = _("Fajr");
    prayer_names_str[1] = _("Shorooq");
    prayer_names_str[2] = _("Dhuhr");
    prayer_names_str[3] = _("Asr");
    prayer_names_str[4] = _("Maghrib");
    prayer_names_str[5] = _("Ishaa");
    prayer_names_str[6] = _("Fajr");

    hijri_months_str[0] = _("Muharram");
    hijri_months_str[1] = _("Safar");
    hijri_months_str[2] = _("Rabee Alawwal");
    hijri_months_str[3] = _("Rabee Althani");
    hijri_months_str[4] = _("Jumada Alawwal");
    hijri_months_str[5] = _("Jumada Althani");
    hijri_months_str[6] = _("Rajab");
    hijri_months_str[7] = _("Shaaban");
    hijri_months_str[8] = _("Ramadhan");
    hijri_months_str[9] = _("Shawwal");
    hijri_months_str[10] = _("Thul Qidah");
    hijri_months_str[11] = _("Thul Hijjah");

    /* I am including the following two sets of strings (weekday names
     * and gregorian month names) due to problems with strftime()
     * localization */

    weekday_names_str[0] = _("Sunday");
    weekday_names_str[1] = _("Monday");
    weekday_names_str[2] = _("Tuesday");
    weekday_names_str[3] = _("Wednesday");
    weekday_names_str[4] = _("Thursday");
    weekday_names_str[5] = _("Friday");
    weekday_names_str[6] = _("Saturday");

    greg_months_str[0] = _("January");
    greg_months_str[1] = _("February");
    greg_months_str[2] = _("March");
    greg_months_str[3] = _("April");
    greg_months_str[4] = _("May");
    greg_months_str[5] = _("June");
    greg_months_str[6] = _("July");
    greg_months_str[7] = _("August");
    greg_months_str[8] = _("September");
    greg_months_str[9] = _("October");
    greg_months_str[10] = _("November");
    greg_months_str[11] = _("December");
}

char *time_format(gboolean is24)
{
    if (is24)
        return time_format_24_str;
    else
        return time_format_12_str;
}

char *period_format_hms(void)
{
    return period_format_hms_str;
}

char *period_format_ms(void)
{
    return period_format_ms_str;
}

char *period_format_s(void)
{
    return period_format_s_str;
}

char *prayer_names(int i)
{
    return prayer_names_str[i];
}

char *hijri_months(int i)
{
    return hijri_months_str[i];
}

char *weekday_names(int i)
{
    return weekday_names_str[i];
}

char *greg_months(int i)
{
    return greg_months_str[i];
}
