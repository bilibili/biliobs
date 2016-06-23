#include "oper_tip_link_fail_content_wgt.h"
#include "ui_oper_tip_link_fail_content_wgt.h"
#include "oper_tip_dlg.h"

OperTipLinkFailContentWgt::OperTipLinkFailContentWgt(QWidget *parent, OperTipDlg *dlg) :
    QWidget(parent),
    dlg_(dlg),
    ui(new Ui::OperTipLinkFailContentWgt)
{
    ui->setupUi(this);

    ui->OperTipToOpentBtn->setDefault(true);
}

OperTipLinkFailContentWgt::~OperTipLinkFailContentWgt()
{
    delete ui;
}

void OperTipLinkFailContentWgt::on_OperTipToOpentBtn_clicked()
{
    dlg_->accept();
}

void OperTipLinkFailContentWgt::on_OperTipCancelBtn_clicked()
{
    dlg_->reject();
}
