#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "maindelegator.h"
#include <QtGui/QLabel>
#include <QLineEdit>
#include <QTextCodec>
#include "syncdialog.h"
#include "updatedialog.h"
#include "warningdialog.h"
#include <QMovie>
#include <QTransform>

namespace Ui {
class MainWindow;
}

class QTransform;

class MainWindow : public QMainWindow, public MainDelegator::EventListener
{
    Q_OBJECT
    
public:
    //virtual void onRFSerialNumber(char* serial);
    virtual void onSyncStart();
    virtual void onSyncCount(int count);
    virtual void onSyncIndex(int index);
    virtual void onSyncEnd(bool val);
    virtual void onUpdateStart();
    virtual void onUpdateCount(int delete_count, int update_count, int insert_count);
    virtual void onUpdateLocalDBEnd(const char* updatetime);
    virtual void onUpdateFBBegin(int delete_count, int save_count);
    virtual void onUpdateFBDeleteIndex(int index);
    virtual void onUpdateFBSaveIndex(int index);
    virtual void onUpdateFBEnd();
    virtual void onMessage(std::string tag, std::string data);
    virtual void onLogo(std::string data);
    virtual void onEmployeeInfo(std::string CoName, std::string Name, std::string PinNo);
    virtual void onStatus(std::string status);
    virtual void onResultImage(bool val);
    virtual const char* onGetPinNo();
    virtual void onWarning(std::string msg1, std::string msg2);
    virtual void onFingerImage(const unsigned char* img, int len);

    void runMainDelegator(const char* config);
    
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
signals:
    void sigStartSync();
    void sigEndSync();
    void sigStartUpdate();
    void sigEndUpdate();
    void sigEmployeeInfo();
    void sigResultImage();
    void sigFingerImage(const unsigned char*, int);
    void sigWarning();
    
private slots:
    void startSync();
    void endSync();
    void startUpdate();
    void endUpdate();
    void updateTime();
    void updateEmployeeInfo();
    void cleanInfo();
    void displayResultImage();
    void restoreLogo();
    void warning();
    void fingerImage(const unsigned char*, int);
    void cleanFingerImage();
    void doSecurityNumber(QString);
    
private:
    Ui::MainWindow *ui;
    QLabel* m_statusLabel; 
    std::map<std::string, QLabel*> labelTable;
    QString m_CoName;
    QString m_Name;
    QString m_PinNo;
    unsigned char* m_img_buf;
    int m_img_sz;
    QPixmap m_pm_logo;
    QPixmap m_pm_auth_pass;
    QPixmap m_pm_auth_fail;
    QPixmap* m_pm_auth;
    QTimer *m_timerEmployeeInfo;
    QTimer *m_timerVIMG;
    static QTextCodec * m_codec;
    SyncDialog m_syncDialog;
    UpdateDialog m_updateDialog;
    WarningDialog m_warningDialog;
    QMovie *m_aninfinger;
    //const char* m_updatetime;
    QLineEdit *m_qlePinNo;
    QTransform m_trans;
    MainDelegator* m_md;
};

#endif // MAINWINDOW_H
