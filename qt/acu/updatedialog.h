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
    void setLocalDBCount(int delete_count, int update_count, int insert_count);
    void setFBCount(int delete_count, int save_count);
    void setFBIndexOfDelete(int);
    void setFBIndexOfSave(int);
private:
    Ui::UpdateDialog *ui;
};

#endif // UPDATEDIALOG_H
