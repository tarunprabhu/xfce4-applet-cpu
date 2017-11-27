/*  settings.c
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
#include "settings.h"

#include <glib.h>
#include <gtk/gtk.h>

#include <libxfce4ui/libxfce4ui.h>

void read_settings(XfcePanelPlugin *plugin, CPUGraph *base) {
  const char *value;
  char *file;
  XfceRc *rc;

  guint rate = 0;

  GdkRGBA fg;
  GdkRGBA bg;
  guint size;

  gdk_rgba_parse(&fg, "#ff0000");
  gdk_rgba_parse(&bg, "#ffffff");
  size = xfce_panel_plugin_get_size(plugin);

  if ((file = xfce_panel_plugin_lookup_rc_file(plugin)) != NULL) {
    rc = xfce_rc_simple_open(file, TRUE);
    g_free(file);
    
    if (rc) {
      rate = xfce_rc_read_int_entry(rc, "UpdateInterval", rate);
      size = xfce_rc_read_int_entry(rc, "Size", size);
      
      if ((value = xfce_rc_read_entry(rc, "Foreground", NULL)))
        gdk_rgba_parse(&fg, value);
      if ((value = xfce_rc_read_entry(rc, "Background", NULL)))
        gdk_rgba_parse(&bg, value);
      
      xfce_rc_close(rc);
    }
  }
  
  set_update_rate(base, rate);
  set_size(base, size);
  set_fg_color(base, &fg);
  set_bg_color(base, &bg);
}

void write_settings(XfcePanelPlugin *plugin, CPUGraph *base) {
  XfceRc *rc;
  char *file;

  if (!(file = xfce_panel_plugin_save_location(plugin, TRUE)))
    return;

  rc = xfce_rc_simple_open(file, FALSE);
  g_free(file);

  if (!rc)
    return;

  xfce_rc_write_int_entry(rc, "UpdateInterval", base->update_interval);
  xfce_rc_write_int_entry(rc, "Size", base->size);

  xfce_rc_write_entry(rc, "Foreground", base->fgcolor);
  xfce_rc_write_entry(rc, "Background", base->bgcolor);

  xfce_rc_close(rc);
}
