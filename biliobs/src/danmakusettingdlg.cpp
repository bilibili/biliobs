#include "BiliUIConfigSync.hpp"
#include "danmakusettingdlg.h"

#include <assert.h>

DanmakuSettingDlg::DanmakuSettingDlg(config_t* basicConfig, QWidget *parent)
	: AdvSetttingBaseDlg(parent)
	, mBasicConfig(basicConfig)
{
	setWindowTitle(ui.SettingTitleLab->text());
}

DanmakuSettingDlg::~DanmakuSettingDlg() { 
}

void DanmakuSettingDlg::show_() {
	emit OnLoadSignal(qVPtr<config_t>::toVariant(mBasicConfig));
	QPushButton *btn = getBtnContainerWid_()->findChild<QPushButton *>("DanmakuDisplay");
	btn->setChecked(true);
	exec();
}

int DanmakuSettingDlg::saveSetting_() {
	emit OnSaveSignal(qVPtr<config_t>::toVariant(mBasicConfig));
	return (QDialog::Accepted);
}

int DanmakuSettingDlg::cancelSetting_() {
	emit OnCancelSignal(qVPtr<config_t>::toVariant(mBasicConfig));
	return (QDialog::Rejected);
}

QPushButton *DanmakuSettingDlg::addBtn_(){

	QWidget *btnContainerWid = getBtnContainerWid_();
	QPushButton *btn = new QPushButton(btnContainerWid);
	btnContainerWid->layout()->addWidget(btn);
	btn->setFixedSize(QSize(160, 37));
	btn->setCheckable(true);
	return btn;
}

void DanmakuSettingDlg::addStackedPageWid_(int index, QWidget* wid) {

	getStackedWid_()->insertWidget(index, wid);
	QPushButton *btn = addBtn_();
	btnGroup_->addButton(btn, index);
	btnGroup_->setExclusive(true);

	QMetaObject::Connection saveConnection = QObject::connect(this, SIGNAL(OnSaveSignal(QVariant)), wid, SLOT(OnSaveSetting(QVariant)));
	QMetaObject::Connection loadConnection = QObject::connect(this, SIGNAL(OnLoadSignal(QVariant)), wid, SLOT(OnLoadSetting(QVariant)));
	QMetaObject::Connection cancelConnection = QObject::connect(this, SIGNAL(OnCancelSignal(QVariant)), wid, SLOT(OnCancelSetting(QVariant)));

	assert(static_cast<bool>(saveConnection));
	assert(static_cast<bool>(loadConnection));
	assert(static_cast<bool>(cancelConnection));

	switch (index) {
		case 0:{
			btn->setObjectName("DanmakuDisplay");
			btn->setText(tr("DanmakuDisplay"));
			break;
		}
		case 1:{
			btn->setObjectName("DanmakuDetail");
			btn->setText(tr("DanmakuDetail"));
			break;
		}
        case 2: {
            btn->setObjectName("DanmakuHistory");
            btn->setText(tr("DanmakuHistory"));
            break;
        }
	}
}