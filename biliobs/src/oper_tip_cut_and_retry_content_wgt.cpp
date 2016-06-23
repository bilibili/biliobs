#include "oper_tip_cut_and_retry_content_wgt.h"
#include "ui_oper_tip_cut_and_retry_content_wgt.h"
#include "oper_tip_dlg.h"

OperTipCutAndRetryContentWgt::OperTipCutAndRetryContentWgt(QWidget *parent, OperTipDlg *dlg) :
    QWidget(parent),
    dlg_(dlg),
    ui(new Ui::OperTipCutAndRetryContentWgt)
{
    ui->setupUi(this);

    ui->OperTipCancelBtn->setDefault(true);
}

OperTipCutAndRetryContentWgt::~OperTipCutAndRetryContentWgt()
{
    delete ui;
}

void OperTipCutAndRetryContentWgt::on_OperTipCancelBtn_clicked()
{
    dlg_->reject();
}
