/*********************************************************************
 *                     Mihrab: Muslim Prayer Times
 * mihrab.h
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

#ifndef MIHRAB_H
#define MIHRAB_H

void update_location_using_timezone_offset(double latitude, double longitude, double timezone_offset);
void update_location_using_timezone_id(double latitude, double longitude, const char *timezone_id, gboolean dst);

int update_timezone(const char *timezone_id);

#endif
