#ifndef DANMAKUHISTORYSETTINGWID_H
#define DANMAKUHISTORYSETTINGWID_H

#include <QWidget>
#include <QVariant>

namespace Ui {
class DanmakuHistorySettingWid;
}

class DanmakuHistorySettingWid : public QWidget
{
    Q_OBJECT

public:
    explicit DanmakuHistorySettingWid(QWidget *parent = 0);
    ~DanmakuHistorySettingWid();

signals:
    /*0 - 3, refer to the btns drder by their positions*/
    void modeChanged(int mode_code, int interval);

public slots:
    void OnSaveSetting(QVariant pConfig);
    void OnLoadSetting(QVariant pConfig);
    void OnCancelSetting(QVariant pConfig);

private slots:
    void onIntervalModeBtn(bool checked);
    void onSliderValChanged(int val);

private:
    void sliderSetVal(int);

private:
    Ui::DanmakuHistorySettingWid *ui;

    int interval_def_;
};

#endif // DANMAKUHISTORYSETTINGWID_H
