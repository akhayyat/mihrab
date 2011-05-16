/*
 * File name: common.h
 */

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
