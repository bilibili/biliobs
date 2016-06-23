#include "AdvancedSettingWid.h"
#include "VideoSettingWid.h"
#include "AudioDeviceSettingWid.h"
#include "PushStreamSettingWid.h"
#include "HotkeySettingWid.h"
#include <QMouseEvent>
#include <QButtonGroup>

#define ENABLE_CUSTOM_PUSHSTREAM 1

AdvancedSettingWid::AdvancedSettingWid(QWidget *parent)
	: QDialog(parent)
	, mIsPress(false) 
{
	ui.setupUi(this);

	QButtonGroup *btnGroup = new QButtonGroup(this);
	btnGroup->addButton(ui.VideoSettingBtn, 0);
	btnGroup->addButton(ui.AudioSettingBtn, 1);
	btnGroup->setExclusive(true);
#if ENABLE_CUSTOM_PUSHSTREAM
	btnGroup->addButton(ui.PushStreamBtn, 2);
#else
	ui.PushStreamBtn->setHidden(true);
#endif
	btnGroup->addButton(ui.HotkeyBtn, 3);

	connect(btnGroup, SIGNAL(buttonClicked(int)), this, SLOT(mSltStackChanged(int)));

	connect(ui.CloseBtn, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui.CancelBtn, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui.SetBtn, SIGNAL(clicked()), this, SLOT(mSltSaveSetting()));

	ui.SetBtn->setDefault(true);

	setWindowTitle(ui.SettingTitleLab->text());
}

AdvancedSettingWid::~AdvancedSettingWid() {

}

void AdvancedSettingWid::mAddStackedPageWid(int index, QWidget* wid)
{
	 switch (index){
	 case 0:{
		 wid->setParent(ui.VideoSettingPage);
		 ui.VSGLayout->addWidget(wid, 0, 0, 1, 1);
		 break;
	 }
	 case 1:{
		 wid->setParent(ui.AudioSettingPage);
		 ui.ASGLayout->addWidget(wid, 0, 0, 1, 1);
		 break;
	 }
#if ENABLE_CUSTOM_PUSHSTREAM
	 case 2:{
		 wid->setParent(ui.VideoSettingPage);
		 ui.PSGLayout->addWidget(wid, 0, 0, 1, 1);
		 break;
	 }
#endif
	 case 3:{
		 wid->setParent(ui.HotkeyPage);
		 ui.HKGLayout->addWidget(wid, 0, 0, 1, 1);
		 break;
	 }
	 }
} 

void AdvancedSettingWid::mShow() {
	ui.VideoSettingBtn->setChecked(true);
	exec();
}

void AdvancedSettingWid::mSltStackChanged(int index) {
	ui.SettingStackedWid->setCurrentIndex(index);
}

void AdvancedSettingWid::mousePressEvent(QMouseEvent *e) {
	if (e->button() & Qt::LeftButton)
		mIsPress = true;
	mPoint = e->globalPos() - pos();
}

void AdvancedSettingWid::mouseMoveEvent(QMouseEvent *e) {
	if (mIsPress){
		QPoint point = e->globalPos();
		move(point - mPoint);
	}
}

void AdvancedSettingWid::mouseReleaseEvent(QMouseEvent *e) {
	mIsPress = false;
}

void AdvancedSettingWid::mSltSaveSetting(){

	VideoSettingWid *vsw = ui.SettingStackedWid->widget(0)->findChild<VideoSettingWid *>("VideoSetWid");
	AudioDeviceSettingWid *asw = ui.SettingStackedWid->widget(1)->findChild<AudioDeviceSettingWid *>("AudioSetWid");
#if ENABLE_CUSTOM_PUSHSTREAM
	PushStreamSettingWid *psw = ui.SettingStackedWid->widget(2)->findChild<PushStreamSettingWid *>("PushStreamWid");
#endif
	HotkeySettingWid* hsw = ui.SettingStackedWid->widget(3)->findChild<HotkeySettingWid *>("HotkeyWid");

	bool succeed = true;
	succeed = succeed && vsw->SaveConfig();
	succeed = succeed && asw->SaveConfig();
#if ENABLE_CUSTOM_PUSHSTREAM
	succeed = succeed && psw->SaveConfig();
#endif
	succeed = succeed && hsw->SaveConfig();

	if (succeed)
		done(QDialog::Accepted);
}