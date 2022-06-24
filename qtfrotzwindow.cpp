#include "qtfrotzwindow.h"
#include "ui_qtfrotzwindow.h"

QtFrotzWindow::QtFrotzWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QtFrotzWindow)
{
    ui->setupUi(this);
}

QtFrotzWindow::~QtFrotzWindow()
{
    delete ui;
}

void QtFrotzWindow::on_actionExit_triggered()
{
    ui->frotzView->quit();
}

void QtFrotzWindow::on_actionSave_triggered()
{
    ui->frotzView->save();
}

void QtFrotzWindow::on_actionSaveSlot1_triggered()
{
    ui->frotzView->save("1");
}

void QtFrotzWindow::on_actionSaveSlot2_triggered()
{
    ui->frotzView->save("2");
}

void QtFrotzWindow::on_actionSaveSlot3_triggered()
{
    ui->frotzView->save("3");
}

void QtFrotzWindow::on_actionSaveSlot4_triggered()
{
    ui->frotzView->save("4");
}

void QtFrotzWindow::on_actionSaveSlot5_triggered()
{
    ui->frotzView->save("5");
}

void QtFrotzWindow::on_actionSaveSlot6_triggered()
{
    ui->frotzView->save("6");
}

void QtFrotzWindow::on_actionSaveSlot7_triggered()
{
    ui->frotzView->save("7");
}

void QtFrotzWindow::on_actionSaveSlot8_triggered()
{
    ui->frotzView->save("8");
}

void QtFrotzWindow::on_actionSaveSlot9_triggered()
{
    ui->frotzView->save("9");
}

void QtFrotzWindow::on_actionRestore_triggered()
{
    ui->frotzView->restore();
}

void QtFrotzWindow::on_actionRestoreSlot1_triggered()
{
    ui->frotzView->restore("1");
}

void QtFrotzWindow::on_actionRestoreSlot2_triggered()
{
    ui->frotzView->restore("2");
}

void QtFrotzWindow::on_actionRestoreSlot3_triggered()
{
    ui->frotzView->restore("3");
}

void QtFrotzWindow::on_actionRestoreSlot4_triggered()
{
    ui->frotzView->restore("4");
}

void QtFrotzWindow::on_actionRestoreSlot5_triggered()
{
    ui->frotzView->restore("5");
}

void QtFrotzWindow::on_actionRestoreSlot6_triggered()
{
    ui->frotzView->restore("6");
}

void QtFrotzWindow::on_actionRestoreSlot7_triggered()
{
    ui->frotzView->restore("7");
}

void QtFrotzWindow::on_actionRestoreSlot8_triggered()
{
    ui->frotzView->restore("8");
}

void QtFrotzWindow::on_actionRestoreSlot9_triggered()
{
    ui->frotzView->restore("9");
}
void QtFrotzWindow::on_actionHelp_triggered()
{
    ui->frotzView->help();
}

void QtFrotzWindow::on_actionSettings_triggered()
{
    ui->frotzView->settings();
}

void QtFrotzWindow::on_frotzView_gameStart()
{
    ui->actionSave->setEnabled(true);
    ui->actionSaveSlot1->setEnabled(true);
    ui->actionSaveSlot2->setEnabled(true);
    ui->actionSaveSlot3->setEnabled(true);
    ui->actionSaveSlot4->setEnabled(true);
    ui->actionSaveSlot5->setEnabled(true);
    ui->actionSaveSlot6->setEnabled(true);
    ui->actionSaveSlot7->setEnabled(true);
    ui->actionSaveSlot8->setEnabled(true);
    ui->actionSaveSlot9->setEnabled(true);
    ui->actionRestore->setEnabled(true);
    ui->actionRestoreSlot1->setEnabled(true);
    ui->actionRestoreSlot2->setEnabled(true);
    ui->actionRestoreSlot3->setEnabled(true);
    ui->actionRestoreSlot4->setEnabled(true);
    ui->actionRestoreSlot5->setEnabled(true);
    ui->actionRestoreSlot6->setEnabled(true);
    ui->actionRestoreSlot7->setEnabled(true);
    ui->actionRestoreSlot8->setEnabled(true);
    ui->actionRestoreSlot9->setEnabled(true);
    ui->actionHelp->setEnabled(true);
    ui->actionSettings->setEnabled(false);
    ui->actionStart->setEnabled(false);
}

void QtFrotzWindow::on_frotzView_gameQuit()
{
    ui->actionSave->setEnabled(false);
    ui->actionSaveSlot1->setEnabled(false);
    ui->actionSaveSlot2->setEnabled(false);
    ui->actionSaveSlot3->setEnabled(false);
    ui->actionSaveSlot4->setEnabled(false);
    ui->actionSaveSlot5->setEnabled(false);
    ui->actionSaveSlot6->setEnabled(false);
    ui->actionSaveSlot7->setEnabled(false);
    ui->actionSaveSlot8->setEnabled(false);
    ui->actionSaveSlot9->setEnabled(false);
    ui->actionRestore->setEnabled(false);
    ui->actionRestoreSlot1->setEnabled(false);
    ui->actionRestoreSlot2->setEnabled(false);
    ui->actionRestoreSlot3->setEnabled(false);
    ui->actionRestoreSlot4->setEnabled(false);
    ui->actionRestoreSlot5->setEnabled(false);
    ui->actionRestoreSlot6->setEnabled(false);
    ui->actionRestoreSlot7->setEnabled(false);
    ui->actionRestoreSlot8->setEnabled(false);
    ui->actionRestoreSlot9->setEnabled(false);
    ui->actionHelp->setEnabled(false);
    ui->actionSettings->setEnabled(true);
    ui->actionStart->setEnabled(true);
}
