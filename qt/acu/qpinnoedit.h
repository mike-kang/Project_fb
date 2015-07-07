#ifndef QPINNOEDIT_H
#define QPINNOEDIT_H

#include <QLineEdit>

class QPinNoEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit QPinNoEdit(QWidget *parent = 0);
    bool event(QEvent* event);
    
signals:
    void sigPinNo(QString pinno);

public slots:
    
};

#endif // QPINNOEDIT_H
