/*  cpu.c
 *  Part of xfce4-cpugraph-plugin
 *
 *  Copyright (c) Alexander Nordfelth <alex.nordfelth@telia.com>
 *  Copyright (c) gatopeich <gatoguan-os@yahoo.com>
 *  Copyright (c) 2007-2008 Angelo Arrifano <miknix@gmail.com>
 *  Copyright (c) 2007-2008 Lidiriel <lidiriel@coriolys.org>
 *  Copyright (c) 2010 Florian Rivoal <frivoal@gmail.com>
 *  Copyright (c) 2017 Tarun Prabhu <tarun.prabhu@gmail.com>
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
#include "cpu.h"
#include "properties.h"
#include "settings.h"

#include <glib.h>
#include <gtk/gtk.h>

#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4util/libxfce4util.h>

static void draw_graph_normal( CPUGraph *base, GtkWidget *da, gint w, gint h );
static void cpugraph_construct(XfcePanelPlugin *plugin);
static CPUGraph *create_gui(XfcePanelPlugin *plugin);
static guint init_cpu_data(CpuData **data);
static void shutdown(XfcePanelPlugin *plugin, CPUGraph *base);
static gboolean size_cb(XfcePanelPlugin *plugin, guint size, CPUGraph *base);
static void about_cb(XfcePanelPlugin *plugin, CPUGraph *base);
static int update_cb(gpointer data);
static void update_tooltip(CPUGraph *base);
static void update_temp(CPUGraph* base);
static gboolean tooltip_cb(GtkWidget *widget, gint x, gint y, gboolean keyboard,
                           GtkTooltip *tooltip, CPUGraph *base);
static void draw_area_cb(GtkWidget *da, cairo_t*, gpointer data);
static void draw_graph(CPUGraph *base);
static void mode_cb(XfcePanelPlugin *plugin, XfcePanelPluginMode mode,
                    CPUGraph *base);

XFCE_PANEL_PLUGIN_REGISTER(cpugraph_construct);

static void cpugraph_construct(XfcePanelPlugin *plugin) {
  CPUGraph *base;

  base = create_gui(plugin);
  read_settings(plugin, base);
  xfce_panel_plugin_menu_show_configure(plugin);

  xfce_panel_plugin_menu_show_about(plugin);

  g_signal_connect(plugin, "about", G_CALLBACK(about_cb), base);
  g_signal_connect(plugin, "free-data", G_CALLBACK(shutdown), base);
  g_signal_connect(plugin, "save", G_CALLBACK(write_settings), base);
  g_signal_connect(plugin, "configure-plugin", G_CALLBACK(create_options),
                   base);
  g_signal_connect(plugin, "size-changed", G_CALLBACK(size_cb), base);
  g_signal_connect(plugin, "mode-changed", G_CALLBACK(mode_cb), base);
}

static CPUGraph *create_gui(XfcePanelPlugin *plugin) {
  GtkWidget *ebox;
  GtkOrientation orientation;
  GtkCssProvider *css_provider;
  CPUGraph *base = g_new0(CPUGraph, 1);
  int i;

  orientation = xfce_panel_plugin_get_orientation(plugin);
  if((base->nr_cores = init_cpu_data(&base->cpu_data)) == 0)
    fprintf(stderr, "Cannot init cpu data !\n");
  if((base->nr_temps = init_temperature_data(base)) == 0)
    fprintf(stderr, "Cannot init temperature data !\n");

  base->plugin = plugin;

  ebox = gtk_event_box_new();
  gtk_event_box_set_visible_window(GTK_EVENT_BOX(ebox), FALSE);
  gtk_event_box_set_above_child(GTK_EVENT_BOX(ebox), TRUE);
  gtk_container_add(GTK_CONTAINER(plugin), ebox);
  xfce_panel_plugin_add_action_widget(plugin, ebox);

  base->box = gtk_box_new(orientation, 0);
  gtk_container_add(GTK_CONTAINER(ebox), base->box);
  gtk_widget_set_has_tooltip(base->box, TRUE);
  g_signal_connect(base->box, "query-tooltip", G_CALLBACK(tooltip_cb), base);

  base->grid = gtk_grid_new();
  gtk_box_pack_start(GTK_BOX(base->box), base->grid, TRUE, TRUE, 0);

  /* Title label */
  base->title = gtk_label_new("CPU");
  /* FIXME: Make title and font configurable */
  gtk_grid_attach(GTK_GRID(base->grid), base->title, 0, 0, 1, 1);
  gtk_widget_show(base->title);
  css_provider = gtk_css_provider_new();
  gtk_style_context_add_provider(GTK_STYLE_CONTEXT(gtk_widget_get_style_context(
                                     GTK_WIDGET(base->title))),
                                 GTK_STYLE_PROVIDER(css_provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_set_data(G_OBJECT(base->title), "css_provider",
                    css_provider);
  gtk_css_provider_load_from_data(
      g_object_get_data(G_OBJECT(base->title), "css_provider"),
      "label { font-weight: Bold; \
               font-size: 7pt;                       \
               font-family: \"Bitstream Vera Sans\"; \
               color: #696969; \
             }", -1, NULL);

  /* Graph */
  base->frame = gtk_frame_new(NULL);
  gtk_grid_attach(GTK_GRID(base->grid), base->frame, 0, 1, 1, 1);
  
  base->draw_area = gtk_drawing_area_new();
  gtk_widget_set_name(GTK_WIDGET(base->draw_area), "graph");
  gtk_container_add(GTK_CONTAINER(base->frame), GTK_WIDGET(base->draw_area));
  g_signal_connect(base->draw_area, "draw", G_CALLBACK(draw_area_cb), base);
  gtk_widget_show(base->draw_area);

  /* Temperature bars */
  if(base->nr_temps > 0) {
    base->temps = g_malloc0(base->nr_temps * sizeof(GtkWidget*));
    for(i = 0; i < base->nr_temps; i++) {
      base->temps[i] = gtk_label_new("");
      gtk_widget_set_name(GTK_WIDGET(base->temps[i]), "label");
      gtk_grid_attach(GTK_GRID(base->grid), base->temps[i], 0, i+2, 1, 1);
      gtk_widget_show(base->temps[i]);

      css_provider = gtk_css_provider_new();
      gtk_style_context_add_provider(
          GTK_STYLE_CONTEXT(
              gtk_widget_get_style_context(GTK_WIDGET(base->temps[i]))),
          GTK_STYLE_PROVIDER(css_provider),
          GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

      g_object_set_data(G_OBJECT(base->temps[i]), "css_provider", css_provider);
      gtk_css_provider_load_from_data(
          g_object_get_data(G_OBJECT(base->temps[i]), "css_provider"),
          "label     { font-weight: Bold; \
                      font-size: 7pt;                       \
                      font-family: \"Bitstream Vera Sans\"; \
                      padding: 1px; \
                      color: white; \
                    } \
          label#p0  { background: green; } \
          label#p1  { background: linear-gradient(to right, green, #FF1A00); } \
          label#p2  { background: linear-gradient(to right, green, #FF3500); } \
          label#p3  { background: linear-gradient(to right, green, #FF5000); } \
          label#p4  { background: linear-gradient(to right, green, #FF6B00); } \
          label#p5  { background: linear-gradient(to right, green, #FF8600); } \
          label#p6  { background: linear-gradient(to right, green, #FFA100); } \
          label#p7  { background: linear-gradient(to right, green, #FFBB00); } \
          label#p8  { background: linear-gradient(to right, green, #FFD600); } \
          label#p9  { background: linear-gradient(to right, green, #FFF100); } \
          label#p10 { background: linear-gradient(to right, green, #F1FF00); } \
          label#p11 { background: linear-gradient(to right, green, #D6FF00); } \
          label#p12 { background: linear-gradient(to right, green, #BBFF00); } \
          label#p13 { background: linear-gradient(to right, green, #A1FF00); } \
          label#p14 { background: linear-gradient(to right, green, #86FF00); } \
          label#p15 { background: linear-gradient(to right, green, #6BFF00); } \
          label#p16 { background: linear-gradient(to right, green, #50FF00); } \
          label#p17 { background: linear-gradient(to right, green, #35FF00); } \
          label#p18 { background: linear-gradient(to right, green, #1AFF00); } \
          label#p19 { background: linear-gradient(to right, green, #00FF00); } \
          label#p20 { background: red; } \
          ",
          -1, NULL);
    }
  }

  mode_cb(plugin, orientation, base);
  gtk_widget_show_all(ebox);
  
  base->tooltip_text = gtk_label_new(NULL);
  g_object_ref(base->tooltip_text);
    
  return base;
}

static void about_cb(XfcePanelPlugin *plugin, CPUGraph *base) {
  GdkPixbuf *icon;
  const gchar *auth[] = {"Alexander Nordfelth <alex.nordfelth@telia.com>",
                         "gatopeich <gatoguan-os@yahoo.com>",
                         "lidiriel <lidiriel@coriolys.org>",
                         "Angelo Miguel Arrifano <miknix@gmail.com>",
                         "Florian Rivoal <frivoal@gmail.com>",
                         "Peter Tribble <peter.tribble@gmail.com>",
                         "Tarun Prabhu <tarun.prabhu@gmail.com>",
                         NULL};
  icon = xfce_panel_pixbuf_from_source("utilities-system-monitor", NULL, 32);
  gtk_show_about_dialog(
      NULL, "logo", icon, "license",
      xfce_get_license_text(XFCE_LICENSE_TEXT_GPL), "version", PACKAGE_VERSION,
      "program-name", PACKAGE_NAME, "comments",
      "Displays graph of CPU and temperature", "website",
      "http://www.github.com/tarunprabhu/xfce4-applet-cpu", "copyright",
      "Copyright (c) 2003-2012\n", "authors", auth, NULL);

  if (icon)
    g_object_unref(G_OBJECT(icon));
}

guint init_cpu_data(CpuData **data) {
  guint cpuNr;

  cpuNr = detect_cpu_number();
  if (cpuNr == 0)
    return 0;

  *data = (CpuData *)g_malloc0((cpuNr + 1) * sizeof(CpuData));

  return cpuNr;
}

static void shutdown(XfcePanelPlugin *plugin, CPUGraph *base) {
  g_free(base->cpu_data);
  gtk_widget_destroy(base->box);
  gtk_widget_destroy(base->tooltip_text);
  if (base->timeout_id)
    g_source_remove(base->timeout_id);
  g_free(base->history);
  g_free(base);
}

static gboolean size_cb(XfcePanelPlugin *plugin, guint size, CPUGraph *base) {
  gint frame_h, frame_v, history;
  GtkOrientation orientation;

  orientation = xfce_panel_plugin_get_orientation(plugin);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    frame_h = base->size;
    frame_v = size;
    history = base->size;
  } else {
    frame_h = size;
    frame_v = base->size;
    history = size;
  }

  gtk_widget_set_size_request(GTK_WIDGET(base->frame), frame_h, frame_v);

  base->history = (guint *)g_realloc(base->history, history * sizeof(guint));
  if (history > base->history_size)
    memset(base->history + base->history_size, 0,
           (history - base->history_size) * sizeof(guint));
  base->history_size = history;

  return TRUE;
}

static void mode_cb(XfcePanelPlugin *plugin, XfcePanelPluginMode mode,
                    CPUGraph *base) {
  GtkOrientation orientation = (mode == XFCE_PANEL_PLUGIN_MODE_HORIZONTAL)
                                   ? GTK_ORIENTATION_HORIZONTAL
                                   : GTK_ORIENTATION_VERTICAL;

  gtk_orientable_set_orientation(GTK_ORIENTABLE(base->box),
                                 xfce_panel_plugin_get_orientation(plugin));

  size_cb(plugin, xfce_panel_plugin_get_size(base->plugin), base);
}

static int update_cb(gpointer data) {
  CPUGraph* base = (CPUGraph*)data;
  gint i, a, b, factor;
  if (!read_cpu_data(base->cpu_data, base->nr_cores))
    return TRUE;
  if (!read_temperature_data(base->temp_data, base->nr_temps))
    return TRUE;

  memmove(base->history + 1, base->history,
          (base->history_size - 1) * sizeof(guint));

  base->history[0] = base->cpu_data[0].load;

  update_temp(base);
  update_tooltip(base);
  gtk_widget_queue_draw(base->draw_area);

  return TRUE;
}

static void update_temp(CPUGraph *base) {
  int i;
  int t;
  char temp[5];
  char class[16];

  for(i = 0; i < base->nr_temps; i++) {
    t = base->temp_data[i].temp;

    /* Scale the temperature to between 30 and 90 */
    if(t < 30) {
      snprintf(temp, sizeof(temp), "%d°", t);
      snprintf(class, sizeof(class), "p%d", 0);
    } else if(t > 90) {
      snprintf(temp, sizeof(temp), "%s", "MAX");
      snprintf(class, sizeof(class), "p%d", 20);
    } else {
      snprintf(temp, sizeof(temp), "%d°", t);
      /* There are twenty different color classes, and since our temperature
         range is 60°, each step is 3° in size */
      snprintf(class, sizeof(class), "p%d", (t - 30) / 3);
    }

    gtk_label_set_text(GTK_LABEL(base->temps[i]), temp);
    gtk_widget_set_name(GTK_WIDGET(base->temps[i]), class);
  }
}

static void update_tooltip(CPUGraph *base) {
  gchar tooltip[1024];
  int i, curr;

  curr = 0;
  curr = g_snprintf(tooltip + curr, sizeof(tooltip) - curr,
                    "Utilization\n\n");
  for(i = 0; i < base->nr_cores; i++)
    if(curr < sizeof(tooltip))
      curr += g_snprintf(tooltip + curr, sizeof(tooltip) - curr,
                         "Core %d: %u%%\n",
                         i, (guint)base->cpu_data[i+1].load * 100 / CPU_SCALE);
  curr += g_snprintf(tooltip + curr, sizeof(tooltip) - curr,
                     "---------------\nTotal: %u%%\n",
                     (guint)base->cpu_data[0].load * 100 / CPU_SCALE);

  curr += g_snprintf(tooltip + curr, sizeof(tooltip) - curr, "\n");
  for(i = 0; i < base->nr_temps; i++)
    if(curr < sizeof(tooltip))
      curr += g_snprintf(tooltip + curr, sizeof(tooltip) - curr,
                         "Temp %d: %u°",
                         i, base->temp_data[i].temp);
  
  gtk_label_set_text(GTK_LABEL(base->tooltip_text), tooltip);
}

static gboolean tooltip_cb(GtkWidget *widget, gint x, gint y, gboolean keyboard,
                           GtkTooltip *tooltip, CPUGraph *base) {
  gtk_tooltip_set_custom(tooltip, base->tooltip_text);
  return TRUE;
}
 
static void draw_area_cb(GtkWidget *da, cairo_t *cr, gpointer pdata) {
  gint w, h;
	gint x, y;
	gint usage;
  CPUGraph* base = (CPUGraph*)pdata;
  GtkStyleContext *css_context = gtk_widget_get_style_context(GTK_WIDGET(da));

  w = gtk_widget_get_allocated_width(da);
  h = gtk_widget_get_allocated_height(da);

  gtk_render_background (css_context, cr, 0, 0, w, h);
  
	for(x = 0; x < w; x++) {
		usage = h * base->history[w - 1- x] / CPU_SCALE;
		if(usage == 0)
      continue;
    gtk_render_line(css_context, cr, x, h-usage, x, h-1);
	}
}

void set_update_rate(CPUGraph *base, guint rate) {
  guint update;

  base->update_interval = rate;

  if (base->timeout_id)
    g_source_remove(base->timeout_id);
  switch (base->update_interval) {
  case 0:
    update = 250;
    break;
  case 1:
    update = 500;
    break;
  case 2:
    update = 750;
    break;
  default:
    update = 1000;
  }
  base->timeout_id = g_timeout_add(update, update_cb, base);
}

void set_size(CPUGraph *base, guint size) {
  base->size = size;
  size_cb(base->plugin, xfce_panel_plugin_get_size(base->plugin), base);
}

static void rgba_component_to_string(double comp, gchar* out, int start) {
  int icomp = comp * 255;
  const char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                          'a', 'b', 'c', 'd', 'e', 'f' };
  out[start] = digits[icomp / 16];
  out[start + 1] = digits[icomp % 16];
}

static void rgba_to_string(const GdkRGBA* color, gchar* out) {
  out[0] = '#';
  rgba_component_to_string(color->red, out, 1);
  rgba_component_to_string(color->green, out, 3);
  rgba_component_to_string(color->blue, out, 5);
  out[7] = '\0';
}

static void set_color(CPUGraph *base) {
  GtkCssProvider *css_provider;
  gchar css[256];
  
  css_provider = gtk_css_provider_new();
  gtk_style_context_add_provider(GTK_STYLE_CONTEXT(gtk_widget_get_style_context(
                                     GTK_WIDGET(base->draw_area))),
                                 GTK_STYLE_PROVIDER(css_provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_set_data(G_OBJECT(base->draw_area), "css_provider",
                    css_provider);

  g_snprintf(css, sizeof(css), "#graph { color: %s; background-color: %s; }",
             base->fgcolor, base->bgcolor);

  gtk_css_provider_load_from_data(
      g_object_get_data(G_OBJECT(base->draw_area), "css_provider"),
      css, -1, NULL);

}

void set_fg_color(CPUGraph *base, const GdkRGBA *color) {
  rgba_to_string(color, base->fgcolor);
  set_color(base);
}

void set_bg_color(CPUGraph* base, const GdkRGBA *color) {
  rgba_to_string(color, base->bgcolor);
  set_color(base);
}
