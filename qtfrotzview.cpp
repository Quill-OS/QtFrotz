
// include files for Qt
//#include <QPrinter>
#include <QPainter>

#include <QApplication>
#include <QPaintEvent>
#include <QtCore>
#include <QBrush>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>

#include <QDebug>

// application specific includes
#include "qtfrotzview.h"

#include "frotz/frotz.h"
#include "qtfrotz.h"
#include "kofiledialog.h"

#include <iostream>

extern "C" bool is_terminator(zchar);
extern "C" int completion(const zchar *, zchar *);
extern "C" int frotz_main(const char* filename, const char*);


void unix_add_to_history(zchar *str);
int unix_history_back(zchar *str, int searchlen, int maxlen);
int unix_history_forward(zchar *str, int searchlen, int maxlen);
extern char **history_next;
extern char **history_view;

////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::QtFrotzView
//
////////////////////////////////////////////////////////////////////////////

QtFrotzView::QtFrotzView(QWidget *parent)
  : QWidget(parent),
    running(false)
{
  global_qtfrotzwindow = this;
  setAttribute(Qt::WA_OpaquePaintEvent,true);
  setFocusPolicy(Qt::StrongFocus);

  key = 0;

  lineBuffer = "";

  clearedOnInit = false;

  // Create colormap.

  colorMap[0] = Qt::white;
  colorMap[1] = Qt::black;

  colorMap[BLACK_COLOUR]      = Qt::black;
  colorMap[RED_COLOUR]        = Qt::red;
  colorMap[GREEN_COLOUR]      = Qt::green;
  colorMap[YELLOW_COLOUR]     = Qt::yellow;
  colorMap[BLUE_COLOUR]       = Qt::blue;
  colorMap[MAGENTA_COLOUR]    = Qt::magenta;
  colorMap[CYAN_COLOUR]       = Qt::cyan;
  colorMap[WHITE_COLOUR]      = Qt::white;
  colorMap[GREY_COLOUR]       = Qt::lightGray;
  colorMap[MEDIUMGREY_COLOUR] = Qt::gray;
  colorMap[DARKGREY_COLOUR]   = Qt::darkGray;

  reverse = false;

  setDefaultBgColor(Qt::white);
  setDefaultFgColor(Qt::black);
  //setBackgroundMode(NoBackground);
  fm = new QFontMetrics(textFont[0][0]);

  connect(&timer, SIGNAL(timeout()), this, SLOT(timedOut()));

  event = None;
  screenBuffer = QPixmap(width(),height());

  loadSettings();
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::~QtFrotzView
//
////////////////////////////////////////////////////////////////////////////

QtFrotzView::~QtFrotzView()
{
  delete fm;
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::erase_screen
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::erase_screen()
{
  erase_area(1,1,height(),width());
  update();
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::getFontData
//
////////////////////////////////////////////////////////////////////////////

int QtFrotzView::getFontData(int font, int *height, int *width) const
{
  if (font != TEXT_FONT && font != FIXED_WIDTH_FONT)
    return 0; // Font not present.

  *height = fontHeight; // Frotz prefers all fonts the same height.

  if (font == TEXT_FONT)
    *width  = QFontMetrics( textFont[0][0]).maxWidth();
  if (font == FIXED_WIDTH_FONT)
    *width  = QFontMetrics(fixedFont[0][0]).maxWidth();

  return 1;
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::setTextFont
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::setTextFont(const QFont& f)
{
  // Frotz does not like fonts of different height, so we'll make
  // them the same height by playing with the line distance.
  // See also 'padding' in setFontAndStyle.

  const int  text_h = QFontMetrics(              f).height();
  const int fixed_h = QFontMetrics(fixedFont[0][0]).height();

  fontHeight = (text_h > fixed_h) ? text_h : fixed_h;

  // Create text fonts.

  QFont f_bold      = f;             f_bold.setBold(true);
  QFont f_emph      = f;           f_emph.setItalic(true);
  QFont f_bold_emph = f_bold; f_bold_emph.setItalic(true);

  textFont[0][0] = f;
  textFont[1][0] = f_bold;
  textFont[0][1] = f_emph;
  textFont[1][1] = f_bold_emph;
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::setFixedFont
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::setFixedFont(const QFont& f)
{
  // Frotz does not like fonts of different height, so we'll make
  // them the same height by playing with the line distance.
  // See also 'padding' in setFontAndStyle.

  const int  text_h = QFontMetrics(textFont[0][0]).height();
  const int fixed_h = QFontMetrics(             f).height();

  fontHeight = (text_h > fixed_h) ? text_h : fixed_h;

  // Create fixed fonts.

  QFont f_bold      = f;             f_bold.setBold(true);
  QFont f_emph      = f;           f_emph.setItalic(true);
  QFont f_bold_emph = f_bold; f_bold_emph.setItalic(true);

  fixedFont[0][0] = f;
  fixedFont[1][0] = f_bold;
  fixedFont[0][1] = f_emph;
  fixedFont[1][1] = f_bold_emph;
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::selectFontAndStyle
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::selectFontAndStyle(int font, int style)
{
  flushLineBuffer();

  reverse = (style & REVERSE_STYLE);

  bool b = style & BOLDFACE_STYLE;
  bool e = style & EMPHASIS_STYLE;

  if (style & FIXED_WIDTH_STYLE)
    font = FIXED_WIDTH_FONT;

  QFont* f;
  if (font == TEXT_FONT)
    f = &textFont[b][e];
  else if (font == FIXED_WIDTH_FONT)
    f = &fixedFont[b][e];
  else
    return;

  setFont(*f);

  delete fm;
  fm = new QFontMetrics(*f);

  padding = (fm->height() < fontHeight)
    ? int( (fontHeight - fm->height()) / 2)
    : 0;
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::writeString
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::writeString(QString s)
{
    lineBuffer += s;
}

void QtFrotzView::writeString(QChar c)
{
    lineBuffer += c;
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::flushLineBuffer
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::flushLineBuffer()
{
  if (lineBuffer.length() == 0)
    return;

  QPainter p(&screenBuffer);
  // Always erase background first.

  QColor bg = reverse ? fgColor : bgColor;
  p.fillRect(xPos, yPos, fm->width(lineBuffer), fontHeight, bg);

  // Draw text but do not update screen yet.

  p.setPen(reverse ? bgColor : fgColor);
  p.setFont(textFont[0][0]);
  p.drawText(xPos, yPos + fm->ascent()-1 + padding, lineBuffer);

  xPos += fm->width(lineBuffer);

  lineBuffer = "";
  p.end();
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::QtFrotzView
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::writeInputLine(int x, char* text, int len, int cursorpos, QChar deletedChar)
{
  int xCursor = x;

  QString qs;
  for (int i=0; i<len; i++)
  {
    if (i == cursorpos)
      xCursor += fm->width(qs);

     qs += text[i];
  }

  if (cursorpos == len)
    xCursor += fm->width(qs);

  if(!isVisible()) return;
  QPainter p(&screenBuffer);
  QColor bg = reverse ? fgColor : bgColor;
  if(deletedChar.isNull())
        p.fillRect(x, yPos, fm->width(qs)+1, fontHeight, bg);
    else
        p.fillRect(x, yPos, fm->width(qs+deletedChar)+1, fontHeight, bg);

  p.setPen(reverse ? bgColor : fgColor);
  p.setFont(textFont[0][0]);
  p.drawText(x, yPos + fm->ascent()-1 + padding, qs);
  xPos += fm->width(qs);

  if (cursorpos != -1)
    p.drawLine(xCursor, yPos+1+padding, xCursor, yPos+fm->height()-1);

    update();
    //qDebug() << "x:" << x  << "len:" << len << "cursorpos:" << cursorpos << "xPos:" << xPos << "xPos0:" << xPos0 << "yPos:" << yPos << "text:" << text;
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::readChar
//
////////////////////////////////////////////////////////////////////////////

unsigned char QtFrotzView::readChar(int timeout)
{
  // The original code read as follows:
  //
  //  if (timeout && !timer.isActive())
  //    timer.start(timeout, true);
  //
  //  while (!event)
  //    kapp->processOneEvent();
  //
  // This causes problems where a timeout read is called, followed very
  // shortly by a non-timeout read.  The timeout is still active from
  // the first, and since the second is not expecting to ever receive
  // timeout, the VM is prone to crashing or hanging should the timeout
  // be received.  Also, I'm not sure that the timeout should not be
  // reset on every timed read, not just if inactive... (SB)

  if (timeout)				// Always start timer on timed read
    timer.start(timeout);

  while (!event){			// Wait for any notable event
     qApp->processEvents();
  }

  if (timeout)				// Ensure timeout stopped
    timer.stop();

  if (!forcedInput.isEmpty())
    return ZC_RETURN; // let readLine deal with it.

  event = None;

  return key;
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::readLine
//
//  buf can contain preloaded text which is already printed.
//
////////////////////////////////////////////////////////////////////////////

unsigned char QtFrotzView::readLine(int max, zchar *buf, int timeout,
  int width, int continued)
{
  flushLineBuffer();
  update();

  // These are static to allow input continuation to work smoothly.

  static int scrpos = 0, searchpos = -1, insert_flag = 1;

  // Find start of input area.

  QString preloaded;
  for (int i=0; buf[i]; i++)
    preloaded += QChar(buf[i]);

  xPos -= fm->width(preloaded);

  int xPos0 = xPos;

  if (width < max)
    max = width;

  /* Better be careful here or it might segv.  I wonder if we should just
     ignore 'continued' and check for len > 0 instead?  Might work better
     with Beyond Zork. */

  int len = strlen((char *)buf);
  if (!(continued && scrpos <= len && searchpos <= len))
  {
    scrpos = len;
    history_view = history_next; /* Reset user's history view. */
    searchpos = -1;              /* -1 means initialize from len. */
    insert_flag = 1;             /* Insert mode is now default. */
  }

  for (;;)
  {
    // Repaint input line.

    writeInputLine(xPos0, (char*)buf, len, scrpos);
    xPos = xPos0; // We'll update the cursos position later.

    // Process key.

    zchar ch;

    if (forcedInput.isEmpty())
      ch = readChar(100*timeout);

    // forcedInput is some kludgy machinery where we force some
    // input into the interpreter, like hotkeys for quitting or
    // restarting.

    if (!forcedInput.isEmpty())
    {
      QString forced = forcedInput.first();
      strcpy((char*)buf, forced.toLatin1().constData());
      len = forced.length();

      forcedInput.removeOne(*forcedInput.begin());

      // If we have pushed two strings in forcedInput,
      // the second one is a confirmation key by design.

      if (!forcedInput.isEmpty() and (forcedInput.first().toLatin1().constData())[0] == 'y')
      {
        forcedInput.removeOne(*forcedInput.begin());
        key = 'y';
        event = Key;
      }

      if ( (buf[0] == ZC_HKEY_QUIT) || (buf[0] == ZC_HKEY_RESTART) )
        return buf[0];

      writeInputLine(xPos0, (char*)buf, len, -1);

      return ZC_RETURN;
    }

    switch (ch)
    {
      case ZC_BACKSPACE: /* Delete preceeding character */
        if (scrpos != 0)
          {
            QString qbuf = QString::fromLatin1((char*)buf);
              len--; scrpos--; searchpos = -1;
              memmove(buf + scrpos, buf + scrpos + 1, len - scrpos);
              writeInputLine(xPos0, (char*)buf, len, scrpos,qbuf.at(len));
          }
          break;

        case DEL_CHAR: /* Delete following character */
          if (scrpos < len)
          {
          len--; searchpos = -1;
          memmove(buf + scrpos, buf + scrpos + 1, len - scrpos);
          }
          continue;

        case EOL_CHAR: /* Delete from cursor to end of line.  */
        len = scrpos;
          continue;

        case ZC_ESCAPE: /* Delete whole line */
        len = scrpos = 0;
          searchpos = -1;
          history_view = history_next;
          continue;

        case ZC_ARROW_LEFT:
          if (scrpos)
            scrpos--;
          continue;

        case ZC_ARROW_RIGHT:
          if (scrpos < len)
            scrpos++;
          continue;

        case HOME_CHAR:
          scrpos = 0;
          continue;

        case END_CHAR:
          scrpos = len;
          continue;

        case INS_CHAR: /* Insert Character */
          if (insert_flag == true)
          {
            insert_flag = false;
            //((KwestApp*) parentWidget())->setINSMsg("OVR");
          }
          else
          {
            insert_flag = true;
            //((KwestApp*) parentWidget())->setINSMsg("INS");
          }
          continue;

        case ZC_ARROW_UP:
        case ZC_ARROW_DOWN:
          if (searchpos < 0)
              searchpos = len;
          if ((ch == ZC_ARROW_UP ? unix_history_back : unix_history_forward)
              (buf, searchpos, max))
            {
              scrpos = len = strlen((char *) buf);
        }
          continue;

        /* Passthrough as up/down arrows for Beyond Zork. */

        case PGUP_CHAR:
          ch = ZC_ARROW_UP;
          break;

        case PGDN_CHAR:
          ch = ZC_ARROW_DOWN;
          break;

        case '\t':
        {
            zchar saved_char = buf[scrpos];
            buf[scrpos] = '\0';

            zchar extension[10];
            int status = completion(buf, extension);
            buf[scrpos] = saved_char;

            if (status != 2)
            {
              int ext_len = strlen((char *) extension);
              if (ext_len > max - len)
              {
                  ext_len = max - len;
                  status = 1;
              }
              memmove(buf + scrpos + ext_len, buf + scrpos,len - scrpos);
              memmove(buf + scrpos, extension, ext_len);
              scrpos += ext_len;
              len += ext_len;
              searchpos = -1;
            }

            if (status)
              os_beep(1);

          continue;	/* TAB is invalid as an input character. */
        }

        default:

          if ((ch >= ZC_ASCII_MIN  && ch <= ZC_ASCII_MAX) or
            (ch >= ZC_LATIN1_MIN && ch <= ZC_LATIN1_MAX) )
            {
              searchpos = -1;
              if ((scrpos == max) || (insert_flag && (len == max)))
              {
                os_beep(1);
                continue;
              }

              if (insert_flag && (scrpos < len))
                memmove(buf + scrpos + 1, buf + scrpos, len - scrpos);

              if (insert_flag || scrpos == len)
                len++;

              buf[scrpos++] = ch;
              continue;
          }
    } // end switch

    if (is_terminator(ch))
    {
      buf[len] = '\0';

      // Repaint input line, but without cursor.

      writeInputLine(xPos0, (char*)buf, len, -1);

      // Expand abbreviations.

      if ( (len == 1) || (buf[1] == ' ') )
      {
        static zchar examine[] = "examine";
        static zchar again[]   = "again";
        static zchar wait[]    = "wait";

        zchar* buf2 = new zchar[INPUT_BUFFER_SIZE];
        memcpy(buf2,buf,len+1);

        switch(buf[0])
        {
          case ('x'):
            memcpy(buf,examine,7);
            memcpy(buf+7,buf2+1,len);
            break;
          case ('g'):
            memcpy(buf,again,5);
            memcpy(buf+5,buf2+1,len);
            break;
          case ('z'):
            memcpy(buf,wait,4);
            memcpy(buf+4,buf2+1,len);
          default:
            break;
        }

        delete buf2;
      }

      if (ch == ZC_RETURN)
        unix_add_to_history(buf);

      return ch;
    }
  }
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::morePrompt
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::morePrompt()
{
  // Save current line.

  flushLineBuffer();
  repaint();

  QPixmap b(width(), fontHeight);
  QPainter(&b).drawPixmap(0,0,screenBuffer,0,yPos,width(),fontHeight);

  // Write [MORE] prompt.

  selectFontAndStyle(TEXT_FONT, 0);

  QColor bg = reverse ? fgColor : bgColor;
  QPainter p(&screenBuffer);
  p.fillRect(0, yPos, width(), fontHeight, bg);

  p.setPen(reverse ? bgColor : fgColor);
  p.setFont(textFont[0][0]);
  p.drawText(0, yPos + fm->ascent()-1 + padding, "[MORE]");

  // Wait for keypress and restore current line.

  readChar();
  p.drawPixmap(0,yPos,b,0,0,width(),fontHeight);
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::erase_area
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::erase_area(int top, int left, int bottom, int right)
{
  QPainter p(&screenBuffer);
  p.fillRect(left, top, right - left + 1, bottom - top + 1, bgColor);
  update();
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::scroll_area
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::scroll_area(int top, int left, int bottom, int right,
  int units)
{
  flushLineBuffer();

  if (!units)
    return;
  if(!isVisible()){
      return;
  }

  QPainter p(&screenBuffer);
  if (units > bottom - top || units < top - bottom)
    p.fillRect(left, top, right - left + 1, bottom - top + 1, bgColor);
  else
    if (units > 0)
    {
        p.drawPixmap(left,top,screenBuffer,left, top + units,
                                          right - left + 1, bottom - top - units + 1);

      p.fillRect(left, bottom - units, right - left + 1, units + 1, bgColor);
    }
    else
    {
        p.drawPixmap(left, top + units,
                                          screenBuffer, left, top,
                                                         right - left + 1, bottom - top + units + 1);

      p.fillRect(left, top, right - left + 1, - units + 1, bgColor);
    }
  update();
}

void QtFrotzView::quit()
{
    if(!running){
        qApp->quit();
    }
    event = Key;
    key = ZC_HKEY_QUIT;
}

void QtFrotzView::help()
{
    event = Key;
    key = ZC_HKEY_HELP;
}

void QtFrotzView::save(QString slot)
{
    if(!running) return;
    quicksaveloadslot = slot;
    forceInput("SAVE");
}

void QtFrotzView::restore(QString slot)
{
    if(!running) return;
    quicksaveloadslot = slot;
    forceInput("RESTORE");
}

void QtFrotzView::settings()
{
    curSettings = KoSettingsDialog::getSettings(curSettings);
    saveSettings();
}

QString QtFrotzView::getFileName(QString defaultFileName, int flag)
{
    if(!running) return QString();
    bool save = false;

    QString filter, caption, extension;
    switch (flag)
    {
      case FILE_SAVE:
        filter  = QObject::tr("*.sav");
        caption = QObject::tr("Save game...");
        save = true;
        break;
      case FILE_RESTORE:
        filter  = QObject::tr("*.sav");
        caption = QObject::tr("Restore game...");
        break;
      case FILE_SCRIPT:
        filter  = QObject::tr("*.src");
        caption = QObject::tr("Save script...");
        save = true;
        break;
      case FILE_RECORD:
        filter  = QObject::tr("*.rec");
        caption = QObject::tr("Record commands...");
        break;
      case FILE_PLAYBACK:
        filter  = QObject::tr("*.rec");
        caption = QObject::tr("Play back commands...");
        break;
      case FILE_SAVE_AUX:
        filter  = QObject::tr("*.aux");
        caption = QObject::tr("Save auxiliary file...");
        save = true;
        break;
      case FILE_LOAD_AUX:
        filter  = QObject::tr("*.aux");
        caption = QObject::tr("Restore auxiliary file...");
        break;
    }

    if(quicksaveloadslot.isEmpty() || (flag != FILE_RESTORE && flag != FILE_SAVE)){
        if(save)
            return KoFileDialog::getSaveFile(defaultFileName,QStringList()<<filter,caption);
        else
            return KoFileDialog::getOpenFile(defaultFileName,QStringList()<<filter,caption);
    }else{
        QString slot = QString("%1_SLOT%2.%3").arg(currentStoryName,quicksaveloadslot,extension);
        quicksaveloadslot.clear();
        return slot;
    }
}

void QtFrotzView::loadSettings()
{
    QSettings settings("frotz.ini",QSettings::IniFormat,this);
    curSettings.insert(KoSettingsDialog::FontSize,settings.value("FontSize","8"));
}

void QtFrotzView::saveSettings()
{
    QSettings settings("frotz.ini",QSettings::IniFormat,this);
    settings.setValue("FontSize",curSettings.value(KoSettingsDialog::FontSize,8));
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::resizeEvent
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::resizeEvent(QResizeEvent* e)
{
  screenBuffer = QPixmap(width(),height());
  erase_screen();
  if(running){
      if (!clearedOnInit)
      {
          QPainter p(&screenBuffer);
          p.fillRect(0,0,width(),height(), bgColor);
        clearedOnInit = true;
      }

      os_init_screen();
      flushLineBuffer();
  }
  update();
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::paintEvent
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::paintEvent(QPaintEvent* e)
{
  QRect r = e->rect();
  QPainter p(this);
  p.fillRect(0,0,width(),height(),Qt::red);
  p.drawPixmap(r.left(),r.top(), screenBuffer, r.left(), r.top(),
                            r.width(), r.height());
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::keyPressEvent
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::keyPressEvent(QKeyEvent* e)
{
  event = Key;

  // Standard case.

  key = e->key();

//  // Frotz hotkeys.

  if (e->modifiers() | Qt::AltModifier)
    switch (key)
    {
      case 'p': key = ZC_HKEY_PLAYBACK; return;
      case 'r': key = ZC_HKEY_RECORD;   return;
      case 's': key = ZC_HKEY_SEED;     return;
      case 'u': key = ZC_HKEY_UNDO;     return;
      case 'n': key = ZC_HKEY_RESTART;  return;
      case 'x': key = ZC_HKEY_QUIT;     return;
      case 'd': key = ZC_HKEY_DEBUG;    return;
      case 'h': key = ZC_HKEY_HELP;     return;
    }

//  /* These are the Emacs-editing characters. */

//  if (e->state() & Qt::ControlButton)
//    switch (key)
//    {
//      case 'b': key = ZC_ARROW_LEFT;  return;
//      case 'f': key = ZC_ARROW_RIGHT; return;
//      case 'p': key = ZC_ARROW_UP;    return;
//      case 'n': key = ZC_ARROW_DOWN;  return;
//      case 'a': key = HOME_CHAR;      return;
//      case 'e': key = END_CHAR;       return;
//      case 'd': key = DEL_CHAR;       return;
//      case 'k': key = EOL_CHAR;       return;
//    }

  /* Special keys. */

  switch (e->key())
  {
    case Qt::Key_Enter:     key = ZC_RETURN;        return;
    case Qt::Key_Return:    key = ZC_RETURN;        return;
    case Qt::Key_Backspace: key = ZC_BACKSPACE;     return;
    case Qt::Key_Delete:    key = DEL_CHAR;         return;
    case Qt::Key_Insert:    key = INS_CHAR;         return;
    case Qt::Key_Up:        key = ZC_ARROW_UP;      return;
    case Qt::Key_Down:      key = ZC_ARROW_DOWN;    return;
    case Qt::Key_Left:      key = ZC_ARROW_LEFT;    return;
    case Qt::Key_Right:     key = ZC_ARROW_RIGHT;   return;
    case Qt::Key_PageUp:    key = PGUP_CHAR;        return;
    case Qt::Key_PageDown:  key = PGDN_CHAR;        return;
    case Qt::Key_F1:        key = ZC_FKEY_MIN;      return;
    case Qt::Key_F2:        key = ZC_FKEY_MIN + 1;  return;
    case Qt::Key_F3:        key = ZC_FKEY_MIN + 2;  return;
    case Qt::Key_F4:        key = ZC_FKEY_MIN + 3;  return;
    case Qt::Key_F5:        key = ZC_FKEY_MIN + 4;  return;
    case Qt::Key_F6:        key = ZC_FKEY_MIN + 5;  return;
    case Qt::Key_F7:        key = ZC_FKEY_MIN + 6;  return;
    case Qt::Key_F8:        key = ZC_FKEY_MIN + 7;  return;
    case Qt::Key_F9:        key = ZC_FKEY_MIN + 8;  return;
    case Qt::Key_F10:       key = ZC_FKEY_MIN + 9;  return;
    case Qt::Key_F11:       key = ZC_FKEY_MIN + 10; return;
    case Qt::Key_F12:       key = ZC_FKEY_MIN + 11; return;
  }

  // Empirically, a QKeyEvent's ascii() method returns 0 if the key pressed
  // has no ASCII associated with it, for example, Shift.  Unfortunately,
  // this matches ZC_TIME_OUT, and if the call to clients did not specify
  // a timeout, then receiving ZC_TIME_OUT back confuses them, and generally
  // crashes or hangs the virtual machine.  So... we'll forget all about
  // this event if we get to here, by simply resetting event back to None.
  // The readChar() loop "while (!event)..." will therefore not terminate
  // on such QKeyEvents. (SB)

  if (e->text().isEmpty())
    event = None;
}



////////////////////////////////////////////////////////////////////////////
//
// QtFrotzView::timedOut
//
////////////////////////////////////////////////////////////////////////////

void QtFrotzView::timedOut()
{
  event = Timer;
  key = ZC_TIME_OUT;
}

void QtFrotzView::start()
{
    QString path = KoFileDialog::getOpenFile(QString(),QStringList()<<"*.z?"<<"*.dat"<<"*.zblorb"<<"*.zlb");
    if(path.isEmpty()) return;

    setTextFont(QFont(font().family(),curSettings.value(KoSettingsDialog::FontSize).toInt()));
    setFixedFont(QFont(font().family(),curSettings.value(KoSettingsDialog::FontSize).toInt()));
    QtConcurrent::run(this,&QtFrotzView::run,path,QFileInfo(path).baseName());
}

void QtFrotzView::run(QString filename, QString shortname)
{
    emit gameStart();
    running = true;
    currentFileName = filename;
    currentStoryName = shortname;
    frotz_main(filename.toLatin1().constData(),shortname.toLatin1().constData());
    erase_screen();
    running = false;
    currentFileName.clear();
    currentStoryName.clear();
    emit gameQuit();
}
