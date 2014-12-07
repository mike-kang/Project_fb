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

void UpdateDialog::setCount(int count)
{
  QMetaObject::invokeMethod(ui->labelCount, "setText", Q_ARG(QString, QString::number(count)));
  QMetaObject::invokeMethod(ui->progressBar, "setMaximum", Q_ARG(int, count));
}

void UpdateDialog::setIndex(int index)
{
  QMetaObject::invokeMethod(ui->progressBar, "setValue", Q_ARG(int, index + 1));

}

