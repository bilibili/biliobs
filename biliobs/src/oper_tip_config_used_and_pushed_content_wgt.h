#ifndef OPER_TIP_CONFIG_USED_AND_PUSHED_CONTENT_WGT_H
#define OPER_TIP_CONFIG_USED_AND_PUSHED_CONTENT_WGT_H

#include <QWidget>

namespace Ui {
class OperTipConfigUsedAndPushedContentWgt;
}

class OperTipDlg;

class OperTipConfigUsedAndPushedContentWgt : public QWidget
{
    Q_OBJECT

public:
    explicit OperTipConfigUsedAndPushedContentWgt(QWidget *parent, OperTipDlg *dlg);
    ~OperTipConfigUsedAndPushedContentWgt();

private slots:
    void on_OperTipOkBtn_clicked();

private:
    Ui::OperTipConfigUsedAndPushedContentWgt *ui;

    OperTipDlg *const dlg_;
};

#endif // OPER_TIP_CONFIG_USED_AND_PUSHED_CONTENT_WGT_H
