#include "oper_tip_dlg.h"
#include "ui_oper_tip_dlg.h"
#include "oper_tip_dlg.h"

#include <qcheckbox.h>

#include <QGraphicsDropShadowEffect>

OperTipDlg::OperTipDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OperTipDlg),
    not_tip_checkBox_(0)
{
    ui->setupUi(this);
    //setWindowTitle(QStringLiteral("操作提示"));
    setWindowTitle("\346\223\215\344\275\234\346\217\220\347\244\272"); /*操作提示*/
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    QGraphicsDropShadowEffect *shadowEfct = new QGraphicsDropShadowEffect(this);
    shadowEfct->setBlurRadius(20.0);
    shadowEfct->setColor(QColor(0, 0, 0, 100));
    shadowEfct->setOffset(0, 0);
    setGraphicsEffect(shadowEfct);
}

OperTipDlg::~OperTipDlg()
{
    delete ui;
}

QWidget *OperTipDlg::getContentContainer() const
{
    return ui->contentWgt;
}

bool OperTipDlg::notTipChecked() const
{
    if (not_tip_checkBox_)
        return not_tip_checkBox_->isChecked();
    
    return false;
}

void OperTipDlg::on_OperTipCloseBtn_clicked()
{
    reject();
}

void OperTipDlg::mousePressEvent(QMouseEvent* e)
{
    moveHelper.mousePressEvent(this, frameGeometry(), e);
}

void OperTipDlg::mouseReleaseEvent(QMouseEvent* e)
{
    moveHelper.mouseReleaseEvent(this, e);
}

void OperTipDlg::mouseMoveEvent(QMouseEvent* e)
{
    moveHelper.mouseMoveEvent(this, e);
}