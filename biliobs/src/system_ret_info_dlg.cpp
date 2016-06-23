#include "system_ret_info_dlg.h"
#include "ui_system_ret_info_dlg.h"

#include <QGraphicsDropShadowEffect>

SystemRetInfoDlg::SystemRetInfoDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SystemRetInfoDlg)
{
    ui->setupUi(this);

    setWindowTitle("ͨ\346\217\220\347\244\272");  /*提示*/
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    QGraphicsDropShadowEffect *shadowEfct = new QGraphicsDropShadowEffect(this);
    shadowEfct->setBlurRadius(20.0);
    shadowEfct->setColor(QColor(0, 0, 0, 100));
    shadowEfct->setOffset(0, 0);
    setGraphicsEffect(shadowEfct);

    ui->okButton->setDefault(true);
}

SystemRetInfoDlg::~SystemRetInfoDlg()
{
    delete ui;
}

void SystemRetInfoDlg::setTitle(QString const& title)
{
    ui->Title->setText(title);
}

void SystemRetInfoDlg::setSubTitle(QString const& sub_title)
{
    ui->subTitle->setText(sub_title);
}

void SystemRetInfoDlg::setDetailInfo(QString const& info)
{
    ui->detail->setText(info);
}

void SystemRetInfoDlg::mousePressEvent(QMouseEvent* e)
{
    moveHelper.mousePressEvent(this, frameGeometry(), e);
}

void SystemRetInfoDlg::mouseReleaseEvent(QMouseEvent* e)
{
    moveHelper.mouseReleaseEvent(this, e);
}

void SystemRetInfoDlg::mouseMoveEvent(QMouseEvent* e)
{
    moveHelper.mouseMoveEvent(this, e);
}

void SystemRetInfoDlg::on_okButton_clicked()
{
    accept();
}

void SystemRetInfoDlg::on_OperTipCloseBtn_clicked()
{
    reject();
}