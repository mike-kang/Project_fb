#include "qpinnoedit.h"
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>

QPinNoEdit::QPinNoEdit(QWidget *parent) :
    QLineEdit(parent)
{
}

bool QPinNoEdit::event(QEvent* event)
{
  if(event->type() == QEvent::KeyPress){
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    //qDebug() << keyEvent->key();
    if(keyEvent->key() == Qt::Key_Asterisk) {
      backspace();
    }
    else if(keyEvent->key() == Qt::Key_NumberSign) {
      emit sigPinNo(text());
      clear();
    }
    return QLineEdit::event(event);    
  }

  return QLineEdit::event(event);
}
