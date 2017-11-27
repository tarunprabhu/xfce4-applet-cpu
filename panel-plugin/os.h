/*  os.h
 *  Part of xfce4-applet-cpu
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
#ifndef _XFCE_OS_H_
#define _XFCE_OS_H_

#define CPU_SCALE 256

#include <glib.h>

typedef struct
{
	guint load;
	guint64 previous_used;
	guint64 previous_total;
} CpuData;

typedef struct {
  gint64 temp;
  gchar file[PATH_MAX];
} TemperatureData;

guint detect_cpu_number();
gboolean read_cpu_data( CpuData *data, guint nb_cpu );
guint init_temperature_data();
gboolean read_temperature_data(TemperatureData* data, guint nr_temps);

#endif /* !_XFCE_OS_H */
