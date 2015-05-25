#ifndef WARNINGDIALOG_H
#define WARNINGDIALOG_H

#include <QDialog>

class QTimer;
namespace Ui {
class WarningDialog;
}

class WarningDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit WarningDialog(QWidget *parent = 0);
    ~WarningDialog();
    void setTexts(const std::string& title, std::string& contents); 
protected:    
    void showEvent(QShowEvent * event);

private:
    Ui::WarningDialog *ui;
    QTimer *m_timerClose;
};

#endif // WARNINGDIALOG_H
