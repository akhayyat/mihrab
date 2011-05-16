/*
 * File name: gui.h
 */

#ifndef GUI_H
#define GUI_H

//void init_location(void);

void update_prayers_layouts(day_strings *day);

void update_day_layouts(day_strings *day);

void update_now_layouts(day_strings *day);

void init_gui(int *argc_p, char **argv_p[], day_strings *day);

#endif
