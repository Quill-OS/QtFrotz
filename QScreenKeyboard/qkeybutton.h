#ifndef QKEYBUTTON_H
#define QKEYBUTTON_H

#include <QPushButton>

class QKeyButton : public QPushButton
{
    Q_OBJECT
public:
    explicit QKeyButton(QString text, QString value, QWidget *parent = 0):QPushButton(text,parent),m_value(value),m_modifier(Qt::NoModifier){if(text == "&") setText("&&");}


    QString value(){return m_value;}
    Qt::KeyboardModifier modifier(){return m_modifier;}
    void setModifier(Qt::KeyboardModifier modifier){m_modifier = modifier;}

private:
    QString m_value;
    Qt::KeyboardModifier m_modifier;

};

#endif // QKEYBUTTON_H
