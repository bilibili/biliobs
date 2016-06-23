#include "oper_tip_broadcast_is_cut_off.h"
#include "ui_oper_tip_broadcast_is_cut_off.h"
#include "oper_tip_dlg.h"

OperTipBroadcastIsCutOff::OperTipBroadcastIsCutOff(QWidget *parent, OperTipDlg *dlg) :
    QWidget(parent),
    dlg_(dlg),
    ui(new Ui::OperTipBroadcastIsCutOff)
{
    ui->setupUi(this);

    ui->OperTipOkBtn->setDefault(true);
}

OperTipBroadcastIsCutOff::~OperTipBroadcastIsCutOff()
{
    delete ui;
}

void OperTipBroadcastIsCutOff::on_OperTipOkBtn_clicked()
{
    dlg_->accept();
}
