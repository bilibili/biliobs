#ifndef OPER_TIP_BROADCAST_IS_CUT_OFF_H
#define OPER_TIP_BROADCAST_IS_CUT_OFF_H

#include <QWidget>

namespace Ui {
class OperTipBroadcastIsCutOff;
}
class OperTipDlg;

class OperTipBroadcastIsCutOff : public QWidget
{
    Q_OBJECT

public:
    explicit OperTipBroadcastIsCutOff(QWidget *parent, OperTipDlg *dlg);
    ~OperTipBroadcastIsCutOff();

private slots:
    void on_OperTipOkBtn_clicked();

private:
    Ui::OperTipBroadcastIsCutOff *ui;

    OperTipDlg *const dlg_;
};

#endif // OPER_TIP_BROADCAST_IS_CUT_OFF_H
