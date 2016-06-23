#ifndef OPER_TIP_HOTKEY_DUPLICATE_CONTENT_WGT_H
#define OPER_TIP_HOTKEY_DUPLICATE_CONTENT_WGT_H

#include <QWidget>

namespace Ui {
class OperTipHotkeyDuplicateContentWgt;
}
class OperTipDlg;
class OperTipHotkeyDuplicateContentWgt : public QWidget
{
    Q_OBJECT

public:
    explicit OperTipHotkeyDuplicateContentWgt(QWidget *parent, OperTipDlg *dlg);
    ~OperTipHotkeyDuplicateContentWgt();

private slots:
    void on_OperTipOkBtn_clicked();

private:
    Ui::OperTipHotkeyDuplicateContentWgt *ui;

    OperTipDlg *const dlg_;
};

#endif // OPER_TIP_HOTKEY_DUPLICATE_CONTENT_WGT_H
