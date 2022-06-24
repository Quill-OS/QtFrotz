/*
 * ux_screen.c - Unix interface, screen manipulation
 *
 * This file is part of Frotz.
 *
 * Frotz is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Frotz is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "frotz/frotz.h"
#include "qtfrotz.h"

/*
 * os_erase_area
 *
 * Fill a rectangular area of the screen with the current background
 * colour. Top left coordinates are (1,1). The cursor does not move.
 *
 */

void os_erase_area(int top, int left, int bottom, int right)
{

    QMetaObject::invokeMethod(global_qtfrotzwindow,"erase_area",Qt::BlockingQueuedConnection,Q_ARG(int,top-1),Q_ARG(int,left-1),Q_ARG(int,bottom-1),Q_ARG(int,right-1));
}



/*
 * os_scroll_area
 *
 * Scroll a rectangular area of the screen up (units > 0) or down
 * (units < 0) and fill the empty space with the current background
 * colour. Top left coordinates are (1,1). The cursor stays put.
 *
 */

void os_scroll_area(int top, int left, int bottom, int right, int units)
{
  if (units == 0)
    return;

  QMetaObject::invokeMethod(global_qtfrotzwindow,"scroll_area",Qt::BlockingQueuedConnection,Q_ARG(int,top-1),Q_ARG(int,left-1),Q_ARG(int,bottom-1),Q_ARG(int,right-1),Q_ARG(int,units));
}
