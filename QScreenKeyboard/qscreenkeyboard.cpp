#include "qscreenkeyboard.h"
#include "qkeybutton.h"

#include <QGridLayout>
#include <QPushButton>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QBoxLayout>
#include <QXmlStreamReader>
#include <QFrame>

QScreenKeyboard::QScreenKeyboard(QWidget *parent, QString layout) :
    QWidget(parent),
    isOnePress(false)
{
    QScreenKeyboard::setFont(QFont("u001"));
    mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QFrame * line;
    line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setLineWidth(4);
    mainLayout->addWidget(line);

    if(layout.isEmpty())
        layout = "us";
    loadLayouts(layout);
    QList<QWidget *> widgets = layouts.values();
    for(QList<QWidget *>::Iterator it = widgets.begin(); it != widgets.end(); ++it){
        mainLayout->addWidget(*it);
    }
    showLayout("default");

    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    this->setLayout(mainLayout);
}

void QScreenKeyboard::loadLayouts(QString locale)
{
    QFile f(":/layouts/"+locale+"/layout.xml");
    if(f.open(QIODevice::ReadOnly)){
        QXmlStreamReader xml(&f);
        QWidget *widget = 0;
        QGridLayout *layout = 0;
        while (!xml.atEnd()) {
              xml.readNext();
              if(xml.isStartElement()){
                  QXmlStreamAttributes attributes = xml.attributes();
                  if(xml.name() == "layout"){
                      widget = new QWidget(this);
                      layout = new QGridLayout(this);
                      widget->setLayout(layout);
                      layouts.insert(attributes.value("name").toString(),widget);
                      if(attributes.value("onepress") == "true")
                          onePress.append(attributes.value("name").toString());

                  }else if(xml.name() == "key"){
                      xml.readNext();
                      QString text = xml.text().toString();
                      QString value = text;
                      if(attributes.hasAttribute("value"))
                         value = attributes.value("value").toString();
                      QKeyButton *button = new QKeyButton(text, value, this);
                      if(text == "⏎" or text == "⇧" or text == "↑" or text == "↓" or text == "←" or text == "→" or text == "?") {
                         button->setFont(QFont("Overpass Mono"));
                      }
                      button->setFocusPolicy(Qt::NoFocus);
                      button->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
                      int columnSpan = 1;
                      if(attributes.hasAttribute("colspan")){
                          columnSpan = attributes.value("colspan").toString().toInt();
                      }
                      int rowSpan = 1;
                      if(attributes.hasAttribute("rowspan")){
                          rowSpan = attributes.value("rowspan").toString().toInt();
                      }
                      if(attributes.hasAttribute("modifier")){
                          QString modifier = attributes.value("modifier").toString();
                          if(modifier == "SHIFT")
                            button->setModifier(Qt::ShiftModifier);
                      }
                      int row = attributes.value("row").toString().toInt();
                      int column = attributes.value("column").toString().toInt();
                      layout->addWidget(button,row,column,rowSpan,columnSpan);
                      connect(button,SIGNAL(clicked()),this,SLOT(buttonPressed()));
                  }
              }else if(xml.isEndElement()){
                  if(xml.name() == "layout"){
                      widget = 0;
                      layout = 0;
                  }
              }
        }
        if (xml.hasError()) {
        }
    }
}

void QScreenKeyboard::showLayout(QString layout)
{
    QList<QWidget *> widgets = layouts.values();
    for(QList<QWidget *>::Iterator it = widgets.begin(); it != widgets.end(); ++it){
        (*it)->hide();
    }
    if(layouts.contains(layout))
        layouts.value(layout)->show();
    if(onePress.contains(layout))
        isOnePress = true;
}



void QScreenKeyboard::buttonPressed()
{
    QKeyButton *button = (QKeyButton*)sender();
    Qt::KeyboardModifier modifier = button->modifier();
    QString value = button->value();

    Qt::Key key = (Qt::Key) value.at(0).unicode();
    if (value.at(0).unicode() >= 'a' && value.at(0).unicode() <= 'z') {
      key = (Qt::Key) (value.at(0).unicode() - 'a' + Qt::Key_A);
    }

    if(value.startsWith("{LAYOUT")){
        isOnePress = false;
        showLayout(value.mid(8,value.length()-9));
        return;
    }else if(value == "{ENTER}"){
        key = Qt::Key_Enter;
        value = QString("\n");
    }else if(value == "{BACKSPACE}"){
        key = Qt::Key_Backspace;
        value = QString();
    }else if(value == "{SPACE}"){
        key = Qt::Key_Space;
        value = " ";
    }else if(value == "{DOWN}"){
        key = Qt::Key_Down;
        value = QString();
    }else if(value == "{UP}"){
        key = Qt::Key_Up;
        value = QString();
    }else if(value == "{LEFT}"){
        key = Qt::Key_Left;
        value = QString();
    }else if(value == "{RIGHT}"){
        key = Qt::Key_Right;
        value = QString();
    }

    if(isOnePress){
        showLayout("default");
        isOnePress = false;
    }

    QApplication::sendEvent(QApplication::focusWidget(), new QKeyEvent
          (QEvent::KeyPress, key, modifier, value));
}
