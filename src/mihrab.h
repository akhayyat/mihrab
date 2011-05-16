/*
 * File name: mihrab.h
 */

#ifndef MIHRAB_H
#define MIHRAB_H

void update_location_using_timezone_offset(double latitude, double longitude, double timezone_offset);
void update_location_using_timezone_id(double latitude, double longitude, const char *timezone_id, gboolean dst);

int update_timezone(const char *timezone_id);

#endif
