#include "BiLiOBSSettingWid.h"
#include "VideoSettingWid.h"
#include "ShadowDlg.h"
#include <QCloseEvent>
#include <QMessageBox>
#include <QTranslator>

BiLiOBSSettingWid::BiLiOBSSettingWid(QWidget *parent)
	: QWidget(parent) {

	mSetupUI();
	mSetupConnection();
}

BiLiOBSSettingWid::~BiLiOBSSettingWid() { }

void BiLiOBSSettingWid::mSetupUI() {

	ui.setupUi(this);
	
	//mVideoSettingWid = new VideoSettingWid(this);
	//ui.SetStackedWid->addWidget(mVideoSettingWid);
	//ui.SetStackedWid->setCurrentWidget(mVideoSettingWid);
}

void BiLiOBSSettingWid::mSetupConnection() {

	connect(ui.CloseSetBtn, SIGNAL(clicked()), parent(), SLOT(close()));
}

void BiLiOBSSettingWid::closeEvent(QCloseEvent *e) {

	e->accept();
}

void BiLiOBSSettingWid::mShow() {

	ui.retranslateUi(this);
	qobject_cast<ShadowDlg *>(parent())->mSetSolidWid(this, size());
	qobject_cast<ShadowDlg *>(parent())->exec();
}
