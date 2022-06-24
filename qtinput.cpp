/*
 * ux_input.c - Unix interface, input functions
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
#include <string.h>
#include <limits.h>
#include <ctype.h>

#include "frotz/frotz.h"
#include "qtfrotz.h"


/* These are useful for circular buffers. */
#define RING_DEC( ptr, beg, end) (ptr > (beg) ? --ptr : (ptr = (end)))
#define RING_INC( ptr, beg, end) (ptr < (end) ? ++ptr : (ptr = (beg)))

#define MAX_HISTORY 20

char *history_buffer[MAX_HISTORY];
char **history_next = history_buffer; /* Next available slot. */
char **history_view = history_buffer; /* What the user is looking at. */

#define history_end (history_buffer + MAX_HISTORY - 1)


/*
 * unix_add_to_history
 *
 * Add the given string to the next available history buffer slot.
 *
 */

void unix_add_to_history(zchar *str)
{
  if (*history_next != NULL)
    free( *history_next);

  *history_next = (char *)malloc(strlen((char *)str) + 1);
  strcpy( *history_next, (char *)str);
  RING_INC( history_next, history_buffer, history_end);
  history_view = history_next; /* Reset user frame after each line */
}



/*
 * unix_history_back
 *
 * Copy last available string to str, if possible.  Return 1 if successful.
 * Only lines of at most maxlen characters will be considered.  In addition
 * the first searchlen characters of the history entry must match those of str.
 */

int unix_history_back(zchar *str, int searchlen, int maxlen)
{
  char **prev = history_view;

  do
  {
	  RING_DEC(history_view, history_buffer, history_end);
	  if ((history_view == history_next) || (*history_view == NULL))
	  {
	    os_beep(1);
	    history_view = prev;
	    return 0;
	  }
  }
  while (strlen(*history_view) > maxlen
	     || (searchlen != 0 && strncmp( (char *)str, *history_view, searchlen)));

  strcpy((char *)str + searchlen, *history_view + searchlen);
  return 1;
}



/*
 * unix_history_forward
 *
 * Opposite of unix_history_back, and works in the same way.
 */

int unix_history_forward(zchar *str, int searchlen, int maxlen)
{
  char **prev = history_view;

  do
  {
	  RING_INC(history_view, history_buffer, history_end);
	  if ((history_view == history_next) || (*history_view == NULL))
	  {
	    os_beep(1);
	    history_view = prev;
	    return 0;
	  }
  }
  while (strlen(*history_view) > maxlen
	     || (searchlen != 0 && strncmp( (char *)str, *history_view, searchlen)));

  strcpy((char *)str + searchlen, *history_view + searchlen);
  return 1;
}



/*
 * os_read_line
 *
 * Read a line of input from the keyboard into a buffer. The buffer
 * may already be primed with some text. In this case, the "initial"
 * text is already displayed on the screen. After the input action
 * is complete, the function returns with the terminating key value.
 * The length of the input should not exceed "max" characters plus
 * an extra 0 terminator.
 *
 * Terminating keys are the return key (13) and all function keys
 * (see the Specification of the Z-machine) which are accepted by
 * the is_terminator function. Mouse clicks behave like function
 * keys except that the mouse position is stored in global variables
 * "mouse_x" and "mouse_y" (top left coordinates are (1,1)).
 *
 * Furthermore, Frotz introduces some special terminating keys:
 *
 *     ZC_HKEY_KEY_PLAYBACK (Alt-P)
 *     ZC_HKEY_RECORD (Alt-R)
 *     ZC_HKEY_SEED (Alt-S)
 *     ZC_HKEY_UNDO (Alt-U)
 *     ZC_HKEY_RESTART (Alt-N, "new game")
 *     ZC_HKEY_QUIT (Alt-X, "exit game")
 *     ZC_HKEY_DEBUGGING (Alt-D)
 *     ZC_HKEY_HELP (Alt-H)
 *
 * If the timeout argument is not zero, the input gets interrupted
 * after timeout/10 seconds (and the return value is ZC_TIME_OUT).
 *
 * The complete input line including the cursor must fit in "width"
 * screen units.
 *
 * The function may be called once again to continue after timeouts,
 * misplaced mouse clicks or hot keys. In this case the "continued"
 * flag will be set. This information can be useful if the interface
 * implements input line history.
 *
 * The screen is not scrolled after the return key was pressed. The
 * cursor is at the end of the input line when the function returns.
 *
 * Since Inform 2.2 the helper function "completion" can be called
 * to implement word completion (similar to tcsh under Unix).
 *
 */

zchar os_read_line(int max, zchar *buf, int timeout, int width, int continued)
{
    zchar ret;
    QMetaObject::invokeMethod(global_qtfrotzwindow,
                          "readLine",
                          Qt::BlockingQueuedConnection,
                          Q_RETURN_ARG(unsigned char, ret),
                              Q_ARG(int, max),
                              Q_ARG(unsigned char *, buf),
                              Q_ARG(int, timeout),
                              Q_ARG(int, width),
                              Q_ARG(int, continued)
                          );
    return ret;
//  return global_qtfrotzwindow->readLine(max, buf, timeout, width, continued);
}



/*
 * os_read_key
 *
 * Read a single character from the keyboard (or a mouse click) and
 * return it. Input aborts after timeout/10 seconds.
 *
 */

zchar os_read_key(int timeout, int cursor)
{
  QtFrotzView* kv = global_qtfrotzwindow;

  kv->flushLineBuffer();
  kv->update();

  return kv->readChar(100*timeout);
}



/*
 * os_read_file_name
 *
 * Return the name of a file. Flag can be one of:
 *
 *    FILE_SAVE     - Save game file
 *    FILE_RESTORE  - Restore game file
 *    FILE_SCRIPT   - Transscript file
 *    FILE_RECORD   - Command file for recording
 *    FILE_PLAYBACK - Command file for playback
 *    FILE_SAVE_AUX - Save auxilary ("preferred settings") file
 *    FILE_LOAD_AUX - Load auxilary ("preferred settings") file
 *
 * The length of the file name is limited by MAX_FILE_NAME. Ideally
 * an interpreter should open a file requester to ask for the file
 * name. If it is unable to do that then this function should call
 * print_string and read_string to ask for a file name.
 *
 */

int os_read_file_name(char *file_name, const char *default_name, int flag)
{
  // Turn off playback and recording temporarily.

  int saved_replay = istream_replay;
  int saved_record = ostream_record;

  istream_replay = 0;
  ostream_record = 0;



  // Get filename.
  QString filename;
  QMetaObject::invokeMethod(global_qtfrotzwindow,
                        "getFileName",
                        Qt::BlockingQueuedConnection,
                        Q_RETURN_ARG(QString, filename),
                            Q_ARG(QString, QString(default_name)),
                            Q_ARG(int, flag)
                        );



  if (filename.length() == 0)
    return 0;
  else if (filename.length() >= MAX_FILE_NAME-1)
  {
    os_display_string((const zchar*)"Filename too long.");
    return -1;
  }

  strcpy(file_name, filename.toLatin1().constData());

  // Restore state of playback and recording.

  istream_replay = saved_replay;
  ostream_record = saved_record;

  return 1;
}



/*
 * os_read_mouse
 *
 * Store the mouse position in the global variables "mouse_x" and
 * "mouse_y" and return the mouse buttons currently pressed.
 *
 */

zword os_read_mouse(void)
{
	/* INCOMPLETE */

}
