#ifndef DANMAKUDETAILSETTINGWID_H
#define DANMAKUDETAILSETTINGWID_H

#include <QWidget>
#include <QVariant>
#include "ui_danmakudetailsettingwid.h"

#include "BiliUIConfigSync.hpp"
#include "util/util.hpp"

struct DanmakuFontSize{
	int size_;
	QString itemText_;
	DanmakuFontSize(int size, QString itemText)
		:size_(size)
		, itemText_(itemText){}
};

class DanmakuDetailSettingWid : public QWidget {

	Q_OBJECT

public:
	DanmakuDetailSettingWid(ConfigFile &configData, QWidget *parent = 0);
	~DanmakuDetailSettingWid();

	QObject *backupDetailSettings_;

signals:
	void sglDanmakuWidthChanged(int);
	void sglDanmakuStayTimeChanged(int);
	void sglDanmakuFontSizeChanged(QString);
	void sglSetDMDetailSettings(QObject *);
	void sglTestDM();

public slots:
	void sltDanmakuStayTimeChanged(int staySecond);

	void OnSaveSetting(QVariant pConfig);
	void OnLoadSetting(QVariant pConfig);
	void OnCancelSetting(QVariant pConfig);

	void sltRestoreDefault();

private:
	Ui::DanmakuDetailSettingWid ui;
	ConfigFile &configData_;
	std::vector<DanmakuFontSize> dmFontSizeSets;
	void sendDetailSettings_();
};

#endif // DANMAKUDETAILSETTINGWID_H
