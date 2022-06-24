#ifndef QtFrotzView_H
#define QtFrotzView_H

#include <QWidget>
#include <QList>
#include <QTimer>

#include "kosettingsdialog.h"

namespace Ui {
class QtFrotzView;
}

class QtFrotzView : public QWidget
{
    Q_OBJECT
    public:

      typedef enum {None, Key, Line, Mouse, Timer, Exit} EventType;

      QtFrotzView(QWidget *parent = 0);
      ~QtFrotzView();

public slots:
      int getFontData(int font, int *height, int *width) const;

      void setXY(int  x, int  y)
        {
          flushLineBuffer();
          xPos = x;
          yPos = y;
          //writeInputLine(x,"",0,0);
      }

      void erase_screen();

      void setDefaultBgColor(const QColor& c) {defaultBgColor = c;}
      QColor getDefaultBgColor() const {return defaultBgColor;}
      void setBgColor(int c) {flushLineBuffer(); bgColor = colorMap[c];}
      void setBgColor(const QColor& c) {flushLineBuffer();bgColor = c;}
      void resetBgColor()              {bgColor = defaultBgColor;}

      void setDefaultFgColor(const QColor& c) {defaultFgColor = c;}
      QColor getDefaultFgColor() const {return defaultFgColor;}
      void setFgColor(int c) {flushLineBuffer(); fgColor = colorMap[c];}
      void setFgColor(const QColor& c) {flushLineBuffer(); fgColor = c;}
      void resetFgColor()              {fgColor = defaultFgColor;}

      void setTextFont (const QFont& f);
      void setFixedFont(const QFont& f);

      QFont getTextFont()  const {return  textFont[0][0];}
      QFont getFixedFont() const {return fixedFont[0][0];}

      void selectFontAndStyle(int font, int style);

      int charWidth(QChar c) const {return fm?fm->width(c):0;}

      void writeString(QString s);
      void writeString(QChar c);
      void flushLineBuffer();

      unsigned char readChar(int timeout = 0);
      unsigned char readLine(int max, unsigned char *buf,
      int timeout, int width, int continued);

      void forceInput(const QString& s) {forcedInput.append(s); event=Key;}

      void writeInputLine(int x, char* text, int len, int cursorpos, QChar deletedChar = QChar());

      void morePrompt();

      void  erase_area(int top, int left, int bottom, int right);
      void scroll_area(int top, int left, int bottom, int right, int units);
      int getWidth(){return width();}
      int getHeight(){return height();}

      void quit();
      void help();
      void save(QString slot = QString());
      void restore(QString slot = QString());
      void settings();

      QString getFileName(QString defaultFileName,int flag);

    protected:

      int xPos; // cursor position in Qt coordinates,
      int yPos; // i.e. starting from (0,0)

      void loadSettings();
      void saveSettings();

      void paintEvent(QPaintEvent* e);
      void resizeEvent(QResizeEvent*);
      bool clearedOnInit;

      void keyPressEvent(QKeyEvent* e);
      unsigned char key;
      QList<QString> forcedInput;
      EventType event;

      QColor colorMap[13];

      QColor defaultFgColor, fgColor;
      QColor defaultBgColor, bgColor;
      bool reverse;

      QFont textFont[2][2];
      QFont fixedFont[2][2];

      QFontMetrics* fm;
      int fontHeight, padding;

      QPixmap screenBuffer;
      QString lineBuffer;

      QTimer timer;

      QString quicksaveloadslot;

      bool running;
      QString currentFileName;
      QString currentStoryName;

      KoSettingsDialog::Settings curSettings;

    protected slots:

     void timedOut();
     void start();
     void run(QString filename, QString shortname);

signals:
     void gameStart();
     void gameQuit();
};

#endif // QtFrotzView_H
