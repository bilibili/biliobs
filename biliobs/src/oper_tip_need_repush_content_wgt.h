#ifndef OPER_TIP_NEED_REPUSH_CONTENT_WGT_H
#define OPER_TIP_NEED_REPUSH_CONTENT_WGT_H

#include <QWidget>

namespace Ui {
class OperTipNeedRepushContentWgt;
}
class OperTipDlg;
class OperTipNeedRepushContentWgt : public QWidget
{
    Q_OBJECT

public:
    explicit OperTipNeedRepushContentWgt(QWidget *parent, OperTipDlg *dlg);
    ~OperTipNeedRepushContentWgt();

private slots:
    void on_OperTipRepushAndApplyBtn_clicked();

    void on_OperTipCancelBtn_clicked();

private:
    Ui::OperTipNeedRepushContentWgt *ui;

    OperTipDlg *const dlg_;
};

#endif // OPER_TIP_NEED_REPUSH_CONTENT_WGT_H
