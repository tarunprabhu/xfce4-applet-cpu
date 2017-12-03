/*  cpu.h
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
#ifndef _XFCE_CPU_H_
#define _XFCE_CPU_H_

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <libxfce4panel/xfce-panel-plugin.h>

#include "os.h"

typedef struct
{
	/* GUI components */
	XfcePanelPlugin *plugin;
  GtkWidget *title;
  GtkWidget **temps;
  GtkWidget *frame;
	GtkWidget *grid;
	GtkWidget *draw_area;
	GtkWidget *box;
	GtkWidget *fgbutton;
  GtkWidget *bgbutton;
	GtkWidget *tooltip_text;

	/* Settings */
	guint update_interval; /* Number of ms between updates. */
	guint size;
	gchar fgcolor[8];
  gchar bgcolor[8];

	/* Runtime data */
	guint nr_cores;
  guint nr_temps;
	guint timeout_id;
	guint *history;
	gssize history_size;
  CPUData *cpu_data;
  TemperatureData temp_data[MAX_TEMPERATURES];
} CPUGraph;

void set_update_rate(CPUGraph *base, guint rate);
void set_size(CPUGraph *base, guint width);
void set_fg_color(CPUGraph *base, const GdkRGBA*);
void set_bg_color(CPUGraph *base, const GdkRGBA*);

#endif /* !_XFCE_CPU_H_ */
