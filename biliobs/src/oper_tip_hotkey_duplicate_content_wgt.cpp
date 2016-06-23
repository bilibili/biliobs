#include "oper_tip_hotkey_duplicate_content_wgt.h"
#include "ui_oper_tip_hotkey_duplicate_content_wgt.h"
#include "oper_tip_dlg.h"

OperTipHotkeyDuplicateContentWgt::OperTipHotkeyDuplicateContentWgt(QWidget *parent, OperTipDlg *dlg) :
    QWidget(parent),
    dlg_(dlg),
    ui(new Ui::OperTipHotkeyDuplicateContentWgt)
{
    ui->setupUi(this);

    ui->OperTipOkBtn->setDefault(true);
}

OperTipHotkeyDuplicateContentWgt::~OperTipHotkeyDuplicateContentWgt()
{
    delete ui;
}

void OperTipHotkeyDuplicateContentWgt::on_OperTipOkBtn_clicked()
{
    dlg_->accept();
}
