#include "oper_tip_link_error_simple_contentwgt.h"
#include "ui_oper_tip_link_error_simple_contentwgt.h"
#include "oper_tip_dlg.h"

OperTipLinkErrorSimpleContentWgt::OperTipLinkErrorSimpleContentWgt(QWidget *parent, OperTipDlg *dlg) :
    QWidget(parent),
    dlg_(dlg),
    ui(new Ui::OperTipLinkErrorSimpleContentWgt)
{
    ui->setupUi(this);

    ui->OperTipHaveKnowBtn->setDefault(true);
}

OperTipLinkErrorSimpleContentWgt::~OperTipLinkErrorSimpleContentWgt()
{
    delete ui;
}

void OperTipLinkErrorSimpleContentWgt::on_OperTipHaveKnowBtn_2_clicked()
{
    dlg_->accept();
}
