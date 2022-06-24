/*
 * ux_init.c - Unix interface, initialisation
 *	Galen Hazelwood <galenh@micron.net>
 *	David Griffith <dgriffi@cs.csubak.edu>
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "frotz/frotz.h"
#include <QMessageBox>
#include "qtfrotzview.h"
#include "qtfrotz.h"

char stripped_story_name[FILENAME_MAX+1];
char semi_stripped_story_name[FILENAME_MAX+1];

QtFrotzView* global_qtfrotzwindow = 0;
int frotz_fatal_error = 0;

/*
 * os_fatal
 *
 * Display error message.
 *
 */

void os_fatal(const char *s)
{
  QString qs = "Fatal error: " + QString(s);
  QMessageBox::critical(global_qtfrotzwindow,"Sorry!",qs);
  frotz_fatal_error = 1;
}

extern char script_name[];
extern char command_name[];
extern char save_name[];
extern char auxilary_name[];

/*
 * os_process_arguments
 *
 * Handle command line switches. Some variables may be set to activate
 * special features of Frotz:
 *
 *     f_setup.attribute_assignment
 *     f_setup.attribute_testing
 *     f_setup.context_lines
 *     f_setup.object_locating
 *     f_setup.object_movement
 *     f_setup.left_margin
 *     f_setup.right_margin
 *     f_setup.ignore_errors
 *     f_setup.piracy
 *     f_setup.undo_slots
 *     f_setup.expand_abbreviations
 *     f_setup.script_cols
 *
 * The global pointer "story_name" is set to the story file name.
 *
 */

void os_process_arguments(int argc, char *argv[])
{
  // Set options.

  f_setup.attribute_assignment = false;
  f_setup.attribute_testing = false;
  f_setup.context_lines = 25;
  f_setup.object_locating = false;
  f_setup.object_movement = false;
  f_setup.left_margin = 1;
  f_setup.right_margin = 2;
  f_setup.ignore_errors = true;
  f_setup.piracy = false;
  f_setup.undo_slots = 20;
  f_setup.expand_abbreviations = true;
  f_setup.script_cols = 80;
  f_setup.err_report_mode = ERR_REPORT_NEVER;

  /* Strip path off the story file name
     This used to use story_name and short_name*/

  char* p = (char *)short_name;

  int i;
  for (i = 0; short_name[i] != 0; i++)
    if (short_name[i] == '/')
      p = (char *)short_name + i + 1;

  for (i = 0; p[i] != '\0'; i++)
    semi_stripped_story_name[i] = p[i];
  semi_stripped_story_name[i] = '\0';

  for (i = 0; p[i] != '\0' && p[i] != '.'; i++)
    stripped_story_name[i] = p[i];
  stripped_story_name[i] = '\0';

  /* Create nice default file names */

  strcpy (script_name, stripped_story_name);
  strcpy (command_name, stripped_story_name);
  strcpy (save_name, stripped_story_name);
  strcpy (auxilary_name, stripped_story_name);

  /* Don't forget the extensions */

  strcat (script_name, ".scr");
  strcat (command_name, ".rec");
  strcat (save_name, ".sav");
  strcat (auxilary_name, ".aux");
}

/*
 * os_init_screen
 *
 * Initialise the IO interface. Prepare the screen and other devices
 * (mouse, sound board). Set various OS depending story file header
 * entries:
 *
 *     h_config (aka flags 1)
 *     h_flags (aka flags 2)
 *     h_screen_cols (aka screen width in characters)
 *     h_screen_rows (aka screen height in lines)
 *     h_screen_width
 *     h_screen_height
 *     h_font_height (defaults to 1)
 *     h_font_width (defaults to 1)
 *     h_default_foreground
 *     h_default_background
 *     h_interpreter_number
 *     h_interpreter_version
 *     h_user_name (optional; not used by any game)
 *
 */


void os_init_screen()
{
  //if (h_version == V3 && user_tandy_bit != 0)
  //  h_config |= CONFIG_TANDY;

  if (h_version == V3)
	  h_config |= CONFIG_SPLITSCREEN | CONFIG_PROPORTIONAL;

  if (h_version >= V4)
	  h_config |= CONFIG_BOLDFACE | CONFIG_EMPHASIS | CONFIG_FIXED | CONFIG_TIMEDINPUT;

  if (h_version >= V5)
  {
    h_flags |= GRAPHICS_FLAG | UNDO_FLAG | MOUSE_FLAG | COLOUR_FLAG;
    h_flags &= ~SOUND_FLAG;
  }

  /*
  if (h_version >= V6)
  {
    h_config |= CONFIG_PICTURES;
    h_flags &= ~MENU_FLAG;
  }
  */

  if (h_version >= V5 && (h_flags & UNDO_FLAG))
    if (f_setup.undo_slots == 0)
      h_flags &= ~UNDO_FLAG;

  h_screen_width  = global_qtfrotzwindow->width();
  h_screen_height = global_qtfrotzwindow->height();

  int w,h;
  os_font_data(FIXED_WIDTH_FONT, &h, &w);

  h_font_height = h;
  h_font_width = w;

  h_screen_rows = h_screen_height / h_font_height;
  h_screen_cols = h_screen_width / h_font_width;

  /* Must be after screen dimensions are computed.  */
  /*
  if (h_version == V6) {
      if (unix_init_pictures())
	h_config |= CONFIG_PICTURES;
      else
	h_flags &= ~GRAPHICS_FLAG;
  }*/

  h_interpreter_number = INTERP_DEC_20;
  h_interpreter_version = 'F';

	// Set these not per spec 8.3.2., but to a wronf value,
	// so that the widget has more control over the colors.
	
	h_default_foreground = 1; // WHITE_COLOUR;
	h_default_background = 1; // BLACK_COLOUR;

  os_set_colour(h_default_foreground, h_default_background);
}



/*
 * os_reset_screen
 *
 * Reset the screen before the program stops.
 *
 */

void os_reset_screen(void)
{
    QMetaObject::invokeMethod(global_qtfrotzwindow,"flushLineBuffer",Qt::BlockingQueuedConnection);
    QMetaObject::invokeMethod(global_qtfrotzwindow,"repaint",Qt::BlockingQueuedConnection);
}



/*
 * os_restart_game
 *
 * This routine allows the interface to interfere with the process of
 * restarting a game at various stages:
 *
 *     RESTART_BEGIN - restart has just begun
 *     RESTART_WPROP_SET - window properties have been initialised
 *     RESTART_END - restart is complete
 *
 */

void os_restart_game(int stage)
{
}



/*
 * os_random_seed
 *
 * Return an appropriate random seed value in the range from 0 to
 * 32767, possiblby using the current system time.
 *
 */

int os_random_seed()
{
  return (time(0) & 0x7fff);
}

