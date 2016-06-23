#ifndef SYSTEM_INQUIRY_DLG_H
#define SYSTEM_INQUIRY_DLG_H

#include <QDialog>
#include "bili_move_frameless_window.hpp"

namespace Ui {
class SystemInquiryDlg;
}

class SystemInquiryDlg : public QDialog
{
    Q_OBJECT
public:
    enum DlgStyle {
        STYLE_1,
        STYLE_2
    };

public:
    explicit SystemInquiryDlg(QWidget *parent = 0);
    ~SystemInquiryDlg();

    void init(DlgStyle style);

    void setTitle(QString const& title);

    void setInfo1(QString const &cnt);
    void setInfo2(QString const &cnt);

    void setButtonsText(QString const &ok, QString const &cancel);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

    MovableFramelessWindowUtil<QDialog> moveHelper;

//private:
//    void keyReleaseEvent(QKeyEvent *)override;

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void on_OperTipCloseBtn_clicked();


private:
    Ui::SystemInquiryDlg *ui;

    DlgStyle content_sytle_;
    QWidget *content_wgt_;
};

#endif // SYSTEM_INQUIRY_DLG_H
