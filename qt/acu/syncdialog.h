#ifndef SYNCDIALOG_H
#define SYNCDIALOG_H

#include <QDialog>

namespace Ui {
class SyncDialog;
}

class SyncDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SyncDialog(QWidget *parent = 0);
    ~SyncDialog();
    void setCount(int);
    void setIndex(int);

private:
    Ui::SyncDialog *ui;
};

#endif // SYNCDIALOG_H
