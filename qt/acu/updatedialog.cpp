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

void UpdateDialog::setLocalDBCount(int delete_count, int update_count, int insert_count)
{
  QMetaObject::invokeMethod(ui->labelCountDelete, "setText", Q_ARG(QString, QString::number(delete_count)));
  QMetaObject::invokeMethod(ui->labelCountUpdate, "setText", Q_ARG(QString, QString::number(update_count)));
  QMetaObject::invokeMethod(ui->labelCountInsert, "setText", Q_ARG(QString, QString::number(insert_count)));
}

void UpdateDialog::setFBCount(int delete_count, int save_count)
{
  QMetaObject::invokeMethod(ui->labelFB_D, "setText", Q_ARG(QString, QString::number(delete_count)));
  QMetaObject::invokeMethod(ui->labelFB_S, "setText", Q_ARG(QString, QString::number(save_count)));

  if(!delete_count)
    QMetaObject::invokeMethod(ui->progressBar_D, "setEnabled", Q_ARG(bool, false));
  else
    QMetaObject::invokeMethod(ui->progressBar_D, "setMaximum", Q_ARG(int, delete_count));
  if(!save_count)
    QMetaObject::invokeMethod(ui->progressBar_S, "setEnabled", Q_ARG(bool, false));
  else
    QMetaObject::invokeMethod(ui->progressBar_S, "setMaximum", Q_ARG(int, save_count));
}

void UpdateDialog::setFBIndexOfDelete(int index)
{
  QMetaObject::invokeMethod(ui->progressBar_D, "setValue", Q_ARG(int, index + 1));
}
void UpdateDialog::setFBIndexOfSave(int index)
{
  QMetaObject::invokeMethod(ui->progressBar_S, "setValue", Q_ARG(int, index + 1));
}
