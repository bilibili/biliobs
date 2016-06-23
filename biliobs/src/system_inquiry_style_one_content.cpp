#include "system_inquiry_style_one_content.h"
#include "ui_system_inquiry_style_one_content.h"

SystemInquiryStyleOneContent::SystemInquiryStyleOneContent(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SystemInquiryStyleOneContent)
{
    ui->setupUi(this);
}

SystemInquiryStyleOneContent::~SystemInquiryStyleOneContent()
{
    delete ui;
}

void SystemInquiryStyleOneContent::setDetail(const QString &cnt)
{
    ui->detail->setText(cnt);
}