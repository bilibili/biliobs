#ifndef OPER_TIP_DUPLICATE_NAME_CONTENT_WGT_H
#define OPER_TIP_DUPLICATE_NAME_CONTENT_WGT_H

#include <QWidget>

class OperTipDlg;

namespace Ui {
class OperTipDuplicateNameContentWgt;
}

class OperTipDuplicateNameContentWgt : public QWidget
{
    Q_OBJECT

public:
    explicit OperTipDuplicateNameContentWgt(QWidget *parent, OperTipDlg *dlg);
    ~OperTipDuplicateNameContentWgt();

private slots:
    void on_OperTipOkBtn_clicked();

private:
    Ui::OperTipDuplicateNameContentWgt *ui;

    OperTipDlg *const dlg_;
};

#endif // OPER_TIP_DUPLICATE_NAME_CONTENT_WGT_H
