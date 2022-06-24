#ifndef KOSETTINGSDIALOG_H
#define KOSETTINGSDIALOG_H

#include <QDialog>
#include <QHash>
#include <QVariant>

namespace Ui {
class KoSettingsDialog;
}

class KoSettingsDialog : public QDialog
{
    Q_OBJECT

public:

    enum Setting{
        FontSize
    };

    typedef QHash<Setting, QVariant> Settings;

    explicit KoSettingsDialog(Settings settings = Settings(), QWidget *parent = 0);
    ~KoSettingsDialog();

    Settings getResult(){return result;}

    static Settings getSettings(Settings settings = Settings());

private slots:
    void on_btnOk_clicked();
    void on_spinBoxFontSize_valueChanged(int value);

private:
    Ui::KoSettingsDialog *ui;
    Settings settings;
    Settings result;

};

#endif // KOSETTINGSDIALOG_H
