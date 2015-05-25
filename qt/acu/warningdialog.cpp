#include "warningdialog.h"
#include "ui_warningdialog.h"
#include <QTimer>
#include <QDebug>
#include <QShowEvent>

WarningDialog::WarningDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WarningDialog)
{
    ui->setupUi(this);
    m_timerClose = new QTimer(this);
    m_timerClose->setSingleShot(true);
    connect(m_timerClose, SIGNAL(timeout()), this, SLOT(close()));
}

WarningDialog::~WarningDialog()
{
    delete ui;
}

void WarningDialog::showEvent(QShowEvent * event)
{
  m_timerClose->start(1500);
}

void WarningDialog::setTexts(const std::string& title, std::string& contents)
{
  QMetaObject::invokeMethod(ui->labelTitle, "setText", Q_ARG(QString, QString::fromLocal8Bit(title.c_str())));
  QMetaObject::invokeMethod(ui->labelContents, "setText", Q_ARG(QString, QString::fromLocal8Bit(contents.c_str())));
}



