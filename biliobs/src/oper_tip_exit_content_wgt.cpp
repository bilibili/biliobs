#include "oper_tip_exit_content_wgt.h"
#include "ui_oper_tip_exit_content_wgt.h"
#include "oper_tip_dlg.h"

OperTipExitContentWgt::OperTipExitContentWgt(QWidget *parent, OperTipDlg *dlg) :
    QWidget(parent),
    dlg_(dlg),
    ui(new Ui::OperTipExitContentWgt)
{
    ui->setupUi(this);

    ui->OperTipOkBtn->setDefault(true);
}

OperTipExitContentWgt::~OperTipExitContentWgt()
{
    delete ui;
}

void OperTipExitContentWgt::on_OperTipOkBtn_clicked()
{
    dlg_->accept();
}
void OperTipExitContentWgt::on_OperTipCancelBtn_clicked()
{
    dlg_->reject();
}
