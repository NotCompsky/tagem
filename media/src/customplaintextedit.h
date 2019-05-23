#ifndef CUSTOMPLAINTEXTEDIT_H
#define CUSTOMPLAINTEXTEDIT_H

#include <QPlainTextEdit>
#include <QApplication>
#include <Qt>
#include <QPlainTextEdit>
#include <QDebug>



/**
 * @brief The CustomPlainTextEdit class
 * This class overrides the default QPlainTextEdit object
 * with customizations for the editor.
 */
class CustomPlainTextEdit : public QPlainTextEdit
{
public:
    CustomPlainTextEdit(QWidget *parent = 0);
protected:
    virtual void wheelEvent( QWheelEvent * inEvent );
};


#endif // CUSTOMPLAINTEXTEDIT_H
