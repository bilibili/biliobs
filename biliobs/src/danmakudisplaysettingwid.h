#ifndef DANMAKUDISPLAYSETTINGWID_H
#define DANMAKUDISPLAYSETTINGWID_H

#include <QWidget>
#include "ui_danmakudisplaysettingwid.h"

#include "BiliUIConfigSync.hpp"
#include "util/util.hpp"

class DanmakuDisplaySettingWid : public QWidget {
	Q_OBJECT

public:
	DanmakuDisplaySettingWid(ConfigFile &configData, QWidget *parent = 0);
	~DanmakuDisplaySettingWid();

signals:
	void sglSetDMDisplaySettings(int);
	void sglSysAnnounceOnOff(int);
	void sglCurLiveStateOnOff(int);
	void sglPropsAndLaoYeOnOff(int);
	void sglTestDM();

    void DanmuOpacityChanged(int);

private slots:
    void onDanmuOpacityChanged(int);

public slots:
	void OnSaveSetting(QVariant pConfig);
	void OnLoadSetting(QVariant pConfig);
	void OnCancelSetting(QVariant pConfig);

	void sltRestoreDefault();


private:
	Ui::DanmakuDisplaySettingWid ui;

	ConfigFile &configData_;
	int backupDisplaySetting_;

    int def_opacity_;
};

#endif // DANMAKUDISPLAYSETTINGWID_H
