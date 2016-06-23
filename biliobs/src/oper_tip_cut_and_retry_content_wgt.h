#ifndef OPER_TIP_CUT_AND_RETRY_CONTENT_WGT_H
#define OPER_TIP_CUT_AND_RETRY_CONTENT_WGT_H

#include <QWidget>

namespace Ui {
class OperTipCutAndRetryContentWgt;
}
class OperTipDlg;

class OperTipCutAndRetryContentWgt : public QWidget
{
    Q_OBJECT

public:
    explicit OperTipCutAndRetryContentWgt(QWidget *parent, OperTipDlg *dlg);
    ~OperTipCutAndRetryContentWgt();

private slots:
    void on_OperTipCancelBtn_clicked();

private:
    Ui::OperTipCutAndRetryContentWgt *ui;

    OperTipDlg *dlg_;
};

#endif // OPER_TIP_CUT_AND_RETRY_CONTENT_WGT_H
