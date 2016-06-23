#include "oper_tip_config_used_and_pushed_content_wgt.h"
#include "ui_oper_tip_config_used_and_pushed_content_wgt.h"
#include "oper_tip_dlg.h"

OperTipConfigUsedAndPushedContentWgt::OperTipConfigUsedAndPushedContentWgt(QWidget *parent, OperTipDlg *dlg) :
    QWidget(parent),
    dlg_(dlg),
    ui(new Ui::OperTipConfigUsedAndPushedContentWgt)
{
    ui->setupUi(this);

    ui->OperTipOkBtn->setDefault(true);
}

OperTipConfigUsedAndPushedContentWgt::~OperTipConfigUsedAndPushedContentWgt()
{
    delete ui;
}

void OperTipConfigUsedAndPushedContentWgt::on_OperTipOkBtn_clicked()
{
    dlg_->accept();
}
