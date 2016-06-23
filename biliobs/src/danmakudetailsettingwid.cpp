#include "danmakudetailsettingwid.h"

#include <QStyledItemDelegate>
#include <QDesktopWidget>

#include "spc_stride_slider.h"
#include "slider_interface.h"

#define DMStayTimeMAX	15
DanmakuDetailSettingWid::DanmakuDetailSettingWid(ConfigFile &configData, QWidget *parent)
	: QWidget(parent)
	, configData_(configData) {

	ui.setupUi(this);
	
	auto hideOldCtrls = [this](){

		ui.posWid->setHidden(true);
		ui.posXWid->setHidden(true);
		ui.posYWid->setHidden(true);
		ui.widthWid->setHidden(true);
	};
	hideOldCtrls();

	QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
	ui.DMFontSizeComboBox->setItemDelegate(itemDelegate);

	backupDetailSettings_ = new QObject(this);
	ui.ScreenDMRadioBtn->setHidden(true);

	ui.DMWidWidthSlider->setRange(100, 500, 333);
	ui.DMWidthLab->setText(QString("%1px").arg(ui.DMWidWidthSlider->value()));

	ui.DMStayTimeSlider->initDisplay(3, 10, 999);
	ui.DMStayTimeLab->setText(QString("%1s").arg(ui.DMStayTimeSlider->value()));

	dmFontSizeSets.push_back(DanmakuFontSize(10, tr("10")));
	dmFontSizeSets.push_back(DanmakuFontSize(15, tr("15")));
	dmFontSizeSets.push_back(DanmakuFontSize(20, tr("20 (recommend)")));
	dmFontSizeSets.push_back(DanmakuFontSize(25, tr("25")));
	dmFontSizeSets.push_back(DanmakuFontSize(30, tr("30")));
	for (auto &f : dmFontSizeSets)
		ui.DMFontSizeComboBox->addItem(f.itemText_, f.size_);
	ui.DMFontSizeComboBox->setCurrentIndex(2);

	connect(ui.DMStayTimeSlider, SIGNAL(valueChanged(int)), this, SLOT(sltDanmakuStayTimeChanged(int)));
	connect(ui.DMFontSizeComboBox, SIGNAL(currentTextChanged(QString)), this, SIGNAL(sglDanmakuFontSizeChanged(QString)));

	connect(ui.RestoreDefaultBtn, SIGNAL(clicked()), this, SLOT(sltRestoreDefault()));
	connect(ui.TestDMBtn, SIGNAL(clicked()), this, SIGNAL(sglTestDM()));
}

DanmakuDetailSettingWid::~DanmakuDetailSettingWid() {
}

void DanmakuDetailSettingWid::sltDanmakuStayTimeChanged(int staySecond) {

	if (staySecond > 10){
		staySecond = 999;
		ui.DMStayTimeSlider->blockSignals(true);
		ui.DMStayTimeSlider->setValue(DMStayTimeMAX);
		ui.DMStayTimeSlider->blockSignals(false);
	}
	ui.DMStayTimeLab->setText(QString("%1s").arg(staySecond));
	emit sglDanmakuStayTimeChanged(staySecond);
}

void DanmakuDetailSettingWid::sltRestoreDefault(){

	ui.DMStayTimeSlider->blockSignals(true);
	ui.DMFontSizeComboBox->blockSignals(true);

	ui.SideDMRadioBtn->setChecked(true);
	ui.DMStayTimeSlider->setValue(5);
	ui.DMFontSizeComboBox->setCurrentIndex(2);

	ui.DMStayTimeSlider->blockSignals(false);
	ui.DMFontSizeComboBox->blockSignals(false);

	ui.DMStayTimeLab->setText(QString("%1s").arg(ui.DMStayTimeSlider->value()));

	backupDetailSettings_->setProperty("dmStayTime", QVariant(ui.DMStayTimeSlider->value()));
	backupDetailSettings_->setProperty("dmFontSize", QVariant(ui.DMFontSizeComboBox->currentText()));

	emit sglSetDMDetailSettings(backupDetailSettings_);
}

void DanmakuDetailSettingWid::OnSaveSetting(QVariant pConfig)
{
	config_t* config = qVPtr<config_t>::toPtr(pConfig);

	WidgetToData(BILI_CONFIG_STRING(), ui.DMTypeBtnGroup, config, "Danmaku\0ShowType");
	WidgetToData(BILI_CONFIG_STRING(), ui.DMPosBtnGroup, config, "Danmaku\0ShowPosition");
	WidgetToData(BILI_CONFIG_INT(), ui.DMWidWidthSlider, config, "Danmaku\0ShowWidth");
	WidgetToData(BILI_CONFIG_INT(), (SliderInterface*)ui.DMStayTimeSlider, config, "Danmaku\0StayTime");
	WidgetToData(BILI_CONFIG_UINT(), ui.DMFontSizeComboBox, config, "Danmaku\0FontSize");
	WidgetToData(BILI_CONFIG_INT(), ui.DMPosXSlider, config, "Danmaku\0PosXOffSet");
	WidgetToData(BILI_CONFIG_INT(), ui.DMPosYSlider, config, "Danmaku\0PosYOffSet");
}

void DanmakuDetailSettingWid::OnLoadSetting(QVariant pConfig)
{
	config_t* config = qVPtr<config_t>::toPtr(pConfig);

	DataToWidget(BILI_CONFIG_STRING(), ui.DMTypeBtnGroup, config, "Danmaku\0ShowType");
	DataToWidget(BILI_CONFIG_STRING(), ui.DMPosBtnGroup, config, "Danmaku\0ShowPosition");
	DataToWidget(BILI_CONFIG_INT(), ui.DMWidWidthSlider, config, "Danmaku\0ShowWidth");
	DataToWidget(BILI_CONFIG_INT(), (SliderInterface*)ui.DMStayTimeSlider, config, "Danmaku\0StayTime");
	DataToWidget(BILI_CONFIG_UINT(), ui.DMFontSizeComboBox, config, "Danmaku\0FontSize");
	DataToWidget(BILI_CONFIG_INT(), ui.DMPosXSlider, config, "Danmaku\0PosXOffSet");
	DataToWidget(BILI_CONFIG_INT(), ui.DMPosYSlider, config, "Danmaku\0PosYOffSet");

	if (ui.DMStayTimeSlider->value() > 10){
		ui.DMStayTimeSlider->setValue(DMStayTimeMAX);
		ui.DMStayTimeLab->setText(QString("%1px").arg(999));
	}
	else
		ui.DMStayTimeLab->setText(QString("%1s").arg(ui.DMStayTimeSlider->value()));

	backupDetailSettings_->setProperty("dmStayTime", QVariant(ui.DMStayTimeSlider->value()));
	backupDetailSettings_->setProperty("dmFontSize", QVariant(ui.DMFontSizeComboBox->currentText()));

	emit sglSetDMDetailSettings(backupDetailSettings_);
}

void DanmakuDetailSettingWid::sendDetailSettings_() {

	bool isOk;
	ui.SideDMRadioBtn->setChecked(true);

	ui.DMStayTimeSlider->blockSignals(true);
	ui.DMFontSizeComboBox->blockSignals(true);

	bool isChanged = false;
	int val = backupDetailSettings_->property("dmStayTime").toInt(&isOk);
	if (val != ui.DMStayTimeSlider->value())
		isChanged = true;

	QString fontSize = backupDetailSettings_->property("dmFontSize").toString();
	if (fontSize != ui.DMFontSizeComboBox->currentText()){
		ui.DMFontSizeComboBox->setCurrentText(fontSize);
		isChanged = true;
	}

	ui.RightRadioBtn->blockSignals(false);
	ui.DMStayTimeSlider->blockSignals(false);
	ui.DMFontSizeComboBox->blockSignals(false);

	if (isChanged)
		sglSetDMDetailSettings(backupDetailSettings_);

}

void DanmakuDetailSettingWid::OnCancelSetting(QVariant pConfig) {

	sendDetailSettings_();
}
