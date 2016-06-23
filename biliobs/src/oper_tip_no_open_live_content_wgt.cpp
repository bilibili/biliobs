#include "oper_tip_no_open_live_content_wgt.h"
#include "ui_oper_tip_no_open_live_content_wgt.h"
#include "oper_tip_dlg.h"

OperTipNoOpenLiveContentWgt::OperTipNoOpenLiveContentWgt(QWidget *parent, OperTipDlg *dlg) :
    QWidget(parent),
    dlg_(dlg),
    ui(new Ui::OperTipNoOpenLiveContentWgt)
{
    ui->setupUi(this);

    ui->OperTipToOpentBtn->setDefault(true);
}

OperTipNoOpenLiveContentWgt::~OperTipNoOpenLiveContentWgt()
{
    delete ui;
}

QCheckBox* OperTipNoOpenLiveContentWgt::notTipCheckBox()
{
    return ui->todayWithoutTip;
}

void OperTipNoOpenLiveContentWgt::on_OperTipToOpentBtn_clicked()
{
    dlg_->accept();
}

void OperTipNoOpenLiveContentWgt::on_OperTipCancelBtn_clicked()
{
    dlg_->reject();
}
