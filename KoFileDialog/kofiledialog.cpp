#include "kofiledialog.h"
#include "ui_kofiledialog.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QSwipeGesture>
#include <QScreen>

KoFileDialog::KoFileDialog(QString defaultPath, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KoFileDialog),
    selectionMode(AllFiles)
{
    ui->setupUi(this);

    ui->lblCurrentPath->setFont(QFont("u001"));
    ui->textEditFilename->setFont(QFont("u001"));
    ui->btnOk->setFont(QFont("u001"));
    ui->btnCancel->setFont(QFont("u001"));
    ui->btnUp->setFont(QFont("Wnter"));
    ui->listViewFiles->setFont(QFont("u001"));

    ui->lblCurrentPath->setStyleSheet("padding: 10px");
    ui->textEditFilename->setStyleSheet("padding: 10px");
    ui->btnOk->setStyleSheet("padding: 10px");
    ui->btnCancel->setStyleSheet("padding: 10px");
    ui->btnUp->setStyleSheet("padding: 10px");

    model = new QFileSystemModel();
    model->setNameFilterDisables(false);
    model->setRootPath(defaultPath);
    ui->listViewFiles->setModel(model);
    setMouseTracking(true);
    ui->qScreenKeyboard->hide();

    const QScreen * screen = qApp->primaryScreen();
    this->setGeometry(QRect(QPoint(0,0), screen->geometry().size()));

    ui->textEditFilename->installEventFilter(this);
    ui->btnOk->installEventFilter(this);
    ui->listViewFiles->installEventFilter(this);
    ui->listViewFiles->viewport()->installEventFilter(this);
    ui->btnCancel->installEventFilter(this);
    ui->btnUp->installEventFilter(this);
    changePath(defaultPath);
}

QString KoFileDialog::getSaveFile(QString defaultPath, QStringList filters, QString caption)
{
    KoFileDialog dialog("/app-data");
    dialog.setOkButtonText(tr("Save"));
    dialog.setFilters(filters);
    dialog.exec();
    return dialog.getResult();
}

QString KoFileDialog::getOpenFile(QString defaultPath, QStringList filters, QString caption)
{
    QString path;
    if(caption == "Restore game...") {
        path = "/app-data";
    }
    else {
        path = "/app-misc/games";
    }

    KoFileDialog dialog(path);
    dialog.setOkButtonText(tr("Open"));
    dialog.setSelectionMode(ExistingFiles);
    dialog.setFilters(filters);
    dialog.exec();
    return dialog.getResult();
}

bool KoFileDialog::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->textEditFilename && event->type() == QEvent::FocusIn){
            ui->qScreenKeyboard->show();
    }else if((target == ui->listViewFiles||target == ui->btnCancel||target == ui->btnUp) && event->type() == QEvent::FocusIn){
        ui->qScreenKeyboard->hide();
    }else if(target == ui->btnOk && event->type() == QEvent::FocusIn){
        if(ui->qScreenKeyboard->isVisible())
            ui->textEditFilename->setFocus();
    }else if(target == ui->listViewFiles || target == ui->listViewFiles->viewport()){
        if(event->type() == QEvent::MouseButtonPress){
            QMouseEvent *mevent = static_cast<QMouseEvent *>(event);
            mouseDownY = mevent->globalY();
        }else if(event->type() == QEvent::MouseButtonRelease){
            if(mouseDownY==-1) return false;
            QMouseEvent *mevent = static_cast<QMouseEvent *>(event);
            int deltaY = mouseDownY - mevent->globalY();
            if(abs(deltaY)>=100){
                if(deltaY<0){
                    QModelIndex index = ui->listViewFiles->indexAt(QPoint(0,0));
                    ui->listViewFiles->scrollTo(index,QListView::PositionAtBottom);
                }else{
                    int y = ui->listViewFiles->height();
                    QModelIndex index = ui->listViewFiles->indexAt(QPoint(0,y));
                    ui->listViewFiles->scrollTo(index,QListView::PositionAtTop);
                }
            }
            mouseDownY = -1;
        }
    }
    return false;
}

void KoFileDialog::setOkButtonText(QString text)
{
    ui->btnOk->setText(text);
}

void KoFileDialog::addFilter(QString filter)
{
    model->setNameFilters(model->nameFilters()<<filter);
}

void KoFileDialog::clearFilter()
{
    model->setNameFilters(QStringList());
}

void KoFileDialog::setFilters(QStringList filters)
{
    model->setNameFilters(filters);
}

void KoFileDialog::on_btnUp_clicked()
{
    QString path = QFileInfo(ui->listViewFiles->rootIndex().data(QFileSystemModel::FilePathRole).toString()).absoluteDir().absolutePath();
    changePath(path);
}

void KoFileDialog::on_btnCancel_clicked()
{
    result = QString();
    close();
}

void KoFileDialog::on_btnOk_clicked()
{
    QString path = QString("%1/%2").arg(ui->listViewFiles->rootIndex().data(QFileSystemModel::FilePathRole).toString(),ui->textEditFilename->toPlainText());
    QString name = ui->textEditFilename->toPlainText();
    if(selectionMode == ExistingFiles && !QFileInfo(path).exists()){
        return;
    }else if(selectionMode == AllFiles && QFileInfo(path).exists() && QMessageBox::question( this, "File exists",QString("File %1 already exists, Do you want to override?").arg(name), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes ) == QMessageBox::No){
        return;
    }
    result = path;
    close();
}

void KoFileDialog::changePath(QString newPath)
{
    ui->listViewFiles->setRootIndex(model->index(newPath));
    ui->lblCurrentPath->setText(newPath);
}

void KoFileDialog::on_listViewFiles_clicked(const QModelIndex &index)
{
    QString path = index.data(QFileSystemModel::FilePathRole).toString();
    if(QDir(path).exists()){
        changePath(path);
    }else{
        QString name = index.data(QFileSystemModel::FileNameRole).toString();
        ui->textEditFilename->setText(name);
    }
}
