#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "maindelegator.h"
#include <QtGui/QLabel>
#include <QTextCodec>

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow, public MainDelegator::EventListener
{
    Q_OBJECT
    
public:
    //virtual void onRFSerialNumber(char* serial);
    virtual void onMessage(std::string tag, std::string data);
    virtual void onLogo(std::string data);
    virtual void onEmployeeInfo(std::string CoName, std::string Name, std::string PinNo);
    virtual void onStatus(std::string status);
    virtual void onImage(bool val);

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
signals:
    void employeeInfo();
    void resultImage();
    
private slots:
    void updateTime();
    void updateEmployeeInfo();
    void cleanInfo();
    void displayResultImage();
    
private:
    Ui::MainWindow *ui;
    QLabel* statusLabel; 
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
    static QTextCodec * m_codec;
};

#endif // MAINWINDOW_H
