#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <fstream>
#include <QImage>
#include <QPixmap>
#include <QMessageBox>


#define insertTable(tag)  labelTable.insert(pair<std::string, QLabel*>(#tag, ui->label##tag)) 

using namespace std;

QTextCodec* MainWindow::m_codec = NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) , m_syncDialog(this), m_updateDialog(this)
{
    //m_codec = QTextCodec::codecForName("UTF-8"); //UTF-8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    ui->setupUi(this);
    m_statusLabel = new QLabel;
    statusBar()->addWidget(m_statusLabel);

    ui->labelEmbed->setText("");
    ui->labelGateNo->setText("");
    ui->labelNetwork->setText("");
    ui->labelFID->setText("");
    ui->labelDownload->setText("");
    //ui->labelUpload->setText("");
    ui->labelMsg->setText("");
    ui->labelCoName->setText("");
    ui->labelName->setText("");
    //ui->labelPinNo->setText("");
    ui->labelImage->setText("");  //logo or pass/fail

    m_qlePinNo = ui->lineEditPinNo;
    m_qlePinNo->setText("");
    m_qlePinNo->setValidator(new QIntValidator(m_qlePinNo));
	
    insertTable(Embed);
    insertTable(GateNo);
    insertTable(Network);
    insertTable(FID);
    //insertTable(CoName);
    //insertTable(Name);
    //insertTable(PinNo);
    //insertTable(RfidNo);
    //insertTable(Result);
    insertTable(Msg);
    insertTable(Download);
    insertTable(UploadFilesCount);
    insertTable(UploadCacheCount);
    m_img_buf = NULL;
    m_aninfinger = new QMovie("/home/pi/acufb/Images/finger.gif");
    ui->labelAnimation->setMovie(m_aninfinger);
    m_aninfinger->start();

    m_pm_auth_pass = QPixmap(":/Images/authok.jpg");
    m_pm_auth_fail = QPixmap(":/Images/authfail.jpg");

    //connect(this, SIGNAL(sigStartSync()), &m_syncDialog, SLOT(exec()));
    connect(this, SIGNAL(sigStartSync()), this, SLOT(startSync()));
    connect(this, SIGNAL(sigEndSync()), this, SLOT(endSync()));
    connect(this, SIGNAL(sigStartUpdate()), this, SLOT(startUpdate()));
    connect(this, SIGNAL(sigEndUpdate()), this, SLOT(endUpdate()));
    connect(this, SIGNAL(sigWarning()), this, SLOT(warning()));

    //MainDelegator* md = MainDelegator::createInstance(this);
    //md->setEventListener(this);

    //QDate* date = new QDate();
    QDateTime curDate = QDateTime::currentDateTime();   // 시스템에서 현재 날짜 가져오기
    QString date_string = curDate.toString("yyyy-MM-dd hh:mm:ss"); // QDate 타입을 QString 타입으로 변환
    qDebug() << "current DateTime:" << date_string;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    timer->start(1000);
    
    connect(this, SIGNAL(employeeInfo()), this, SLOT(updateEmployeeInfo()));
    connect(this, SIGNAL(resultImage()), this, SLOT(displayResultImage()));

    m_timerEmployeeInfo = new QTimer(this);
    connect(m_timerEmployeeInfo, SIGNAL(timeout()), this, SLOT(cleanInfo()));
    qDebug() << "MainWindow ---";


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::runMainDelegator(const char* config)
{
  cout << config << endl;
  
  MainDelegator* md = MainDelegator::createInstance(this, config);
}

void MainWindow::onEmployeeInfo(std::string CoName, std::string Name, std::string PinNo)
{
  m_CoName = QString::fromLocal8Bit(CoName.c_str());
  m_Name = QString::fromLocal8Bit(Name.c_str());
  //m_PinNo = PinNo.c_str();
    
  emit employeeInfo();
}

void MainWindow::onSyncStart()
{
  cout << "onSyncStart1" << endl;
  
  emit sigStartSync();
}

void MainWindow::onSyncCount(int count)
{
  cout << "onSyncCount" << endl;
  m_syncDialog.setCount(count);
}

void MainWindow::onSyncIndex(int index)
{
  m_syncDialog.setIndex(index);
}

void MainWindow::onSyncEnd(bool val)
{
  cout << "onSyncEnd" << endl;
  emit sigEndSync();
}

void MainWindow::onUpdateStart()
{
  cout << "onUpdateStart" << endl;
  
  emit sigStartUpdate();
}

void MainWindow::onUpdateCount(int delete_count, int update_count, int insert_count)
{
  cout << "onUpdateCount" << endl;
  m_updateDialog.setLocalDBCount(delete_count, update_count, insert_count);
}

void MainWindow::onUpdateLocalDBEnd(const char* updatetime)
{
  cout << "onUpdateLocalDBEnd" << endl;
  QMetaObject::invokeMethod(ui->labelLUT, "setText", Q_ARG(QString, updatetime));
}

void MainWindow::onUpdateFBBegin(int delete_count, int save_count)
{
  m_updateDialog.setFBCount(delete_count, save_count);
}

void MainWindow::onUpdateFBDeleteIndex(int index)
{
  m_updateDialog.setFBIndexOfDelete(index);
}
void MainWindow::onUpdateFBSaveIndex(int index)
{
  m_updateDialog.setFBIndexOfSave(index);
}
void MainWindow::onUpdateFBEnd()
{
  cout << "onUpdateFBEnd" << endl;
  emit sigEndUpdate();
}


void MainWindow::onMessage(std::string tag, std::string data)
{
  //cout << "onMessage:" << tag << ":" << data << endl;
  for(std::map<std::string, QLabel*>::iterator iter = labelTable.begin(); iter != labelTable.end(); iter++){
    if(iter->first == tag){
      QMetaObject::invokeMethod(iter->second, "setText", Q_ARG(QString, QString::fromLocal8Bit(data.c_str())));
    }
  }
  /*
  if(m_timerEmployeeInfo->isActive())
    m_timerEmployeeInfo->stop();
  m_timerEmployeeInfo->start(5000);
  */
}

void MainWindow::onLogo(std::string data)
{
  if(data == "LT"){
    m_pm_logo = QPixmap(":/Images/logo_lt.bmp");
  }
  else{
    m_pm_logo = QPixmap(":/Images/logo.bmp");
  }
  
  //m_pm_logo = m_pm_logo.scaled(img->
  ui->labelImage->setPixmap(m_pm_logo);
  //ui->labelImage->setPixmap(m_pm_auth_pass);
}

void MainWindow::onStatus(std::string status)
{
  cout << "onStatus:" << status << endl;
  QMetaObject::invokeMethod(m_statusLabel, "setText", Q_ARG(QString, status.c_str()));
}

void MainWindow::onImage(bool val)
{
  cout << "onImage" << endl;
  if(val)
    m_pm_auth = &m_pm_auth_pass;
  else
    m_pm_auth = &m_pm_auth_fail;
  emit resultImage();
}

const char* MainWindow::onGetPinNo()
{
  cout << "onGetPinNo" << endl;
  return m_qlePinNo->text().toStdString().c_str();
}

void MainWindow::onWarning(std::string msg1, std::string msg2)
{
  cout << "onWarning:" << msg1 << endl;
  m_warningDialog.setTexts(msg1, msg2);
  emit sigWarning();
}

void MainWindow::warning()
{
  qDebug() << "warning";
  m_warningDialog.exec();
}

void MainWindow::updateTime()
{
    //qDebug() << "update";
    QDateTime curDate = QDateTime::currentDateTime();
    QString date_string = curDate.toString("yyyy-MM-dd hh:mm:ss");
    ui->labelTime->setText(date_string);
}

void MainWindow::startSync()
{
    cout << "startSync" << endl;
    m_aninfinger->stop();
    m_syncDialog.exec();
}
void MainWindow::endSync()
{
    cout << "endSync" << endl;
    m_aninfinger->start();
    m_syncDialog.close();
}

void MainWindow::startUpdate()
{
    cout << "startUpdate" << endl;
    m_aninfinger->stop();
    m_updateDialog.exec();
}
void MainWindow::endUpdate()
{
    cout << "endUpdate" << endl;
    m_aninfinger->start();
    m_updateDialog.close();
}

void MainWindow::updateEmployeeInfo()
{
  //static QPixmap* pix = NULL;
  //if(m_timerEmployeeInfo->isActive())
  //  m_timerEmployeeInfo->stop();
  m_timerEmployeeInfo->start(5000);
  ui->labelCoName->setText(m_CoName);
  ui->labelName->setText(m_Name);
  //ui->labelPinNo->setText(m_PinNo);
  //ui->labelPhoto->clear();
}

void MainWindow::cleanInfo()
{
  qDebug() << "cleanEmployeeInfo";
  ui->labelCoName->setText("");
  ui->labelName->setText("");
  ui->labelImage->setPixmap(m_pm_logo);
  ui->labelMsg->setText("");
  m_qlePinNo->setText("");
  m_timerEmployeeInfo->stop();
}

void MainWindow::displayResultImage()
{
  //ui->labelImage->setPixmap(m_Name);
  QMetaObject::invokeMethod(ui->labelImage, "setPixmap", Q_ARG(QPixmap, *m_pm_auth));
  
  if(m_timerEmployeeInfo->isActive())
    m_timerEmployeeInfo->stop();
  m_timerEmployeeInfo->start(5000);
}


