#include "oper_tip_error_can_try_content_wgt.h"
#include "ui_oper_tip_error_can_try_content_wgt.h"
#include "oper_tip_dlg.h"

OperTipErrorCanTryContentWgt::OperTipErrorCanTryContentWgt(QWidget *parent, OperTipDlg *dlg) :
    QWidget(parent),
    dlg_(dlg),
    ui(new Ui::OperTipErrorCanTryContentWgt)
{
    ui->setupUi(this);

    ui->OperTipRePushBtn->setDefault(true);
}

OperTipErrorCanTryContentWgt::~OperTipErrorCanTryContentWgt()
{
    delete ui;
}

void OperTipErrorCanTryContentWgt::on_OperTipRePushBtn_clicked()
{
    dlg_->accept();
}

void OperTipErrorCanTryContentWgt::on_OperTipCancelBtn_clicked()
{
    dlg_->reject();
}
