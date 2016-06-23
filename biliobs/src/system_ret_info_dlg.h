#ifndef SYSTEM_RET_INFO_DLG_H
#define SYSTEM_RET_INFO_DLG_H

#include <QDialog>
#include "bili_move_frameless_window.hpp"

namespace Ui {
class SystemRetInfoDlg;
}

class SystemRetInfoDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SystemRetInfoDlg(QWidget *parent = 0);
    ~SystemRetInfoDlg();

    void setTitle(QString const& title);
    void setSubTitle(QString const& sub_title);
    void setDetailInfo(QString const& info);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

    MovableFramelessWindowUtil<QDialog> moveHelper;

private slots:
    void on_okButton_clicked();
    void on_OperTipCloseBtn_clicked();

private:
    Ui::SystemRetInfoDlg *ui;
};

#endif // SYSTEM_RET_INFO_DLG_H
