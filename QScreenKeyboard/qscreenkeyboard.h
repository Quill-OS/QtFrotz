#ifndef QSCREENKEYBOARD_H
#define QSCREENKEYBOARD_H

#include <QWidget>
#include <QKeyEvent>
#include <QGridLayout>
#include <QHash>

class QScreenKeyboard : public QWidget
{
    Q_OBJECT
public:
    explicit QScreenKeyboard(QWidget *parent = 0,QString layout = QString());

private:
    void loadLayouts(QString locale);
    void showLayout(QString layout);

    QBoxLayout *mainLayout;

    QHash<QString, QWidget *> layouts;
    QList<QString> onePress;
    bool isOnePress;

signals:

public slots:


private slots:
    void buttonPressed();

};

#endif // QSCREENKEYBOARD_H
