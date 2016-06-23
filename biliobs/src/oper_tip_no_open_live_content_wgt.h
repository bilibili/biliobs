#ifndef OPER_TIP_NO_OPEN_LIVE_CONTENT_WGT_H
#define OPER_TIP_NO_OPEN_LIVE_CONTENT_WGT_H

#include <QWidget>

namespace Ui {
class OperTipNoOpenLiveContentWgt;
}
class OperTipDlg;
class QCheckBox;
class OperTipNoOpenLiveContentWgt : public QWidget
{
    Q_OBJECT

public:
    explicit OperTipNoOpenLiveContentWgt(QWidget *parent, OperTipDlg *dlg);
    ~OperTipNoOpenLiveContentWgt();

    QCheckBox* notTipCheckBox();

private slots:
    void on_OperTipToOpentBtn_clicked();

    void on_OperTipCancelBtn_clicked();

private:
    Ui::OperTipNoOpenLiveContentWgt *ui;

    OperTipDlg *const dlg_;
};

#endif // OPER_TIP_NO_OPEN_LIVE_CONTENT_WGT_H
