#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>

namespace Ui {
class UpdateDialog;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit UpdateDialog(QWidget *parent = 0);
    ~UpdateDialog();
    void setCount(int delete_count, int update_count, int insert_count);
    void setIndexOfDelete(int);
    void setIndexOfUpdate(int);
    void setIndexOfInsert(int);
private:
    Ui::UpdateDialog *ui;
};

#endif // UPDATEDIALOG_H
