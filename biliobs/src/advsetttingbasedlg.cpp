#include "advsetttingbasedlg.h"
#include <QMouseEvent>
#include <QButtonGroup>
AdvSetttingBaseDlg::AdvSetttingBaseDlg(QWidget *parent)
	: QDialog(parent),
    mIsPress(false) {
	ui.setupUi(this);

	btnGroup_ = new QButtonGroup(this);
	connect(btnGroup_, SIGNAL(buttonClicked(int)), this, SLOT(sltStackChanged(int)));

	connect(ui.CloseBtn, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui.CancelBtn, SIGNAL(clicked()), this, SLOT(sltCancel()));
	connect(ui.SetBtn, SIGNAL(clicked()), this, SLOT(sltSaveSetting()));

	ui.SetBtn->setDefault(true);
}

AdvSetttingBaseDlg::~AdvSetttingBaseDlg() {

}

void AdvSetttingBaseDlg::sltStackChanged(int index) {
	ui.SettingStackedWid->setCurrentIndex(index);
}

void AdvSetttingBaseDlg::sltSaveSetting(){
	saveSetting_();
	close();
}

void AdvSetttingBaseDlg::sltCancel(){
	cancelSetting_();
	close();
}

void AdvSetttingBaseDlg::mousePressEvent(QMouseEvent *e) {
	if (e->button() & Qt::LeftButton)
		mIsPress = true;
	mPoint = e->globalPos() - pos();
}

void AdvSetttingBaseDlg::mouseMoveEvent(QMouseEvent *e) {
	if (mIsPress){
		QPoint point = e->globalPos();
		move(point - mPoint);
	}
}

void AdvSetttingBaseDlg::mouseReleaseEvent(QMouseEvent *e) {
	mIsPress = false;
}