#include "system_inquiry_style_two_content.h"
#include "ui_system_inquiry_style_two_content.h"

SystemInquiryStyleTwoContent::SystemInquiryStyleTwoContent(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SystemInquiryStyleTwoContent)
{
    ui->setupUi(this);
}

SystemInquiryStyleTwoContent::~SystemInquiryStyleTwoContent()
{
    delete ui;
}

void SystemInquiryStyleTwoContent::setInfo1(QString const &cnt)
{
    ui->info1->setText(cnt);
}
void SystemInquiryStyleTwoContent::setInfo2(QString const &cnt)
{
    ui->info2->setText(cnt);
}