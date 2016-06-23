#include "oper_tip_duplicate_name_content_wgt.h"
#include "ui_oper_tip_duplicate_name_content_wgt.h"
#include "oper_tip_dlg.h"

OperTipDuplicateNameContentWgt::OperTipDuplicateNameContentWgt(QWidget *parent, OperTipDlg *dlg) :
    QWidget(parent),
    dlg_(dlg),
    ui(new Ui::OperTipDuplicateNameContentWgt)
{
    ui->setupUi(this);

    ui->OperTipOkBtn->setDefault(true);
}

OperTipDuplicateNameContentWgt::~OperTipDuplicateNameContentWgt()
{
    delete ui;
}

void OperTipDuplicateNameContentWgt::on_OperTipOkBtn_clicked()
{
    dlg_->accept();
}
