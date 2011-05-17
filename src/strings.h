/*********************************************************************
 *                     Mihrab: Muslim Prayer Times
 * strings.h
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
