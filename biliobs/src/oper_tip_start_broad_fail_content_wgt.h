#ifndef OPER_TIP_START_BROAD_FAIL_CONTENT_WGT_H
#define OPER_TIP_START_BROAD_FAIL_CONTENT_WGT_H

#include <QWidget>

namespace Ui {
class OperTipStartBroadFailContentWgt;
}
class OperTipDlg;
class OperTipStartBroadFailContentWgt : public QWidget
{
    Q_OBJECT

public:
    explicit OperTipStartBroadFailContentWgt(QWidget *parent, OperTipDlg *dlg);
    ~OperTipStartBroadFailContentWgt();

private slots:
    void on_OperTipToOpentBtn_clicked();

    void on_OperTipCancelBtn_clicked();

private:
    Ui::OperTipStartBroadFailContentWgt *ui;

    OperTipDlg *const dlg_;
};

#endif // OPER_TIP_START_BROAD_FAIL_CONTENT_WGT_H
