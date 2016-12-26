#include "BiLiMsgDlg.h"

#ifndef USE_NEW_MSGDLG

#include <QMouseEvent>

BiLiMsgDlg::BiLiMsgDlg(QWidget *parent)
	: QDialog(parent)
	, mIsPress(false) {

	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);

	connect(ui.CloseBtn, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui.CancelBtn, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui.OkBtn, SIGNAL(clicked()), this, SLOT(mSltOkBtn()));
	ui.OkBtn->setDefault(true);

	setWindowTitle(tr("Information"));
}

BiLiMsgDlg::~BiLiMsgDlg() {
}

void BiLiMsgDlg::mSltOkBtn() {
	done(QDialog::Accepted);
}

void BiLiMsgDlg::mSetTitle(QString titleStr) {

	ui.TitleLab->setText(titleStr);
	setWindowTitle(titleStr);

	ui.TitleLab->adjustSize();
	this->adjustSize();
}

void BiLiMsgDlg::mSetMsgTxtAndBtn(QString msgTxt, bool hasCancelBtn){
	if (!hasCancelBtn)
		ui.CancelBtn->setHidden(true);
	ui.MsgLab->setText(msgTxt);

	ui.MsgLab->adjustSize();
	this->adjustSize();
}

void BiLiMsgDlg::mousePressEvent(QMouseEvent *e) {
	if (e->button() & Qt::LeftButton)
		mIsPress = true;
	mPoint = e->globalPos() - pos();
}
void BiLiMsgDlg::mouseMoveEvent(QMouseEvent *e) {
	if (mIsPress)
		move(e->globalPos() - mPoint);
}
void BiLiMsgDlg::mouseReleaseEvent(QMouseEvent *e) {
	mIsPress = false;
}

void BiLiMsgDlg::mSetOkButtonText(QString str)
{
	ui.OkBtn->setText(str);
	ui.OkBtn->adjustSize();
}

void BiLiMsgDlg::mSetCancelButtonText(QString str)
{
	ui.CancelBtn->setText(str);
	ui.CancelBtn->adjustSize();
}


#else

BiLiMsgDlg::BiLiMsgDlg(QWidget *parent)
	: BiLiIconMsgDlg(parent)
{
	SetLargeIcon("");
	setWindowTitle("");
}

BiLiMsgDlg::~BiLiMsgDlg()
{
}

void BiLiMsgDlg::mSetMsgTxtAndBtn(QString msgTxt, bool hasCancelBtn)
{
	BiLiIconMsgDlg::SetText(msgTxt);
	if (hasCancelBtn == false)
		BiLiIconMsgDlg::SetCancelButtonText("");
}

void BiLiMsgDlg::mSetTitle(QString titleStr)
{
	QDialog::setWindowTitle(titleStr); //跳过设置窗口标题
	BiLiIconMsgDlg::SetSubTitle(titleStr);
}

void BiLiMsgDlg::mSetOkButtonText(QString str)
{
	BiLiIconMsgDlg::SetOkButtonText(str);
}

void BiLiMsgDlg::mSetCancelButtonText(QString str)
{
	BiLiIconMsgDlg::SetCancelButtonText(str);
}

#endif
