#ifndef OPER_TIP_DLG_H
#define OPER_TIP_DLG_H

#include <QDialog>

#include "bili_move_frameless_window.hpp"

namespace Ui {
class OperTipDlg;
}

class QCheckBox;

class OperTipDlg : public QDialog
{
    Q_OBJECT

public:
    explicit OperTipDlg(QWidget *parent = 0);
    ~OperTipDlg();

    QWidget *getContentContainer() const;

    void setNotTipCheckBox(QCheckBox *com) { not_tip_checkBox_ = com; }

    bool notTipChecked() const;
private:
    QCheckBox *not_tip_checkBox_;

private slots:
    void on_OperTipCloseBtn_clicked();

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

    MovableFramelessWindowUtil<QDialog> moveHelper;

//private slots:
//    void onEnterBtnClk();

private:
    Ui::OperTipDlg *ui;
};

#endif // OPER_TIP_DLG_H
