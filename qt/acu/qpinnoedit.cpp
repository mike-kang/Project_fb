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
    if(keyEvent->key() == Qt::Key_Asterisk) {
      QString s = text();
      if(s.length() > 0){
        setText(s.left(s.length()-1));
      }
        
    }
    return QLineEdit::event(event);    
  }

  return QLineEdit::event(event);
}
