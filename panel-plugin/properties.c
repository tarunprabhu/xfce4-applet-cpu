/*  properties.c
 *  Part of xfce4-cpugraph-plugin
 *
 *  Copyright (c) Alexander Nordfelth <alex.nordfelth@telia.com>
 *  Copyright (c) gatopeich <gatoguan-os@yahoo.com>
 *  Copyright (c) 2007-2008 Angelo Arrifano <miknix@gmail.com>
 *  Copyright (c) 2007-2008 Lidiriel <lidiriel@coriolys.org>
 *  Copyright (c) 2010 Florian Rivoal <frivoal@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "properties.h"
#include "cpu.h"
#include "settings.h"

#include <glib/gprintf.h>
#include <libxfce4ui/libxfce4ui.h>

static GtkBox *create_tab();
static GtkWidget* create_grid();

static void setup_update_interval_option(GtkWidget *grid, int y,
                                         CPUGraph *base);
static void setup_size_option(GtkWidget *grid, int y, GtkOrientation,
                              CPUGraph *base);
static void setup_color_option(GtkWidget *grid, int y, const gchar *name,
                               CPUGraph *base, const gchar *color,
                               GCallback cb);

static void change_bg_color(GtkColorButton*, CPUGraph *base);
static void change_fg_color(GtkColorButton*, CPUGraph *base);
static void response_cb(GtkWidget *dlg, gint response, CPUGraph *base);
static void change_size(GtkSpinButton *sb, CPUGraph *base);
static void change_update(GtkComboBox *om, CPUGraph *base);

void create_options(XfcePanelPlugin *plugin, CPUGraph *base) {
  GtkWidget *dlg, *header;
  GtkBox *vbox1, *vbox2;
  GtkWidget *label;
  GtkWidget *grid1;
  GtkWidget *grid2;
  GtkWidget *notebook;

  xfce_panel_plugin_block_menu(plugin);

  dlg = xfce_titled_dialog_new_with_buttons(
      _("CPU Monitor Properties"),
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(plugin))),
      GTK_DIALOG_DESTROY_WITH_PARENT,
      "gtk-close", GTK_RESPONSE_OK, NULL);

  g_signal_connect(dlg, "response", G_CALLBACK(response_cb), base);

  gtk_window_set_icon_name(GTK_WINDOW(dlg), "utilities-system-monitor");

  grid1 = create_grid();
  vbox1 = create_tab();
  setup_update_interval_option(grid1, 0, base);
  setup_size_option(grid1, 1, xfce_panel_plugin_get_orientation(plugin), base);
  gtk_box_pack_start(GTK_BOX(vbox1), grid1, TRUE, TRUE, 0);

  grid2 = create_grid();
  vbox2 = create_tab();
  setup_color_option(grid2, 0, _("Foreground:"), base, base->fgcolor,
                     G_CALLBACK(change_fg_color));
  setup_color_option(grid2, 1, _("Background:"), base, base->bgcolor,
                     G_CALLBACK(change_bg_color));
  gtk_box_pack_start(GTK_BOX(vbox2), grid2, TRUE, TRUE, 0);
  
  notebook = gtk_notebook_new();
  gtk_container_set_border_width(GTK_CONTAINER(notebook), BORDER - 2);
  label = gtk_label_new(_("Appearance"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), GTK_WIDGET(vbox2),
                           GTK_WIDGET(label));
  label = gtk_label_new(_("Advanced"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), GTK_WIDGET(vbox1),
                           GTK_WIDGET(label));
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dlg))),
                     notebook, TRUE, TRUE, 0);

  gtk_widget_show(grid1);
  gtk_widget_show(grid2);
  gtk_widget_show(notebook);
  gtk_widget_show(dlg);
}

static GtkWidget* create_grid() {
  GtkWidget* grid;

  grid = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(grid), 2);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 2);

  return grid;
}

static GtkBox *create_tab() {
  GtkBox *tab;
  tab = GTK_BOX(gtk_box_new(FALSE, BORDER));
  gtk_container_set_border_width(GTK_CONTAINER(tab), BORDER);
  gtk_widget_show(GTK_WIDGET(tab));
  return tab;
}

static void setup_update_interval_option(GtkWidget *grid, int y, CPUGraph *base) {
  const gchar *items[] = {_("Fastest (~250ms)"), _("Fast (~500ms)"),
                          _("Normal (~750ms)"), _("Slow (~1s)")};
  gsize nb_items = sizeof(items) / sizeof(gchar *);
  GtkWidget *combo, *label;
  int i;

  label = gtk_label_new(_("Update Interval"));
  gtk_grid_attach(GTK_GRID(grid), label, 0, y, 1, 1);
  gtk_widget_show(label);
  
  combo = gtk_combo_box_text_new();
  for (i = 0; i < nb_items; i++)
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), items[i]);
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo), base->update_interval);
  g_signal_connect(combo, "changed", G_CALLBACK(change_update), base);
  gtk_grid_attach(GTK_GRID(grid), combo, 1, y, 1, 1);
  gtk_widget_show(combo);
}

static void setup_size_option(GtkWidget *grid, int y,
                              GtkOrientation orientation, CPUGraph *base) {
  GtkWidget *label;
  GtkWidget *size;

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    label = gtk_label_new(_("Width"));
  else
    label = gtk_label_new(_("Height"));
  gtk_grid_attach(GTK_GRID(grid), label, 0, y, 1, 1);
  gtk_widget_show(label);
  
  size = gtk_spin_button_new_with_range(10, 128, 1);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(size), base->size);
  g_signal_connect(size, "value-changed", G_CALLBACK(change_size), base);
  gtk_grid_attach(GTK_GRID(grid), size, 1, y, 1, 1);
  gtk_widget_show(size);
}

static void setup_color_option(GtkWidget *grid, int y, const gchar *name,
                               CPUGraph *base, const gchar *scol,
                               GCallback cb) {
  GtkWidget *label, *button;
  GdkRGBA color;

  gdk_rgba_parse(&color, scol);
  
  label = gtk_label_new(name);
  gtk_grid_attach(GTK_GRID(grid), label, 0, y, 1, 1);
  gtk_widget_show(label);

  button = gtk_color_button_new_with_rgba(&color);
  g_signal_connect(button, "color-set", cb, base);
  gtk_grid_attach(GTK_GRID(grid), button, 1, y, 1, 1);
  gtk_widget_show(button);
}

static void change_bg_color(GtkColorButton *button, CPUGraph *base) {
  GdkRGBA color;
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(button), &color);
  set_bg_color(base, &color);
}

static void change_fg_color(GtkColorButton *button, CPUGraph *base) {
  GdkRGBA color;
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(button), &color);
  set_fg_color(base, &color);
}

static void response_cb(GtkWidget *dlg, gint response, CPUGraph *base) {
  gtk_widget_destroy(dlg);
  xfce_panel_plugin_unblock_menu(base->plugin);
  write_settings(base->plugin, base);
}

static void change_size(GtkSpinButton *sb, CPUGraph *base) {
  set_size(base, gtk_spin_button_get_value_as_int(sb));
}

static void change_update(GtkComboBox *combo, CPUGraph *base) {
  set_update_rate(base, gtk_combo_box_get_active(combo));
}
