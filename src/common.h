/*********************************************************************
 *                     Mihrab: Muslim Prayer Times
 * common.h
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

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <glib.h>

#define _(string) gettext(string)

#define TIME_STR_LEN 500
#define DATE_STR_LEN 500

typedef struct
{
    /* when this event happens (time of day) */
    char time[TIME_STR_LEN];
    /* time left till the next event */
    char diff[TIME_STR_LEN];
    /* fractional position of the event in the day
     * (time / 24h)
     * range: 0 - 1 */
    double position;
} event_time;

typedef struct
{
    char greg_date[DATE_STR_LEN];
    char hijri_date[DATE_STR_LEN];
    char *weekday;
    event_time now;
    event_time prayers[6];
} day_strings;

#endif
