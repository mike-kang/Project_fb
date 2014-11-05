#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "maindelegator.h"
#include <QtGui/QLabel>
namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow, public MainDelegator::EventListener
{
    Q_OBJECT
    
public:
    //virtual void onRFSerialNumber(char* serial);
    virtual void onMessage(std::string tag, std::string data);
    virtual void onEmployeeInfo(std::string CoName, std::string Name, std::string PinNo, const unsigned char* img_buf, int img_sz);
    virtual void onStatus(std::string status);
    
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
signals:
    void employeeInfo();

private slots:
    void updateTime();
    void updateEmployeeInfo();


private:
    Ui::MainWindow *ui;
    QLabel* statusLabel; 
    std::map<std::string, QLabel*> labelTable;
    QString m_CoName;
    QString m_Name;
    QString m_PinNo;
    unsigned char* m_img_buf;
    int m_img_sz;
    
};

#endif // MAINWINDOW_H
