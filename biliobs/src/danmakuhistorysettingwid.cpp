#include "danmakuhistorysettingwid.h"
#include "ui_danmakuhistorysettingwid.h"


#include "BiliUIConfigSync.hpp"

DanmakuHistorySettingWid::DanmakuHistorySettingWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DanmakuHistorySettingWid)
{
    ui->setupUi(this);

    connect(ui->intervalModeBtn, &QRadioButton::toggled, this, &DanmakuHistorySettingWid::onIntervalModeBtn);
    connect(ui->intervalSlider, &CircleSliderSlider::valueChanged, this, &DanmakuHistorySettingWid::onSliderValChanged);

    ui->intervalSlider->setRange(1, 50, 10);
    ui->intervalVal->setText("1 s");
}

DanmakuHistorySettingWid::~DanmakuHistorySettingWid()
{
    delete ui;
}

void DanmakuHistorySettingWid::OnSaveSetting(QVariant pConfig)
{
    config_t* config = qVPtr<config_t>::toPtr(pConfig);
    int mode_code;
    int p_mode;
    int p_interval;
    bool changed = false;

    if (config_has_user_value(config, "DanmakuHistory", "DanmakuRefreshMode"))
        p_mode = config_get_int(config, "DanmakuHistory", "DanmakuRefreshMode");
    else
        p_mode = config_get_default_int(config, "DanmakuHistory", "DanmakuRefreshMode");

    if (ui->manualModeBtn->isChecked())
        mode_code = 0;
    else if (ui->focusModeBtn->isChecked())
        mode_code = 1;
    else if (ui->rtModeBtn->isChecked())
        mode_code = 2;
    else
        mode_code = 3;

    if (p_mode != mode_code) {
        config_set_int(config, "DanmakuHistory", "DanmakuRefreshMode", mode_code);

        if (3 == mode_code) {
            
            config_set_int(config, "DanmakuHistory", "DanmakuRefreshInterval", ui->intervalSlider->value());

            emit modeChanged(mode_code, ui->intervalSlider->value());

        } else {
            emit modeChanged(mode_code, 0);
        }
    } else {
        if (3 == mode_code) {
            if (ui->intervalSlider->value() != interval_def_) {
                config_set_int(config, "DanmakuHistory", "DanmakuRefreshInterval", ui->intervalSlider->value());

                emit modeChanged(mode_code, ui->intervalSlider->value());

            }
        }
    }

    
}
void DanmakuHistorySettingWid::OnLoadSetting(QVariant pConfig)
{
    config_t* config = qVPtr<config_t>::toPtr(pConfig);

    if (config_has_user_value(config, "DanmakuHistory", "DanmakuRefreshInterval"))
        interval_def_ = config_get_int(config, "DanmakuHistory", "DanmakuRefreshInterval");
    else
        interval_def_ = config_get_default_int(config, "DanmakuHistory", "DanmakuRefreshInterval");

    int mode_code;
    if (config_has_user_value(config, "DanmakuHistory", "DanmakuRefreshMode"))
        mode_code = config_get_int(config, "DanmakuHistory", "DanmakuRefreshMode");
    else
        mode_code = config_get_default_int(config, "DanmakuHistory", "DanmakuRefreshMode");
     
    switch (mode_code) {
    case 1:
        ui->intervalSlider->setEnabled(false);
        ui->intervalVal->clear();
        ui->focusModeBtn->setChecked(true);
        break;
    case 2:
        ui->intervalSlider->setEnabled(false);
        ui->intervalVal->clear();
        ui->rtModeBtn->setChecked(true);
        break;
    case 3:
        ui->intervalSlider->setEnabled(true);
        ui->intervalModeBtn->setChecked(true);
        sliderSetVal(interval_def_);

        break;
    default:
        ui->intervalSlider->setEnabled(false);
        ui->intervalVal->clear();
        ui->manualModeBtn->setChecked(true);
        break;
    }

}
void DanmakuHistorySettingWid::OnCancelSetting(QVariant pConfig)
{

}

void DanmakuHistorySettingWid::onIntervalModeBtn(bool checked)
{
    ui->intervalSlider->setEnabled(checked);

    if (checked)
        sliderSetVal(interval_def_);
    else {
        ui->intervalVal->clear();
        ui->intervalSlider->setValue(interval_def_);
    }
}

void DanmakuHistorySettingWid::sliderSetVal(int val)
{
    ui->intervalSlider->setValue(val);

    
    ui->intervalVal->setText(
                        QString::number((double)val / 10.0)
                        + " s"
                    );
}

void DanmakuHistorySettingWid::onSliderValChanged(int val)
{
    ui->intervalVal->setText(
        QString::number((double)val / 10.0)
        + " s"
        );
}

