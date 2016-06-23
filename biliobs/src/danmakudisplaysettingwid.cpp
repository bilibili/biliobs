#include "danmakudisplaysettingwid.h"
#include "slider_interface.h"

DanmakuDisplaySettingWid::DanmakuDisplaySettingWid(ConfigFile &configData, QWidget *parent)
	: QWidget(parent)
	,configData_(configData)
	,backupDisplaySetting_(0){

	ui.setupUi(this);

	auto hideOldCtrls = [this]() {

		ui.liveStateWid->setHidden(true);
		ui.displayWid->setHidden(true);
	};
	hideOldCtrls();

	connect(ui.CurLiveStateBtnGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(sglCurLiveStateOnOff(int)));
	connect(ui.PropsAndLaoYeBtnGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(sglPropsAndLaoYeOnOff(int)));
	connect(ui.AnnounceTipBtnGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(sglSysAnnounceOnOff(int)));

	connect(ui.RestoreDefaultBtn, SIGNAL(clicked()), this, SLOT(sltRestoreDefault()));
	connect(ui.TestDMBtn, SIGNAL(clicked()), this, SIGNAL(sglTestDM()));

    connect(ui.DanmuOpacitySlider, &SliderInterface::valueChanged, this, &DanmakuDisplaySettingWid::onDanmuOpacityChanged);

    ui.DanmuOpacitySlider->setRange(5, 100, 100);
}

DanmakuDisplaySettingWid::~DanmakuDisplaySettingWid() {

}

void DanmakuDisplaySettingWid::onDanmuOpacityChanged(int o)
{
    emit DanmuOpacityChanged(o);
    ui.DanmuOpacityValLbl->setText(QString::number(o) + "%");
}

void DanmakuDisplaySettingWid::sltRestoreDefault(){

	ui.DanmakuAlwaysOnCheckBox->setChecked(false);
	ui.LiveStateRadioBtnOn->setChecked(true);
    emit sglCurLiveStateOnOff(-2);

	ui.PropsAndLaoYeRadioBtnOn->setChecked(true);
	ui.AnnounceRadioBtnOn->setChecked(true);

	emit sglSetDMDisplaySettings(7);
}


void DanmakuDisplaySettingWid::OnSaveSetting(QVariant pConfig) {

	config_t* config = qVPtr<config_t>::toPtr(pConfig);

	WidgetToData(BILI_CONFIG_BOOL(), ui.DanmakuAlwaysOnCheckBox, config, "Danmaku\0AutoStart");
	WidgetToData(BILI_CONFIG_STRING(), ui.CurLiveStateBtnGroup, config, "Danmaku\0ShowLiveStatus");
	WidgetToData(BILI_CONFIG_STRING(), ui.PropsAndLaoYeBtnGroup, config, "Danmaku\0ItemAndLaoyeMessage");
	WidgetToData(BILI_CONFIG_STRING(), ui.AnnounceTipBtnGroup, config, "Danmaku\0SystemAnnounce");

    WidgetToData(BILI_CONFIG_INT(), ui.DanmuOpacitySlider, config, "Danmaku\0DanmuOpacity");
    ui.DanmuOpacityValLbl->setText(QString::number(ui.DanmuOpacitySlider->value()) + "%");
}

void DanmakuDisplaySettingWid::OnLoadSetting(QVariant pConfig) {

	config_t* config = qVPtr<config_t>::toPtr(pConfig);

	DataToWidget(BILI_CONFIG_BOOL(), ui.DanmakuAlwaysOnCheckBox, config, "Danmaku\0AutoStart");
	DataToWidget(BILI_CONFIG_STRING(), ui.CurLiveStateBtnGroup, config, "Danmaku\0ShowLiveStatus");
	DataToWidget(BILI_CONFIG_STRING(), ui.PropsAndLaoYeBtnGroup, config, "Danmaku\0ItemAndLaoyeMessage");
	DataToWidget(BILI_CONFIG_STRING(), ui.AnnounceTipBtnGroup, config, "Danmaku\0SystemAnnounce");
    DataToWidget(BILI_CONFIG_INT(), ui.DanmuOpacitySlider, config, "Danmaku\0DanmuOpacity");


	backupDisplaySetting_ |= int(ui.DanmakuAlwaysOnCheckBox->isChecked()) << 3;
	backupDisplaySetting_ |= int(ui.LiveStateRadioBtnOn->isChecked()) << 2;
	backupDisplaySetting_ |= int(ui.PropsAndLaoYeRadioBtnOn->isChecked()) << 1;
	backupDisplaySetting_ |= int(ui.AnnounceRadioBtnOn->isChecked());

	emit sglSetDMDisplaySettings(backupDisplaySetting_);

    def_opacity_ = ui.DanmuOpacitySlider->value();
    ui.DanmuOpacityValLbl->setText(QString::number(def_opacity_) + "%");
}

void DanmakuDisplaySettingWid::OnCancelSetting(QVariant pConfig) {

	int curSetting = 0;
	curSetting |= int(ui.DanmakuAlwaysOnCheckBox->isChecked()) << 3;
	curSetting |= int(ui.LiveStateRadioBtnOn->isChecked()) << 2;
	curSetting |= int(ui.PropsAndLaoYeRadioBtnOn->isChecked()) << 1;
	curSetting |= int(ui.AnnounceRadioBtnOn->isChecked());
	if (curSetting != backupDisplaySetting_) {
		emit sglSetDMDisplaySettings(backupDisplaySetting_);

		bool state = (backupDisplaySetting_ & 0x0004) >> 2;

		if (state != ui.LiveStateRadioBtnOn->isChecked())
			if (state)
				emit sglCurLiveStateOnOff(-2);
			else
				emit sglCurLiveStateOnOff(-3);

	}

    emit DanmuOpacityChanged(def_opacity_);

}