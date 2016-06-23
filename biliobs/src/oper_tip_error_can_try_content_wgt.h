#ifndef OPER_TIP_ERROR_CAN_TRY_CONTENT_WGT_H
#define OPER_TIP_ERROR_CAN_TRY_CONTENT_WGT_H

#include <QWidget>

namespace Ui {
class OperTipErrorCanTryContentWgt;
}
class OperTipDlg;
class OperTipErrorCanTryContentWgt : public QWidget
{
    Q_OBJECT

public:
    explicit OperTipErrorCanTryContentWgt(QWidget *parent, OperTipDlg *dlg);
    ~OperTipErrorCanTryContentWgt();

private slots:
    void on_OperTipRePushBtn_clicked();

    void on_OperTipCancelBtn_clicked();

private:
    Ui::OperTipErrorCanTryContentWgt *ui;

    OperTipDlg *const dlg_;

};

#endif // OPER_TIP_ERROR_CAN_TRY_CONTENT_WGT_H
