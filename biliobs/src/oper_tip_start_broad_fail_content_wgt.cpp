#include "oper_tip_start_broad_fail_content_wgt.h"
#include "ui_oper_tip_start_broad_fail_content_wgt.h"
#include "oper_tip_dlg.h"

OperTipStartBroadFailContentWgt::OperTipStartBroadFailContentWgt(QWidget *parent, OperTipDlg *dlg) :
    QWidget(parent),
    dlg_(dlg),
    ui(new Ui::OperTipStartBroadFailContentWgt)
{
    ui->setupUi(this);

    ui->OperTipToOpentBtn->setDefault(true);
}

OperTipStartBroadFailContentWgt::~OperTipStartBroadFailContentWgt()
{
    delete ui;
}

void OperTipStartBroadFailContentWgt::on_OperTipToOpentBtn_clicked()
{
    dlg_->accept();
}

void OperTipStartBroadFailContentWgt::on_OperTipCancelBtn_clicked()
{
    dlg_->reject();
}
