#ifndef KFILEDIALOG_H
#define KFILEDIALOG_H

#include <QDialog>
#include <QDir>
#include <QModelIndex>
#include <QFileSystemModel>

namespace Ui {
class KoFileDialog;
}

class KoFileDialog : public QDialog
{
    Q_OBJECT

public:
    enum SelectionMode{
        ExistingFiles, AllFiles
    };

    explicit KoFileDialog(QString defaultPath = QDir::currentPath(), QWidget *parent = 0);

    static QString getSaveFile(QString defaultPath = "", QStringList filters = QStringList(), QString caption = "");
    static QString getOpenFile(QString defaultPath = "", QStringList filters = QStringList(), QString caption = "");

    QString getResult(){return result;}

    bool eventFilter ( QObject * target, QEvent * event );

    void setOkButtonText(QString text);
    void setSelectionMode(SelectionMode mode){selectionMode = mode;}
    void addFilter(QString filter);
    void clearFilter();
    void setFilters(QStringList filters);
signals:

private slots:
   void on_btnUp_clicked();
   void on_btnCancel_clicked();
   void on_btnOk_clicked();
   void on_listViewFiles_clicked(const QModelIndex &index);

private:
   void changePath(QString newPath);

    Ui::KoFileDialog *ui;
    QFileSystemModel *model;
    QString result;
    SelectionMode selectionMode;
    int mouseDownY;
};

#endif // KFILEDIALOG_H
