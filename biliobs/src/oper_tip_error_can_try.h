#ifndef OPER_TIP_ERROR_CAN_TRY_H
#define OPER_TIP_ERROR_CAN_TRY_H

#include <QWidget>
#include <qdialog.h>
#include "bili_move_frameless_window.hpp"
namespace Ui {
class OperTipErrorCanTry;
}

class OperTipErrorCanTry : public QDialog
{
    Q_OBJECT

public:
    explicit OperTipErrorCanTry(QWidget *parent = 0);
    ~OperTipErrorCanTry();

private slots:
    void rePush();
    void onCancelBtnClicked();

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

    MovableFramelessWindowUtil<QDialog> moveHelper;

private:
    Ui::OperTipErrorCanTry *ui;
};

#endif // OPER_TIP_ERROR_CAN_TRY_H
