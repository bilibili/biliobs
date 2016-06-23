#ifndef SYSTEM_INQUIRY_STYLE_ONE_CONTENT_H
#define SYSTEM_INQUIRY_STYLE_ONE_CONTENT_H

#include <QWidget>

namespace Ui {
class SystemInquiryStyleOneContent;
}

class SystemInquiryStyleOneContent : public QWidget
{
    Q_OBJECT

public:
    explicit SystemInquiryStyleOneContent(QWidget *parent = 0);
    ~SystemInquiryStyleOneContent();

    void setDetail(const QString &);
private:
    Ui::SystemInquiryStyleOneContent *ui;
};

#endif // SYSTEM_INQUIRY_STYLE_ONE_CONTENT_H
