#ifndef OPER_TIP_EXIT_CONTENT_WGT_H
#define OPER_TIP_EXIT_CONTENT_WGT_H

#include <QWidget>

namespace Ui {
class OperTipExitContentWgt;
}
class OperTipDlg;
class OperTipExitContentWgt : public QWidget
{
    Q_OBJECT

public:
    explicit OperTipExitContentWgt(QWidget *parent, OperTipDlg *dlg);
    ~OperTipExitContentWgt();

private slots:
    void on_OperTipOkBtn_clicked();
    void on_OperTipCancelBtn_clicked();

private:
    Ui::OperTipExitContentWgt *ui;

    OperTipDlg *const dlg_;
};

#endif // OPER_TIP_EXIT_CONTENT_WGT_H
