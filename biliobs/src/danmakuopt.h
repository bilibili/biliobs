#ifndef DANMAKUOPT_H
#define DANMAKUOPT_H

#include <QObject>
#include <QVector>
#include "BiliDanmakuHime.hpp"
#include "../libobs/util/config-file.h"

#define WebSwitchRoom	1

class BiliDanmakuHime;
class DanmakuCtrl;
class SuspendLockWid;
class DanmakuWid;
class DanmakuOpt : public QObject {
	Q_OBJECT
public:
	DanmakuOpt(QObject *parent = 0);
	~DanmakuOpt();

	int dmRoomID_;
	int dmUID_;
	std::function<bool()> getDMInfoForStart_;		//get roomId, UID to start danmaku server
	void receiveDMInfo_(const BiliJsonPtr &jsonPtr);
	bool setDMOnOff_(bool on);
	bool startAndShowDM_();

	void changeOpacity_(int opacity);
	void changeStayTime_(int second);
	void changeFontSize_(QString fontSize);
	void freeDanmaku_();

	bool autoStart_;
	bool showLiveStatus_;
	bool itemAndLaoyeMsg_;
	bool sysAnnounce_;
	bool isAutoStartDM_;
	int dmStayTime_;
	QString dmFontSizeStr_;
    uint64_t danmuOpacity_;
	int posX_;
	int posY_;
	int sizeW_;
	int sizeH_;
	void loadSettings_(QVariant cfg);
	void setCfgToDMWid_(config_t *cfg);

	void setNetState_(int state);
	void setNetUpSpeed_(int speed) ;
	void setFrameLostRate_(float rate);
	void setNumOfAudience_(int num);
	void setNumOfFans_(int num) ;

signals:
	void sglReceiveDM(QString dm);
	void sglReceiveTestDM(QString dm);
	void netData(const QString&);
#if WebSwitchRoom	
	void sglSwitchRoom(int);
#endif
	void sglSettingsDM();

public slots:
	void sltSetDMDetailSettings(QObject *detailSettings);
	void sltSetDMDisplaySettings(int displaySettings);
	void sltSetDMDisplaySysAnnounceOnOff(int index);
	void sltSetDMDisplayCurLiveStateOnOff(int index);
	void sltSetDMDisplayPropsAndLaoYeOnOff(int index);
	void sltDanmakuOpacityChanged(int opacity);
	void sltDanmakuStayTimeChanged(int second);
	void sltDanmakuFontSizeChanged(QString fontSize);
	void sltCreateDanmakuWid();

private:
	BiliDanmakuHime *biliDanmaku_;
	DanmakuCtrl *dmCtrlWid_;
	SuspendLockWid *suspendLockWid_;
	DanmakuWid *dmWid_;

	void setDMWidOnOff_(bool on);
};

#endif // DANMAKUOPT_H
