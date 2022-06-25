#include "kosettingsdialog.h"
#include "ui_kosettingsdialog.h"

KoSettingsDialog::KoSettingsDialog(Settings settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KoSettingsDialog),
    settings(settings)
{
    ui->setupUi(this);

    ui->spinBoxFontSize->setFont(QFont("u001"));
    ui->btnOk->setFont(QFont("u001"));
    ui->label->setFont(QFont("u001"));

    ui->btnOk->setStyleSheet("padding: 10px");

    ui->spinBoxFontSize->setValue(settings.value(FontSize).toInt());
    showFullScreen();
}

KoSettingsDialog::~KoSettingsDialog()
{
    delete ui;
}

KoSettingsDialog::Settings KoSettingsDialog::getSettings(KoSettingsDialog::Settings settings)
{
    KoSettingsDialog dialog(settings);
    dialog.exec();
    return dialog.getResult();
}

void KoSettingsDialog::on_btnOk_clicked()
{
    result = settings;
    close();
}

void KoSettingsDialog::on_spinBoxFontSize_valueChanged(int value)
{
    settings.insert(FontSize,value);
}
