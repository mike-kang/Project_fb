#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <fstream>
#include <QImage>
#include <QPixmap>

#define insertTable(tag)  labelTable.insert(pair<std::string, QLabel*>(#tag, ui->label##tag)) 

using namespace std;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    statusLabel = new QLabel;
    statusBar()->addWidget(statusLabel);

    ui->labelGateLoc->setText("");
    ui->labelGateNo->setText("");
    ui->labelServer->setText("");
    ui->labelRfid->setText("");
    ui->labelRfidNo->setText("");
    ui->labelResult->setText("");
    ui->labelMsg->setText("");
    ui->labelCoName->setText("");
    ui->labelName->setText("");
    ui->labelPinNo->setText("");
    ui->labelPhoto->setText("");
    
    insertTable(GateLoc);
    insertTable(GateNo);
    insertTable(Server);
    insertTable(Rfid);
    //insertTable(CoName);
    //insertTable(Name);
    //insertTable(PinNo);
    insertTable(RfidNo);
    insertTable(Result);
    insertTable(Msg);
    m_img_buf = NULL;
    MainDelegator* md = MainDelegator::createInstance(this);
    //md->setEventListener(this);

    //QDate* date = new QDate();
    QDateTime curDate = QDateTime::currentDateTime();   // 시스템에서 현재 날짜 가져오기
    QString date_string = curDate.toString("yyyy-MM-dd hh:mm:ss"); // QDate 타입을 QString 타입으로 변환
    qDebug() << "current DateTime:" << date_string;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    timer->start(1000);
    
    connect(this, SIGNAL(employeeInfo()), this, SLOT(updateEmployeeInfo()));
    



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onEmployeeInfo(std::string CoName, std::string Name, std::string PinNo, const unsigned char* img_buf, int img_sz)
{
/*
    QMetaObject::invokeMethod(ui->labelCoName, "setText", Q_ARG(QString, CoName.c_str()));
    QMetaObject::invokeMethod(ui->labelName, "setText", Q_ARG(QString, Name.c_str()));
    QMetaObject::invokeMethod(ui->labelPinNo, "setText", Q_ARG(QString, PinNo.c_str()));
    QPixmap pix;
    pix.loadFromData(img_buf, img_sz, "JPG");
    QMetaObject::invokeMethod(ui->labelPhoto, "setPixmap", Q_ARG(QPixmap, pix));
*/
  if(m_img_buf)
    delete m_img_buf;
  m_CoName = CoName.c_str();
  m_Name = Name.c_str();
  m_PinNo = PinNo.c_str();
  m_img_buf = (unsigned char*)img_buf;
  m_img_sz = img_sz;
    
  emit employeeInfo();
}

void MainWindow::onMessage(std::string tag, std::string data)
{
    cout << "onMessage:" << tag << ":" << data << endl;
    for(std::map<std::string, QLabel*>::iterator iter = labelTable.begin(); iter != labelTable.end(); iter++){
      if(iter->first == tag){
        QMetaObject::invokeMethod(iter->second, "setText", Q_ARG(QString, data.c_str()));
      }
        
    }
}

void MainWindow::onStatus(std::string status)
{
  cout << status << endl;
  QMetaObject::invokeMethod(statusLabel, "setText", Q_ARG(QString, status.c_str()));
}

void MainWindow::updateTime()
{
    //qDebug() << "update";
    QDateTime curDate = QDateTime::currentDateTime();
    QString date_string = curDate.toString("yyyy-MM-dd hh:mm:ss");
    ui->labelTime->setText(date_string);
}

void MainWindow::updateEmployeeInfo()
{
  static QPixmap* pix = NULL;
  ui->labelCoName->setText(m_CoName);
  ui->labelName->setText(m_Name);
  //QMetaObject::invokeMethod(ui->labelPinNo, "setText", Q_ARG(QString, PinNo.c_str()));
  qDebug() << m_img_buf;
  //ofstream oOut2("aaa.jpg", ofstream::binary);
  //oOut2.write((const char*)m_img_buf, m_img_sz);
  //oOut2.close();
  if(m_img_buf){
    if(pix) delete pix;
    pix = new QPixmap;
    
    bool ret = pix->loadFromData(m_img_buf, m_img_sz, "JPG");
    qDebug() << ret;
    if(ret){
      ui->labelPhoto->setPixmap(*pix);
      ui->labelPhoto->show();
      return;
    }
  }
  ui->labelPhoto->clear();
}

