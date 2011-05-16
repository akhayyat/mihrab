/*
 * File name: strings.h
 */

#ifndef STRINGS_H
#define STRINGS_H

void init_strings(void);

char *time_format(gboolean is24);
char *period_format_hms(void);
char *period_format_ms(void);
char *period_format_s(void);
char *prayer_names(int i);
char *hijri_months(int i);
char *weekday_names(int i);
char *greg_months(int i);

#endif
