/*  mode.h
 *  Part of xfce4-cpugraph-plugin
 *
 *  Copyright (c) Alexander Nordfelth <alex.nordfelth@telia.com>
 *  Copyright (c) gatopeich <gatoguan-os@yahoo.com>
 *  Copyright (c) 2007-2008 Angelo Arrifano <miknix@gmail.com>
 *  Copyright (c) 2007-2008 Lidiriel <lidiriel@coriolys.org>
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __XFCE_MODE_H__
#define __XFCE_MODE_H__

#include "cpu.h"

typedef struct {
  long x;
  long y;
} point;


void drawGraphNormal(CPUGraph *base, GdkGC *fg1, GtkWidget *da, int w, int h);
void drawGraphLED(CPUGraph *base, GdkGC *fg1, GdkGC *fg2, GtkWidget *da, int w, int h);
void drawGraphNoHistory(CPUGraph *base, GdkGC *fg1, GdkGC *fg2, GtkWidget *da, int w, int h);
void drawGraphGrid(CPUGraph *base, GdkGC *fg1, GdkGC *fg2, GtkWidget *da, int w, int h);

#endif