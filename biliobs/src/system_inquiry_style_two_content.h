#ifndef SYSTEM_INQUIRY_STYLE_TWO_CONTENT_H
#define SYSTEM_INQUIRY_STYLE_TWO_CONTENT_H

#include <QWidget>

namespace Ui {
class SystemInquiryStyleTwoContent;
}

class SystemInquiryStyleTwoContent : public QWidget
{
    Q_OBJECT

public:
    explicit SystemInquiryStyleTwoContent(QWidget *parent = 0);
    ~SystemInquiryStyleTwoContent();

    void setInfo1(QString const &cnt);
    void setInfo2(QString const &cnt);
private:
    Ui::SystemInquiryStyleTwoContent *ui;
};

#endif // SYSTEM_INQUIRY_STYLE_TWO_CONTENT_H
