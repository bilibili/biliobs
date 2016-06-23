#include "system_inquiry_dlg.h"
#include "ui_system_inquiry_dlg.h"
#include "system_inquiry_style_one_content.h"
#include "system_inquiry_style_two_content.h"

#include <QGraphicsDropShadowEffect>

SystemInquiryDlg::SystemInquiryDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SystemInquiryDlg)
{
    ui->setupUi(this);
    setWindowTitle("\350\257\242\351\227\256");  /*询问*/
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    QGraphicsDropShadowEffect *shadowEfct = new QGraphicsDropShadowEffect(this);
    shadowEfct->setBlurRadius(20.0);
    shadowEfct->setColor(QColor(0, 0, 0, 100));
    shadowEfct->setOffset(0, 0);
    setGraphicsEffect(shadowEfct);

    ui->okButton->setDefault(true);

}

SystemInquiryDlg::~SystemInquiryDlg()
{
    delete ui;
}

void SystemInquiryDlg::init(DlgStyle style)
{
    content_sytle_ = style;
    QHBoxLayout *content_layout;
   
    switch (content_sytle_)
    {
    case STYLE_1:
        content_layout = new QHBoxLayout(ui->contentWgt);
        content_wgt_ = new SystemInquiryStyleOneContent();
        content_layout->addWidget(content_wgt_);
        break;
    case STYLE_2:
        content_layout = new QHBoxLayout(ui->contentWgt);
        content_wgt_ = new SystemInquiryStyleTwoContent(ui->contentWgt);
        content_layout->addWidget(content_wgt_);
        break;
    default:
        break;
    }
}

void SystemInquiryDlg::setTitle(QString const& title)
{
    ui->Title->setText(title);
}

void SystemInquiryDlg::setInfo1(QString const &cnt)
{
    switch (content_sytle_)
    {
    case STYLE_1:
        ((SystemInquiryStyleOneContent*)content_wgt_)->setDetail(cnt);
        break;
    case STYLE_2:
        ((SystemInquiryStyleTwoContent*)content_wgt_)->setInfo1(cnt);
        break;
    default:
        break;
    }
}

void SystemInquiryDlg::setInfo2(QString const &cnt)
{
    switch (content_sytle_)
    {
    case STYLE_2:
        ((SystemInquiryStyleTwoContent*)content_wgt_)->setInfo2(cnt);
        break;
    default:
        break;
    }
}

void SystemInquiryDlg::setButtonsText(QString const &ok, QString const &cancel)
{
    ui->okButton->setText(ok);
    ui->cancelButton->setText(cancel);
}

void SystemInquiryDlg::mousePressEvent(QMouseEvent* e)
{
    moveHelper.mousePressEvent(this, frameGeometry(), e);
}

void SystemInquiryDlg::mouseReleaseEvent(QMouseEvent* e)
{
    moveHelper.mouseReleaseEvent(this, e);
}

void SystemInquiryDlg::mouseMoveEvent(QMouseEvent* e)
{
    moveHelper.mouseMoveEvent(this, e);
}

//void SystemInquiryDlg::keyReleaseEvent(QKeyEvent *e)
//{
//    
//    if (e->count() == 1 && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)) {
//        e->accept();
//        accept();
//    }
//}

void SystemInquiryDlg::on_okButton_clicked()
{
    accept();
}
void SystemInquiryDlg::on_cancelButton_clicked()
{
    reject();
}
void SystemInquiryDlg::on_OperTipCloseBtn_clicked()
{
    reject();
}

