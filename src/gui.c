/*********************************************************************
 *                     Mihrab: Muslim Prayer Times
 * gui.c
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
 * Mihrab is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mihrab. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#include <assert.h>

#include <gtk/gtk.h>

#include <libgweather-3.0/libgweather/location-entry.h>
#include <libgweather-3.0/libgweather/gweather-location.h>
#include <libgweather-3.0/libgweather/timezone-menu.h>

#include "common.h"
#include "mihrab.h"
#include "strings.h"

#define NOW_MARKUP_BEFORE "<span foreground='DodgerBlue4'><b>"
#define NOW_MARKUP_AFTER  "</b></span>"
#define NOW_MARKUP_LEN 100
#define NOW_SYMB_WIDTH 10

#define MARGIN 10
#define TIMELINE_MARGIN 0
#define TIMELINE_WIDTH 1000
#define TIMELINE_HEIGHT 200
#define TIMELINE_DEPTH 60
#define TIMELINE_LINE_WIDTH 4
#define MIHRAB_WIDTH 10
#define MIHRAB_HEIGHT 25
#define MIHRAB_HEIGHT2 13
#define SUN_RADIUS 10

#define PRAYER_NAME_LAYOUT_Y 50
#define DIFF_LAYOUT_Y 100
#define PRAYER_TIME_LAYOUT_Y 90
#define NOW_LAYOUT_Y 35

#define INITIAL_REGION  "Middle East"
#define INITIAL_COUNTRY "Saudi Arabia"
#define INITIAL_CITY    "Mecca"

#define METHODS 11

/* Prayer Times page widgets */
static PangoLayout *now_layout = NULL;
static PangoLayout *prayer_name_layouts[6];
static PangoLayout *prayer_time_layouts[6];
static PangoLayout *diff_layouts[6];
static GtkWidget *drawarea = NULL;
static GtkWidget *window = NULL;
static GtkWidget *greg_label = NULL;
static GtkWidget *hijri_label = NULL;
static GtkWidget *weekday_label = NULL;
static double now_symb_color[] = {16.0/255, 78.0/255, 139.0/255};
static double prayer_colors[][3] = { {0.4 , 0   , 0.4 },  /* Fajr */
                                     {0.8 , 0.4 , 0   },  /* Shorooq */
                                     {0.8 , 0.7 , 0   },  /* Dhuhr */
                                     {0.8 , 0.55, 0   },  /* Asr */
                                     {0.8 , 0.4 , 0   },  /* Maghrib */
                                     {0.4 , 0   , 0.4 }}; /* Ishaa */
static double prayer_sun_x[] = {-MIHRAB_WIDTH, /* Fajr */
                                -MIHRAB_WIDTH, /* Shorooq */
                                0,             /* Dhuhr */
                                MIHRAB_WIDTH,  /* Asr */
                                MIHRAB_WIDTH,  /* Maghrib */
                                MIHRAB_WIDTH}; /* Ishaa */
static double prayer_sun_y[] = {0,                             /* Fajr */
                                -SUN_RADIUS,                   /* Shorooq */
                                -MIHRAB_HEIGHT-MIHRAB_HEIGHT2, /* Dhuhr */
                                -MIHRAB_HEIGHT,                /* Asr */
                                -SUN_RADIUS,                   /* Maghrib */
                                0};                            /* Isha */
static char *method_names[] = {
    "none",
    "Egyptian General Authority of Survey",
    "University of Islamic Sciences, Karachi (Shaf'i)",
    "University of Islamic Sciences, Karachi (Hanafi)",
    "Islamic Society of North America",
    "Muslim World League (MWL)",
    "Umm Al-Qurra, Saudi Arabia",
    "Fixed Ishaa Interval (always 90)",
    "Egyptian General Authority of Survey (Egypt)",
    "Umm Al-Qurra Ramadan, Saudi Arabia",
    "Moonsighting Committee Worldwide"};
static gboolean method_display[] = {
    FALSE,   /* none */
    TRUE,    /* Egyptian General Authority of Survey */
    TRUE,    /* University of Islamic Sciences, Karachi (Shaf'i) */
    TRUE,    /* University of Islamic Sciences, Karachi (Hanafi) */
    TRUE,    /* Islamic Society of North America */
    TRUE,    /* Muslim World League (MWL) */
    TRUE,    /* Umm Al-Qurra, Saudi Arabia */
    FALSE,   /* Fixed Ishaa Interval (always 90) */
    FALSE,   /* Egyptian General Authority of Survey (Egypt) */
    FALSE,   /* Umm Al-Qurra Ramadan, Saudi Arabia */
    TRUE     /* Moonsighting Committee Worldwide */
};

/* Settings page widgets */
static GtkWidget *known_location_radio = NULL;
static GtkWidget *custom_location_radio = NULL;
static GtkWidget *location_entry = NULL;
static GtkWidget *name_entry = NULL;
static GtkWidget *latitude_spin = NULL;
static GtkWidget *longitude_spin = NULL;
static GtkWidget *timezone_menu = NULL;
static GtkWidget *dst_label = NULL;
static GtkWidget *no_dst_radio = NULL;
static GtkWidget *yes_dst_radio = NULL;
static GtkWidget *method_buttons[METHODS];

static void extract_location_from_location_entry(void)
{
    double latitude, longitude;
    GWeatherTimezone *timezone;
    const char *timezone_id;
    int dst, timezone_offset;

    GWeatherLocation *gw_location = gweather_location_entry_get_location(GWEATHER_LOCATION_ENTRY(location_entry));

    /* Timezone */
    timezone = gweather_location_get_timezone(gw_location);
    timezone_id = gweather_timezone_get_tzid(timezone);
    dst = update_timezone(timezone_id);
    if (dst > 0)
        timezone_offset = gweather_timezone_get_dst_offset(timezone);
    else
        timezone_offset = gweather_timezone_get_offset(timezone);
    timezone_offset /= 60.0; /* minutes to hours */

#if DEBUG
    /* Make sure GWeather and localtime() dst are consistent */
    assert(gweather_timezone_has_dst(timezone) == (dst >= 0));
#endif

    /* Latitude & longitude */
    if (gweather_location_has_coords(gw_location))
        gweather_location_get_coords(gw_location, &latitude, &longitude);
    else
    {
        latitude = -1;
        longitude = -1;
    }

    /* Update widgets */
    gtk_entry_set_text(GTK_ENTRY(name_entry), gweather_location_get_city_name(gw_location));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(latitude_spin), latitude);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(longitude_spin), longitude);
    gweather_timezone_menu_set_tzid(GWEATHER_TIMEZONE_MENU(timezone_menu), timezone_id);

    /* A disabled DST label indicates that DST is not observed in this timezone */
    gtk_widget_set_sensitive(dst_label, dst >= 0);

    /* DST radio buttons: yes or no */
    if (dst > 0)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(yes_dst_radio), TRUE);
    else
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(no_dst_radio), TRUE);

    update_location_using_timezone_offset(latitude, longitude, timezone_offset);
}

static GWeatherLocation *find_gweather_location(GWeatherLocation *parent, const char *name)
{
    if (parent == NULL)
        return NULL;

    GWeatherLocation **locations;
    const char *some_name;
    int i = -1;

    locations = gweather_location_get_children(parent);
    do
    {
        i++;
        if (locations[i] == NULL)  // no more locations
            return NULL;

        some_name = gweather_location_get_name(locations[i]);
    } while (strcmp(some_name, name) != 0); // `name` not found yet
    return locations[i];
}

static void init_location(void)
{
    GWeatherLocation *gw_world, *gw_region, *gw_country, *gw_city, *gw_location;
    const char *city, *code;
    int i, j;

    gw_world = gweather_location_get_world();
    gw_region = find_gweather_location(gw_world, INITIAL_REGION);
    gw_country = find_gweather_location(gw_region, INITIAL_COUNTRY);
    gw_city = find_gweather_location(gw_country, INITIAL_CITY);

    if (gw_city == NULL)
        g_error("Initial location not found\n"); // currently irrecoverable
    // TODO: gracefully handle invalid initial location

    gw_location = gweather_location_get_children(gw_city)[0];

    city = gweather_location_get_city_name(gw_location);
    code = gweather_location_get_code(gw_location);

#if DEBUG
    GWeatherTimezone *timezone;
    const char *timezone_id;
    int timezone_offset;
    timezone = gweather_location_get_timezone(gw_location);
    timezone_id = gweather_timezone_get_tzid(timezone);
    timezone_offset = gweather_timezone_get_offset(timezone);
    g_printf("Initial location\n");
    g_printf("  Name      : %s\n", gweather_location_get_name(gw_location));
    g_printf("  Timezone  : %s, %d\n", timezone_id, timezone_offset);
    g_printf("  City/code : %s / %s\n", city, code);
#endif

    /* For some weird reason, set_city doesn't work now but
     * set_location works - used to be the other way around! */
    gweather_location_entry_set_location(GWEATHER_LOCATION_ENTRY(location_entry), gw_location);
    /* gweather_location_entry_set_city(GWEATHER_LOCATION_ENTRY(location_entry), city, code); */

    /* force a toggled event on the known_location_radio to disable custom location widgets */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(custom_location_radio), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(known_location_radio), TRUE);
}

void update_prayers_layouts(day_strings *day)
{
    int i;

    for (i = 0; i < 6; i++)
    {
        pango_layout_set_text(prayer_time_layouts[i], day->prayers[i].time, -1);
        pango_layout_set_text(diff_layouts[i], day->prayers[i].diff, -1);
    }
}

void update_day_layouts(day_strings *day)
{
    gtk_label_set_label(GTK_LABEL(greg_label), day->greg_date);
    gtk_label_set_label(GTK_LABEL(weekday_label), day->weekday);
    gtk_label_set_label(GTK_LABEL(hijri_label), day->hijri_date);

    if (gtk_widget_get_window(drawarea))
        gdk_window_invalidate_rect(gtk_widget_get_window(drawarea), NULL, TRUE);
}

void *get_now_layout_rectangle(day_strings *day, GdkRectangle *rectangle)
{
    pango_layout_get_pixel_size(now_layout, &(rectangle->width), &(rectangle->height));
    rectangle->x = TIMELINE_MARGIN +
        (gtk_widget_get_allocated_width(drawarea) - 2 * TIMELINE_MARGIN) * day->now.position -
        rectangle->width / 2;
    if (rectangle->x < TIMELINE_MARGIN)
        rectangle->x = TIMELINE_MARGIN;
    else if (rectangle->x + rectangle->width > gtk_widget_get_allocated_width(drawarea) - TIMELINE_MARGIN)
        rectangle->x = gtk_widget_get_allocated_width(drawarea) - TIMELINE_MARGIN - rectangle->width;
    rectangle->y = gtk_widget_get_allocated_height(drawarea) / 2 + NOW_LAYOUT_Y;
}

void update_now_layouts(day_strings *day)
{
    char now_str_w_markup[TIME_STR_LEN];

    g_snprintf(now_str_w_markup, TIME_STR_LEN, "%s%s%s", NOW_MARKUP_BEFORE, day->now.time, NOW_MARKUP_AFTER);
    strcat(now_str_w_markup, day->now.diff);
    pango_layout_set_markup(now_layout, now_str_w_markup, -1);

    if (gtk_widget_get_window(drawarea))
    {
        static GdkRectangle old_rectangle;
        GdkRectangle new_rectangle;

        get_now_layout_rectangle(day, &new_rectangle);
        /* Increase rectangle height to cover the now symbol */
        new_rectangle.height += NOW_LAYOUT_Y + MIHRAB_HEIGHT;
        new_rectangle.y -= NOW_LAYOUT_Y + MIHRAB_HEIGHT;

        gdk_window_invalidate_rect(gtk_widget_get_window(drawarea), &old_rectangle, TRUE);
        gdk_window_invalidate_rect(gtk_widget_get_window(drawarea), &new_rectangle, TRUE);
#if DEBUG
        cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(drawarea));
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_rectangle(cr, old_rectangle.x, old_rectangle.y, old_rectangle.width, old_rectangle.height);
        cairo_stroke(cr);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_rectangle(cr, new_rectangle.x, new_rectangle.y, new_rectangle.width, new_rectangle.height);
        cairo_stroke(cr);
        cairo_destroy(cr);
#endif
        old_rectangle = new_rectangle;
    }
}

static gboolean location_entry_handler(GtkWidget *widget, GdkEvent *event)
{
    extract_location_from_location_entry();
}

static gboolean location_type_handler(GtkWidget *widget, GdkEvent *event)
{
    gboolean known;

    if ((widget == known_location_radio &&
         gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(known_location_radio)))
        ||
        (widget == custom_location_radio &&
         gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(custom_location_radio))))
    {
        if (widget == known_location_radio)
        {
            /* Known location */
            known = TRUE;
            extract_location_from_location_entry();
        }
        else
        {
            /* Custom location */
            known = FALSE;
            gtk_widget_set_sensitive(dst_label, TRUE);
        }

        gtk_widget_set_sensitive(location_entry, known);

        gtk_widget_set_sensitive(name_entry, !known);
        gtk_widget_set_sensitive(latitude_spin, !known);
        gtk_widget_set_sensitive(longitude_spin, !known);
        gtk_widget_set_sensitive(timezone_menu, !known);
        gtk_widget_set_sensitive(no_dst_radio, !known);
        gtk_widget_set_sensitive(yes_dst_radio, !known);
    }
}

static gboolean custom_location_change_handler(GtkWidget *widget, GdkEvent *event)
{
    /* Conditions to avoid handling inactive radio buttons: */
    /* 1. Custom location is active */
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(custom_location_radio)) &&
        /* 2. widget is not a radio button (DST) */
        ((widget != no_dst_radio && widget != yes_dst_radio) ||
         /* 3. widget is an active radio button (DST) */
         (widget == no_dst_radio && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(no_dst_radio))) ||
         (widget == yes_dst_radio && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(yes_dst_radio)))))
    {
        double latitude, longitude;
        const char *timezone_id;
        gboolean dst;

        latitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(latitude_spin));
        longitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(longitude_spin));
        timezone_id = gweather_timezone_menu_get_tzid(GWEATHER_TIMEZONE_MENU(timezone_menu));
        dst = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(yes_dst_radio));

        update_location_using_timezone_id(latitude, longitude, timezone_id, dst);
    }
}

gboolean draw_handler(GtkWidget *widget, cairo_t *cr, gpointer day_data)
{
    day_strings *day = day_data;
    int width, height, x, y, old_x, avg_x, i;
    y = gtk_widget_get_allocated_height(widget) / 2;
    cairo_pattern_t *pattern, *mask_pattern;

    for (i = 0; i < 6; i++)
    {
        old_x = x;

        x = TIMELINE_MARGIN + (gtk_widget_get_allocated_width(drawarea) - 2 * TIMELINE_MARGIN) * day->prayers[i].position;

        pango_layout_get_pixel_size(prayer_time_layouts[i], &width, &height);
        gtk_render_layout(gtk_widget_get_style_context(drawarea),
                          cr,
                          (i==0 || i==4? (x - width * 2/3):(i==1 || i==5? (x - width * 1/3): x - width / 2)),
                          y + PRAYER_TIME_LAYOUT_Y,
                          prayer_time_layouts[i]);

        pango_layout_get_pixel_size(prayer_name_layouts[i], &width, &height);
        gtk_render_layout(gtk_widget_get_style_context(drawarea),
                          cr,
                          x - width / 2,
                          y - PRAYER_NAME_LAYOUT_Y - height,
                          prayer_name_layouts[i]);

        if (i > 0)
        {
            avg_x = (old_x + x) / 2;
            pango_layout_get_pixel_size(diff_layouts[i-1], &width, &height);
            gtk_render_layout(gtk_widget_get_style_context(drawarea),
                              cr,
                              avg_x - width / 2,
                              y - DIFF_LAYOUT_Y - height,
                              diff_layouts[i-1]);
            if (i == 5)
            {
                avg_x = (x + gtk_widget_get_allocated_width(drawarea) - TIMELINE_MARGIN) / 2;
                pango_layout_get_pixel_size(diff_layouts[i], &width, &height);
                gtk_render_layout(gtk_widget_get_style_context(drawarea),
                                  cr,
                                  avg_x - width / 2,
                                  y - DIFF_LAYOUT_Y - height,
                                  diff_layouts[i]);
            }
        }

        /* draw prayer symbol: sun */
        pattern = cairo_pattern_create_radial (x + prayer_sun_x[i], y + prayer_sun_y[i] - SUN_RADIUS, 1,
                                               x + prayer_sun_x[i], y + prayer_sun_y[i] - SUN_RADIUS, 2.5 * SUN_RADIUS);
        cairo_pattern_add_color_stop_rgb (pattern, 0, 1, 1, 0);
        cairo_pattern_add_color_stop_rgb (pattern, 1, prayer_colors[i][0], prayer_colors[i][1], prayer_colors[i][2]);
        cairo_set_source (cr, pattern);
        cairo_move_to (cr, x + prayer_sun_x[i], y + prayer_sun_y[i]);
        if (i == 0 || i == 5)
            cairo_arc (cr, x + prayer_sun_x[i], y + prayer_sun_y[i], SUN_RADIUS, -G_PI, 0);
        else
            cairo_arc (cr, x + prayer_sun_x[i], y + prayer_sun_y[i], SUN_RADIUS, 0, 2 * G_PI);
        cairo_fill (cr);
        /* draw prayer symbol: mihrab */
        cairo_move_to (cr, x - MIHRAB_WIDTH, y);
        cairo_line_to (cr, x - MIHRAB_WIDTH, y - MIHRAB_HEIGHT);
        cairo_arc (cr, x, y - MIHRAB_HEIGHT, MIHRAB_WIDTH, G_PI, 5.0/4 * G_PI);
        cairo_line_to (cr, x, y - MIHRAB_HEIGHT - MIHRAB_HEIGHT2);
        cairo_arc (cr, x, y - MIHRAB_HEIGHT, MIHRAB_WIDTH, -1.0/4 * G_PI, 0);
        cairo_line_to (cr, x + MIHRAB_WIDTH, y);
        cairo_set_source_rgb (cr, 1, 1, 1);
        cairo_fill_preserve(cr);
        cairo_set_source_rgb (cr, prayer_colors[i][0], prayer_colors[i][1], prayer_colors[i][2]);
        cairo_stroke (cr);
        cairo_pattern_destroy (pattern);
#if DEBUG
        assert (cairo_pattern_get_reference_count (pattern) == 0);
#endif
    }

    /* draw the timeline */
    cairo_push_group (cr);

    /* draw the timeline: horizontal color pattern */
    pattern = cairo_pattern_create_linear (TIMELINE_MARGIN, y, gtk_widget_get_allocated_width(widget), y);
    cairo_pattern_add_color_stop_rgb (pattern, 0, 0, 0, 0);
    for (i = 0; i < 6; i++)
    {
        cairo_pattern_add_color_stop_rgb (pattern, day->prayers[i].position, prayer_colors[i][0], prayer_colors[i][1], prayer_colors[i][2]);
        cairo_pattern_add_color_stop_rgb (pattern, day->prayers[i].position, prayer_colors[i][0], prayer_colors[i][1], prayer_colors[i][2]);
    }
    cairo_pattern_add_color_stop_rgb (pattern, 1, 0, 0, 0);
    cairo_set_source (cr, pattern);

    /* draw the timeline: vertical transparency (alpha) pattern */
    mask_pattern = cairo_pattern_create_linear (0, y, 0, y + TIMELINE_DEPTH);
    cairo_pattern_add_color_stop_rgba (mask_pattern, 0, 0, 0, 0, 1);
    cairo_pattern_add_color_stop_rgba (mask_pattern, 1, 0, 0, 0, 0);
    cairo_mask (cr, mask_pattern);
    cairo_pop_group_to_source (cr);
    /* paint the timeline */
    cairo_rectangle (cr, TIMELINE_MARGIN, y, gtk_widget_get_allocated_width(widget) - 2 * TIMELINE_MARGIN, TIMELINE_DEPTH);
    cairo_fill (cr);

    /* release the current source pattern */
    cairo_set_source_rgb (cr, 1, 1, 1);

    cairo_pattern_destroy (pattern);
    cairo_pattern_destroy (mask_pattern);
#if DEBUG
    assert (cairo_pattern_get_reference_count (pattern) == 0);
    assert (cairo_pattern_get_reference_count (mask_pattern) == 0);
#endif

    /* draw the now symbol */
    x = TIMELINE_MARGIN + (gtk_widget_get_allocated_width(drawarea) - 2 * TIMELINE_MARGIN) * day->now.position;
    pattern = cairo_pattern_create_radial (x-3, y-3, 1.5,
                                           x-1, y-1, 15);
    cairo_pattern_add_color_stop_rgba (pattern, 0, 1, 1, 1, 1);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 1);
    cairo_set_source (cr, pattern);
    cairo_arc (cr, x, y, 5, 0, 2 * G_PI);
    cairo_fill (cr);

    GdkRectangle rectangle;
    get_now_layout_rectangle(day, &rectangle);

    gtk_render_layout(gtk_widget_get_style_context(drawarea),
                      cr,
                      rectangle.x,
                      rectangle.y,
                      now_layout);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_pattern_destroy (pattern);
#if DEBUG
    assert (cairo_pattern_get_reference_count (pattern) == 0);
#endif

    return TRUE;
}

static GtkWidget *create_times_page(day_strings *day)
{
    GtkWidget *grid;
    char now_markup_str[NOW_MARKUP_LEN];

    greg_label = gtk_label_new("");
    weekday_label = gtk_label_new("");
    hijri_label = gtk_label_new("");

    drawarea = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawarea, TIMELINE_WIDTH, TIMELINE_HEIGHT);
    g_signal_connect(G_OBJECT(drawarea), "draw",
                     G_CALLBACK (draw_handler), day);

    now_layout = gtk_widget_create_pango_layout(drawarea, NULL);
    pango_layout_set_alignment(now_layout, PANGO_ALIGN_CENTER);
    int i;
    for (i = 0; i < 6; i++)
    {
        prayer_name_layouts[i] = gtk_widget_create_pango_layout(drawarea, prayer_names(i));
        prayer_time_layouts[i] = gtk_widget_create_pango_layout(drawarea, "");
        diff_layouts[i] = gtk_widget_create_pango_layout(drawarea, "");
    }

    grid = gtk_grid_new();
    g_object_set(greg_label, "margin-left", MARGIN, NULL);
    gtk_widget_set_hexpand(weekday_label, TRUE);
    g_object_set(hijri_label, "margin-right", MARGIN, NULL);
    gtk_grid_attach(GTK_GRID(grid), greg_label, 0, 0, 1, 1);
    gtk_grid_attach_next_to(GTK_GRID(grid), weekday_label, greg_label, GTK_POS_RIGHT, 1, 1);
    gtk_grid_attach_next_to(GTK_GRID(grid), hijri_label, weekday_label, GTK_POS_RIGHT, 1, 1);

    gtk_widget_set_hexpand(drawarea, TRUE);
    gtk_widget_set_vexpand(drawarea, TRUE);
    gtk_grid_attach(GTK_GRID(grid), drawarea, 0, 1, 3, 1);

    gtk_widget_show(greg_label);
    gtk_widget_show(weekday_label);
    gtk_widget_show(hijri_label);
    gtk_widget_show(drawarea);
    gtk_widget_show(grid);

    return grid;
}

static GtkWidget *create_location_frame(void)
{
    GtkWidget *grid, *location_frame, *label;
    GtkAdjustment *adjust;
    GWeatherLocation *gw_world = gweather_location_get_world();
    int row = 0;

    grid = gtk_grid_new();
    g_object_set(grid, "margin", MARGIN, NULL);
    gtk_grid_set_row_spacing(GTK_GRID(grid), MARGIN);
    gtk_grid_set_column_spacing(GTK_GRID(grid), MARGIN);

    known_location_radio = gtk_radio_button_new_with_mnemonic(NULL, _("_Known Location"));
    g_signal_connect(G_OBJECT(known_location_radio), "toggled", G_CALLBACK(location_type_handler), NULL);
    location_entry = gweather_location_entry_new(gw_world);
    g_signal_connect(G_OBJECT(location_entry), "changed", G_CALLBACK(location_entry_handler), NULL);
    gtk_grid_attach(GTK_GRID(grid), known_location_radio, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), location_entry, 1, row, 2, 1);
    row++;

    label = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 3, 1);
    row++;

    custom_location_radio = gtk_radio_button_new_with_mnemonic_from_widget(GTK_RADIO_BUTTON(known_location_radio), _("_Custom Location:"));
    g_signal_connect(G_OBJECT(custom_location_radio), "toggled", G_CALLBACK(location_type_handler), NULL);
    gtk_grid_attach(GTK_GRID(grid), custom_location_radio, 0, row, 3, 1);
    row++;

    label = gtk_label_new_with_mnemonic(_("Na_me"));
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    name_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, row, 2, 1);
    gtk_label_set_mnemonic_widget(GTK_LABEL(label), name_entry);
    row++;

    label = gtk_label_new_with_mnemonic(_("La_titude"));
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    adjust = (GtkAdjustment *) gtk_adjustment_new(0, -90.0, 90.0, 0.01, 1.0, 0.0);
    latitude_spin = gtk_spin_button_new(adjust, 0.1, 3);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(latitude_spin), TRUE);
    g_signal_connect(G_OBJECT(latitude_spin), "value-changed", G_CALLBACK(custom_location_change_handler), NULL);
    gtk_grid_attach(GTK_GRID(grid), latitude_spin, 1, row, 2, 1);
    gtk_label_set_mnemonic_widget(GTK_LABEL(label), latitude_spin);
    row++;

    label = gtk_label_new_with_mnemonic(_("Lon_gitude"));
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    adjust = (GtkAdjustment *) gtk_adjustment_new(0, -180.0, 180.0, 0.01, 1.0, 0.0);
    longitude_spin = gtk_spin_button_new(adjust, 0.1, 3);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(longitude_spin), TRUE);
    g_signal_connect(G_OBJECT(longitude_spin), "value-changed", G_CALLBACK(custom_location_change_handler), NULL);
    gtk_grid_attach(GTK_GRID(grid), longitude_spin, 1, row, 2, 1);
    gtk_label_set_mnemonic_widget(GTK_LABEL(label), longitude_spin);
    row++;

    label = gtk_label_new_with_mnemonic(_("Time _Zone"));
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    timezone_menu = gweather_timezone_menu_new(gw_world);
    g_signal_connect(G_OBJECT(timezone_menu), "notify::tzid", G_CALLBACK(custom_location_change_handler), NULL);
    gtk_grid_attach(GTK_GRID(grid), timezone_menu, 1, row, 2, 1);
    gtk_label_set_mnemonic_widget(GTK_LABEL(label), timezone_menu);
    row++;

    dst_label = gtk_label_new_with_mnemonic(_("Daylight Saving"));
    gtk_misc_set_alignment(GTK_MISC(dst_label), 1, 0.5);
    gtk_grid_attach(GTK_GRID(grid), dst_label, 0, row, 1, 1);
    no_dst_radio = gtk_radio_button_new_with_mnemonic(NULL, _("_No"));
    yes_dst_radio = gtk_radio_button_new_with_mnemonic_from_widget(GTK_RADIO_BUTTON(no_dst_radio), _("_Yes"));
    g_signal_connect(G_OBJECT(no_dst_radio), "toggled", G_CALLBACK(custom_location_change_handler), NULL);
    g_signal_connect(G_OBJECT(yes_dst_radio), "toggled", G_CALLBACK(custom_location_change_handler), NULL);
    gtk_grid_attach(GTK_GRID(grid), no_dst_radio, 1, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), yes_dst_radio, 2, row, 1, 1);
    row++;

    location_frame = gtk_frame_new(_("Location"));
    gtk_container_add(GTK_CONTAINER(location_frame), grid);

/* http://www.earthtools.org/ */
/* http://world.maporama.com/ */
/* http://www.getty.edu/research/conducting_research/vocabularies/tgn/ */

    gtk_widget_show_all(location_frame);

    return location_frame;
}

static GtkWidget *create_method_frame(void)
{
    GtkWidget *grid, *method_frame;

    grid = gtk_grid_new();
    g_object_set(grid, "margin", MARGIN, NULL);

    gboolean first_method = -1;
    int i, row;
    for (i = 0, row = 0; i < METHODS; i++)
    {
        if (method_display[i])
        {
            if (first_method == -1)
            {
                method_buttons[i] = gtk_radio_button_new_with_mnemonic(NULL,
                    _(method_names[i]));
                first_method = i;
            }
            else
            {
                method_buttons[i] = gtk_radio_button_new_with_mnemonic_from_widget(
                    GTK_RADIO_BUTTON(method_buttons[first_method]),
                    _(method_names[i]));
            }
            gtk_grid_attach(GTK_GRID(grid), method_buttons[i], 1, row++, 1, 1);
        }
    }

    method_frame = gtk_frame_new(_("Calculation Method"));
    gtk_container_add(GTK_CONTAINER(method_frame), grid);

    gtk_widget_show_all(method_frame);

    return method_frame;
}

static GtkWidget *create_settings_page(void)
{
    GtkWidget *grid;

    grid = gtk_grid_new();

    gtk_container_set_border_width(GTK_CONTAINER(grid), MARGIN);
    gtk_grid_set_row_spacing(GTK_GRID(grid), MARGIN);
    gtk_grid_set_column_spacing(GTK_GRID(grid), MARGIN);

    gtk_grid_attach(GTK_GRID(grid), create_location_frame(), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), create_method_frame(), 1, 0, 1, 1);

    gtk_widget_show(grid);

    return grid;
}

void init_gui(int *argc_p, char **argv_p[], day_strings *day)
{
    GtkWidget *notebook, *times_page_label, *settings_page_label;

    bindtextdomain("mihrab", "./");
    textdomain("mihrab");

    gtk_init(argc_p, argv_p);

    notebook = gtk_notebook_new();
    times_page_label = gtk_label_new_with_mnemonic(_("_Prayer Times"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), create_times_page(day), times_page_label);
    settings_page_label = gtk_label_new_with_mnemonic(_("_Settings"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), create_settings_page(), settings_page_label);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), _("Mihrab Prayer Times"));
    gtk_window_set_default_icon_from_file("mihrab.png", NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 2);
    g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_add(GTK_CONTAINER(window), notebook);

    init_location();

    gtk_widget_show(notebook);
    gtk_widget_show(window);

    gtk_main();
}
