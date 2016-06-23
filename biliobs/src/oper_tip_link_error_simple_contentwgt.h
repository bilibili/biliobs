#ifndef OPER_TIP_LINK_ERROR_SIMPLE_CONTENTWGT_H
#define OPER_TIP_LINK_ERROR_SIMPLE_CONTENTWGT_H

#include <QWidget>

namespace Ui {
class OperTipLinkErrorSimpleContentWgt;
}
class OperTipDlg;
class OperTipLinkErrorSimpleContentWgt : public QWidget
{
    Q_OBJECT

public:
    explicit OperTipLinkErrorSimpleContentWgt(QWidget *parent, OperTipDlg *dlg);
    ~OperTipLinkErrorSimpleContentWgt();

private slots:
    void on_OperTipHaveKnowBtn_2_clicked();

private:
    Ui::OperTipLinkErrorSimpleContentWgt *ui;

    OperTipDlg *const dlg_;
};

#endif // OPER_TIP_LINK_ERROR_SIMPLE_CONTENTWGT_H
