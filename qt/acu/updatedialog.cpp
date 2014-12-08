#include "updatedialog.h"
#include "ui_updatedialog.h"

UpdateDialog::UpdateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);
    
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

void UpdateDialog::setCount(int delete_count, int update_count, int insert_count)
{
  QMetaObject::invokeMethod(ui->labelCountDelete, "setText", Q_ARG(QString, QString::number(delete_count)));
  QMetaObject::invokeMethod(ui->labelCountUpdate, "setText", Q_ARG(QString, QString::number(update_count)));
  QMetaObject::invokeMethod(ui->labelCountInsert, "setText", Q_ARG(QString, QString::number(insert_count)));
  if(!delete_count)
    QMetaObject::invokeMethod(ui->progressBar_D, "setEnabled", Q_ARG(bool, false));
  else
    QMetaObject::invokeMethod(ui->progressBar_D, "setMaximum", Q_ARG(int, delete_count));
  QMetaObject::invokeMethod(ui->progressBar_U, "setMaximum", Q_ARG(int, update_count));
  QMetaObject::invokeMethod(ui->progressBar_I, "setMaximum", Q_ARG(int, insert_count));
}

void UpdateDialog::setIndexOfDelete(int index)
{
  QMetaObject::invokeMethod(ui->progressBar_D, "setValue", Q_ARG(int, index + 1));
}
void UpdateDialog::setIndexOfUpdate(int index)
{
  QMetaObject::invokeMethod(ui->progressBar_U, "setValue", Q_ARG(int, index + 1));
}
void UpdateDialog::setIndexOfInsert(int index)
{
  QMetaObject::invokeMethod(ui->progressBar_I, "setValue", Q_ARG(int, index + 1));
}

