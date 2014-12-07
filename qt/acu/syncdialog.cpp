#include "syncdialog.h"
#include "ui_syncdialog.h"

SyncDialog::SyncDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SyncDialog)
{
    ui->setupUi(this);
    
}

SyncDialog::~SyncDialog()
{
    delete ui;
}

void SyncDialog::setCount(int count)
{
  QMetaObject::invokeMethod(ui->labelCount, "setText", Q_ARG(QString, QString::number(count)));
  QMetaObject::invokeMethod(ui->progressBar, "setMaximum", Q_ARG(int, count));
}

void SyncDialog::setIndex(int index)
{
  QMetaObject::invokeMethod(ui->progressBar, "setValue", Q_ARG(int, index + 1));

}

