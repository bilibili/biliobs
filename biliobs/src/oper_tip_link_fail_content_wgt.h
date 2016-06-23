#ifndef OPER_TIP_LINK_FAIL_CONTENT_WGT_H
#define OPER_TIP_LINK_FAIL_CONTENT_WGT_H

#include <QWidget>

namespace Ui {
class OperTipLinkFailContentWgt;
}
class OperTipDlg;
class OperTipLinkFailContentWgt : public QWidget
{
    Q_OBJECT

public:
    explicit OperTipLinkFailContentWgt(QWidget *parent, OperTipDlg *dlg);
    ~OperTipLinkFailContentWgt();

private slots:
    void on_OperTipToOpentBtn_clicked();

    void on_OperTipCancelBtn_clicked();

private:
    Ui::OperTipLinkFailContentWgt *ui;

    OperTipDlg *const dlg_;
};

#endif // OPER_TIP_LINK_FAIL_CONTENT_WGT_H
