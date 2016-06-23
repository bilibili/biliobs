#ifndef BILIOBSSETTINGWID_H
#define BILIOBSSETTINGWID_H

#include <QWidget>
#include "ui_BiLiOBSSettingWid.h"

class VideoSettingWid;

class BiLiOBSSettingWid : public QWidget {

	Q_OBJECT

public:
	BiLiOBSSettingWid(QWidget *parent = 0);
	~BiLiOBSSettingWid();

	void closeEvent(QCloseEvent *e) ;
	void mShow();

private:
	Ui::BiLiOBSSettingWid ui;
	//VideoSettingWid *mVideoSettingWid;
	void mSetupUI();
	void mSetupConnection();
};

#endif // BILIOBSSETTINGWID_H
