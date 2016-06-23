#include "oper_tip_need_repush_content_wgt.h"
#include "ui_oper_tip_need_repush_content_wgt.h"
#include "oper_tip_dlg.h"

OperTipNeedRepushContentWgt::OperTipNeedRepushContentWgt(QWidget *parent, OperTipDlg *dlg) :
    QWidget(parent),
    dlg_(dlg),
    ui(new Ui::OperTipNeedRepushContentWgt)
{
    ui->setupUi(this);

    ui->OperTipRepushAndApplyBtn->setDefault(true);
}

OperTipNeedRepushContentWgt::~OperTipNeedRepushContentWgt()
{
    delete ui;
}

void OperTipNeedRepushContentWgt::on_OperTipRepushAndApplyBtn_clicked()
{
    dlg_->accept();
}

void OperTipNeedRepushContentWgt::on_OperTipCancelBtn_clicked()
{
    dlg_->reject();
}
