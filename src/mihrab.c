/*********************************************************************
 *                     Mihrab: Muslim Prayer Times
 * mihrab.c
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

#include <time.h>
#include <locale.h>

#include <itl/prayer.h>
#include <itl/hijri.h>

#include "common.h"
#include "gui.h"
#include "strings.h"

#define STR_LEN 200

typedef struct
{
    time_t now_t; //=0 TODO: make local
    struct tm now_tm;
    int next_prayer;// = 0;

    /* ITL structures */
    Date today;
    Prayer prayers[7];
    Location location;
    Method method;

    /* Options */
    gboolean is24;// = FALSE;
    gboolean include_shorooq;// = TRUE;
} internal_state;

static internal_state *state;
static day_strings *day;

static void init_state(void)
{
    state->now_t = 0;
    state->next_prayer = 0;
    state->is24 = FALSE;
    state->include_shorooq = TRUE;
}

static void sec_to_hms(gint32 seconds, char *hms_str, int hms_str_len)
{
    if (seconds > 3600)
        snprintf(hms_str, hms_str_len, period_format_hms(), seconds / 3600, seconds % 3600 / 60, seconds % 60);
    else if (seconds > 60)
        snprintf(hms_str, hms_str_len, period_format_ms(), seconds / 60, seconds % 60);
    else
        snprintf(hms_str, hms_str_len, period_format_s(), seconds);
}

static gint32 hms_to_sec(int hour, int min, int sec)
{
    return (gint32) (hour * 3600 + min * 60 + sec);
}

static void update_prayers(void)
{
    struct tm t;
    gint32 prayer_diff;
    int i;

    getPrayerTimes(&(state->location), &(state->method), &(state->today), state->prayers);
    getNextDayFajr(&(state->location), &(state->method), &(state->today), &(state->prayers[6]));
    state->prayers[6].hour += 24;

    for (i = 0; i < 6; i++)
    {
	t.tm_hour = state->prayers[i].hour;
	t.tm_min = state->prayers[i].minute;
	t.tm_sec = state->prayers[i].second;

	/* prayer_time_str */
        strftime(day->prayers[i].time, TIME_STR_LEN, time_format(state->is24), &t);

	/* prayer_diff_str */
	prayer_diff = hms_to_sec(state->prayers[i+1].hour, state->prayers[i+1].minute, state->prayers[i+1].second) -
	    hms_to_sec(state->prayers[i].hour, state->prayers[i].minute, state->prayers[i].second);
	sec_to_hms(prayer_diff, day->prayers[i].diff, TIME_STR_LEN);

	/* prayer positions */
	day->prayers[i].position = (state->prayers[i].hour * 60.0 + state->prayers[i].minute) / (23 * 60.0 + 59);
    }

    update_prayers_layouts(day);
}

static void update_day(void)
{
    sDate today_hijri;

    state->today.day = state->now_tm.tm_mday;
    state->today.month = state->now_tm.tm_mon + 1;
    state->today.year = state->now_tm.tm_year + 1900;

    h_date(&today_hijri, state->today.day, state->today.month, state->today.year);

    snprintf(day->greg_date, DATE_STR_LEN, "%d %s %d", state->today.day, greg_months(state->today.month - 1), state->today.year);
    day->weekday = weekday_names(state->now_tm.tm_wday);
    snprintf(day->hijri_date, DATE_STR_LEN, "%d %s %d", today_hijri.day, hijri_months(today_hijri.month - 1), today_hijri.year);

    update_prayers();
    state->next_prayer = 0;

    update_day_layouts(day);
}

static gboolean update_now(gpointer data)
{
    char tmp_str[TIME_STR_LEN];
    gint32 remain_sec;

    state->now_t = time(NULL);
    localtime_r(&(state->now_t), &(state->now_tm));

    /* now.time */
    strftime(day->now.time, TIME_STR_LEN, time_format(state->is24), &(state->now_tm));

    /* now.position */
    day->now.position = (state->now_tm.tm_hour * 3600.0 +
                         state->now_tm.tm_min * 60.0 +
                         state->now_tm.tm_sec)
	/ (23 * 3600.0 + 59 * 60.0 + 59);

    /* now.diff: remaining time till next prayer */
    remain_sec = hms_to_sec(state->prayers[state->next_prayer].hour, state->prayers[state->next_prayer].minute,
			    state->prayers[state->next_prayer].second) -
	hms_to_sec(state->now_tm.tm_hour, state->now_tm.tm_min, state->now_tm.tm_sec);
    while (remain_sec < 0)
    {
	state->next_prayer++;
	remain_sec = hms_to_sec(state->prayers[state->next_prayer].hour, state->prayers[state->next_prayer].minute,
				state->prayers[state->next_prayer].second) -
	    hms_to_sec(state->now_tm.tm_hour, state->now_tm.tm_min, state->now_tm.tm_sec);
    }
    if (remain_sec == 0)
	snprintf(day->now.diff, TIME_STR_LEN, _("\n%s prayer time"), prayer_names(state->next_prayer));
    else
    {
	sec_to_hms(remain_sec, tmp_str, TIME_STR_LEN);
	snprintf(day->now.diff, TIME_STR_LEN, _("\n%s\ntill %s"), tmp_str, prayer_names(state->next_prayer));
    }

    update_now_layouts(day);

    if (state->now_tm.tm_mday != state->today.day)
	update_day();

    return TRUE;
}

/* Return value:
 *   positive: daylight saving time is in effect now.
 *   zero    : it is not.
 *   negative: daylight saving not observed.
 * See TZSET(3) */
int update_timezone(const char *timezone_id)
{
    setenv("TZ", timezone_id, 1);
    tzset();
    update_now(NULL);
    if (daylight == 1)
	return state->now_tm.tm_isdst;
    return -1;
}

/* Accepts timezone_offset as input */
/* Used with Known Location where offset can be found using
 * libgweather provided you know whether DST is in effect. Use
 * update_timezone() above to get DST status. */
void update_location_using_timezone_offset(double latitude, double longitude, double timezone_offset)
{
    state->location.degreeLat = latitude;
    state->location.degreeLong = longitude;
    state->location.gmtDiff = timezone_offset;
    state->location.dst = 0;
    state->location.seaLevel = 131.59;
    state->location.pressure = 1010;
    state->location.temperature = 10;
    state->next_prayer = 0;

    update_prayers();
}

/* Finds out timezone_offset using tzset() */
/* Used with Custom Location where offset is not available */
void update_location_using_timezone_id(double latitude, double longitude, const char *timezone_id, gboolean dst)
{
    double offset;

    setenv("TZ", timezone_id, 1);
    tzset();
    offset = -1 * timezone / 3600.0;
    if (dst)
        offset++;

#if DEBUG
    printf("Timezone: %s\n", timezone_id);
    printf("Timezone offset - origianl: %.3lf\n", -1 * timezone / 3600.0);
    printf("Timezone offset - DST     : %.3lf\n\n", offset);
#endif

    update_location_using_timezone_offset(latitude, longitude, offset);
}

static void init_settings(void)
{
    getMethod(4, &(state->method));
    state->method.round = 0;
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    bindtextdomain("mihrab", "./");
    //bindtextdomain("mihrab", "/usr/share/locale");
    textdomain("mihrab");

    internal_state initial_state;
    day_strings initial_day;
    state = &initial_state;
    day = &initial_day;
    init_state();

    init_strings();
    init_settings();
    g_timeout_add(100, update_now, NULL);
    init_gui(&argc, &argv, day);

    return 0;
}
