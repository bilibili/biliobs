#include "danmakuopt.h"
#include "danmakuwid.h"
#include "suspendlockwid.h"
#include "BiliUIConfigSync.hpp"

#include <QTimer>
#include <QTime>
#include <windows.h>
#include <QDesktopWidget>

#include "BiliGlobalDatas.hpp"

#define DM_INTERVAL	10
#define DM_FADEOUT_TIME	2
#define DM_FONT_SIZE 20

DanmakuOpt::DanmakuOpt(QObject *parent)
	: QObject(parent)
	, biliDanmaku_(nullptr)
	, dmWid_(nullptr){

	QObject::connect(this, &DanmakuOpt::sglReceiveTestDM, [this](QString dm){
		if (!dmWid_){
			if ( setDMOnOff_(true) )
				emit dmWid_->sglSendDM(dm);
		}
		else
			emit dmWid_->sglSendDM(dm);
	});
}

DanmakuOpt::~DanmakuOpt() { }

bool DanmakuOpt::startAndShowDM_() {

	bool ret = setDMOnOff_(true);
	if (ret){
		if (!isAutoStartDM_)
			dmWid_->hide();
	}
	return ret;
}

bool DanmakuOpt::setDMOnOff_(bool on) {

	if (on)
		if (!getDMInfoForStart_())
			return false;

	if (biliDanmaku_){
		biliDanmaku_->Stop();
		delete biliDanmaku_;
		biliDanmaku_ = nullptr;
	}

	if (on){
		biliDanmaku_ = new BiliDanmakuHime(dmRoomID_, dmUID_);
		biliDanmaku_->SetOnAudienceCountProc([this](int audienceCount)->void{
			Q_UNUSED(audienceCount);
		});
		biliDanmaku_->SetOnJsonProc(std::bind(&DanmakuOpt::receiveDMInfo_, this, std::placeholders::_1));

		bool ret = biliDanmaku_->Start();
		if (ret)
			setDMWidOnOff_(on);
		return ret;
	}
	setDMWidOnOff_(on);
	return true;
}

void DanmakuOpt::setCfgToDMWid_(config_t *cfg) {

	if (dmWid_){
		config_set_int(cfg, "Danmaku", "PosX", dmWid_->pos().x());
		config_set_int(cfg, "Danmaku", "PosY", dmWid_->pos().y());
		config_set_int(cfg, "Danmaku", "SizeW", dmWid_->width());
		config_set_int(cfg, "Danmaku", "SizeH", dmWid_->height());
	}
}

void DanmakuOpt::sltCreateDanmakuWid() {

	dmWid_ = new DanmakuWid;
	if (sizeW_)
		dmWid_->setGeometry(posX_, posY_, sizeW_, sizeH_);
	else {
		QRect curScreenRect = QApplication::desktop()->screenGeometry(QCursor::pos());
		dmWid_->resize(dmWid_->width(), curScreenRect.height());
		dmWid_->move(curScreenRect.topRight() - QPoint(dmWid_->width() + 10, 0));
	}

	QObject::connect(dmWid_, SIGNAL(sglSettingsDM()), this, SIGNAL(sglSettingsDM()));

	dmWid_->show();
	SetWindowPos((HWND)dmWid_->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	dmWid_->initBKPix_();
	dmWid_->initDMThreads_();
	dmWid_->setDMParams_();
	dmWid_->startDMThreads_();

	QObject::connect(this, SIGNAL(sglReceiveDM(QString)), dmWid_, SIGNAL(sglSendDM(QString)), Qt::QueuedConnection);
}

void DanmakuOpt::setDMWidOnOff_(bool on){

	if (dmWid_) {
		dmWid_->close();
		dmWid_ = nullptr;
	}
	if (on)
		QMetaObject::invokeMethod(this, "sltCreateDanmakuWid", Qt::QueuedConnection);
}

void DanmakuOpt::changeOpacity_(int opacity) {

	if (dmWid_)
		dmWid_->changeOpacity_(opacity);
}

void DanmakuOpt::changeStayTime_(int second) {

	if (dmWid_)
		dmWid_->changeStayTime_(second);
}

void DanmakuOpt::changeFontSize_(QString fontSize) {

	if (dmWid_)
		dmWid_->changeFontSize_(fontSize);
}

void DanmakuOpt::freeDanmaku_() {

	setDMOnOff_(false);
}

void DanmakuOpt::receiveDMInfo_(const BiliJsonPtr &jsonPtr){

	QString dm = QString("dm");
	QString cmd = QString::fromStdString(jsonPtr->GetVal<JSON_STRING>({ "cmd" }));
	if (!cmd.compare("DANMU_MSG", Qt::CaseInsensitive)){

		JsonArrayPtr jaP = jsonPtr->GetVal<JSON_ARRAY>({ "info" });
		QString dmContent = QString::fromStdString(jaP->GetVal<JSON_STRING>(1));

		JsonArrayPtr sender_msg = jaP->GetVal<JSON_ARRAY>(2);
		QString dmSender = QString::fromStdString(sender_msg->GetVal<JSON_STRING>(1));

#if WebSwitchRoom	
		QString dmSenderMID = QString::number(jaP->GetVal<JSON_ARRAY>(2)->GetVal<JSON_INTEGER>(0));
		QString senderCmdStr = QString("[%1]>>").arg(dmSenderMID);
		QString switchCmdStr = QString("[%1]>>").arg(QString::fromStdString(gBili_mid));
		if (!senderCmdStr.compare(switchCmdStr)){
			if (dmContent.startsWith(switchCmdStr, Qt::CaseInsensitive)){
				dmContent = dmContent.remove(switchCmdStr);
				dmContent.trimmed();
				bool isOk;
				int roomId = dmContent.toInt(&isOk, 10);
				if (isOk){
					emit sglSwitchRoom(roomId);
					return;
				}
			}
		}
#endif
		dm = QString("%1|:|%2|:|%3").arg("DANMU_MSG").arg(dmSender).arg(dmContent);
		emit sglReceiveDM(dm);

		try {
			int sender_id = sender_msg->GetVal<JSON_INTEGER>(0);
			bool is_admin = (sender_msg->GetVal<JSON_INTEGER>(2) == 1);
			bool is_vip = (sender_msg->GetVal<JSON_INTEGER>(3) == 1) || (sender_msg->GetVal<JSON_INTEGER>(4) == 1);
			bool is_self = atoi(gBili_mid.c_str()) == sender_id;
			QString ret_msg(":");
			if (is_self)
				ret_msg.append("s");
			else if (is_admin)
				ret_msg.append("f");
			else if (is_vip)
				ret_msg.append("v");
			else
				ret_msg.append(' ');

			ret_msg.append(':');
			ret_msg.append(dmSender);

			ret_msg.append(':');
			ret_msg.append(dmContent);

			emit netData(ret_msg);
		}
		catch (...) {
			emit netData(QString(dmSender + ":" + dmContent));
		}

	}
	else if (!cmd.compare("SEND_GIFT", Qt::CaseInsensitive)){

		if (!itemAndLaoyeMsg_)
			return;
		QString giftSenderNam = QString::fromStdString(jsonPtr->GetVal<JSON_STRING>({ "data", "uname" }));
		QString giftName = QString::fromStdString(jsonPtr->GetVal<JSON_STRING>({ "data", "giftName" }));
		QString giftCount = QString::number(jsonPtr->GetVal<JSON_INTEGER>({ "data", "num" }));
		dm = QString("%1|:|%2|:|%3|:|%4").arg("SEND_GIFT").arg(giftSenderNam).arg(giftName).arg(giftCount);
		emit sglReceiveDM(dm);
		emit netData(QString("%1: %2X%3").arg(giftSenderNam).arg(giftName).arg(giftCount));
	}
	else if (!cmd.compare("WELCOME", Qt::CaseInsensitive)){

		if (!itemAndLaoyeMsg_)
			return;
		int64_t vip;
		if (jsonPtr->TryGetVal<JSON_INTEGER>(&vip, { "data", "svip" })) {
			if (vip == 1){
				QString vipName = QString::fromStdString(jsonPtr->GetVal<JSON_STRING>({ "data", "uname" }));
				dm = QString("%1|:|%2 %3 %4").arg("WELCOME").arg(QString::fromLocal8Bit("年费老爷")).arg(vipName).arg(QString::fromLocal8Bit("进入房间"));
				emit sglReceiveDM(dm);
			}
		}
		else if (jsonPtr->TryGetVal<JSON_INTEGER>(&vip, { "data", "vip" })) {
			if (vip == 1){
				QString vipName = QString::fromStdString(jsonPtr->GetVal<JSON_STRING>({ "data", "uname" }));
				dm = QString("%1|:|%2 %3 %4").arg("WELCOME").arg(QString::fromLocal8Bit("月费老爷")).arg(vipName).arg(QString::fromLocal8Bit("进入房间"));
				emit sglReceiveDM(dm);
			}
		}
	}
	else if (!cmd.compare("SYS_MSG", Qt::CaseInsensitive)){
		if (!sysAnnounce_)
			return;
		QString sysMsg = QString::fromStdString(jsonPtr->GetVal<JSON_STRING>({ "msg" }));
		dm = QString("%1|:|%2").arg("SYS_MSG").arg(sysMsg);
		dm.remove(QString(":?"));
		emit sglReceiveDM(dm);
	}
}

void DanmakuOpt::loadSettings_(QVariant cfg){

	config_t *c = qVPtr<config_t>::toPtr(cfg);

	//danmaku display settings
	int dmDisplaySettings = 0;

	dmDisplaySettings |= (config_get_bool(c, "Danmaku", "AutoStart") ? 1 : 0) << 3;
	const char *showLiveStatus = config_get_string(c, "Danmaku", "ShowLiveStatus");
	if (showLiveStatus)
		dmDisplaySettings |= (QString(showLiveStatus).endsWith("on", Qt::CaseInsensitive) ? 1 : 0) << 2;
	else{
		config_set_string(c, "Danmaku", "ShowLiveStatus", "LiveStateRadioBtnOn");
		dmDisplaySettings |= 1 << 2;
	}
	const char *itemAndLaoyeMsg = config_get_string(c, "Danmaku", "ItemAndLaoyeMessage");
	if (itemAndLaoyeMsg)
		dmDisplaySettings |= (QString(itemAndLaoyeMsg).endsWith("on", Qt::CaseInsensitive) ? 1 : 0) << 1;
	else{
		config_set_string(c, "Danmaku", "ItemAndLaoyeMessage", "PropsAndLaoYeRadioBtnOn");
		dmDisplaySettings |= 1 << 1;
	}
	const char *sysAnnounce = config_get_string(c, "Danmaku", "SystemAnnounce");
	if (sysAnnounce)
		dmDisplaySettings |= (QString(sysAnnounce).endsWith("on", Qt::CaseInsensitive) ? 1 : 0);
	else{
		config_set_string(c, "Danmaku", "SystemAnnounce", "AnnounceRadioBtnOn");
		dmDisplaySettings |= 1;
	}

    danmuOpacity_ = config_get_uint(c, "Danmaku", "DanmuOpacity");
    if (!danmuOpacity_) {
        config_set_uint(c, "Danmaku", "DanmuOpacity", 100);
        danmuOpacity_ = 100;
    }

    posX_ = config_get_int(c, "Danmaku", "PosX");
	posY_ = config_get_int(c, "Danmaku", "PosY");
	sizeW_ = config_get_int(c, "Danmaku", "SizeW");
	sizeH_ = config_get_int(c, "Danmaku", "SizeH");

	QDesktopWidget *dk = QApplication::desktop();
	int screenCount = dk->screenCount();
	bool isContain = false;
	for (int i = 0; i < screenCount; ++i) {
		QWidget* currentScreen = dk->screen(i);
		isContain = currentScreen->geometry().contains(QPoint(posX_ + sizeW_, posY_ + sizeH_));
		if (isContain)
			break;
	}
	if (!isContain)
		posX_ = posY_ = sizeW_ = sizeH_ = 0;
    
	autoStart_ = (dmDisplaySettings & 0x8) >> 3;
	showLiveStatus_ = (dmDisplaySettings & 0x4) >> 2;
	itemAndLaoyeMsg_ = (dmDisplaySettings & 0x2) >> 1;
	sysAnnounce_ = (dmDisplaySettings & 0x1);

	//danmaku detail setting
	bool isSideDanmaku;
	const char *danmakuType = config_get_string(c, "Danmaku", "ShowType");
	if (danmakuType)
		isSideDanmaku = QString(danmakuType).startsWith("SideDM", Qt::CaseInsensitive) ? true : false;
	else
		isSideDanmaku = true;
	if (isSideDanmaku){
		//side danmaku
		int val = config_get_int(c, "Danmaku", "StayTime");
		if (val)
			dmStayTime_ = val;
		else{
			dmStayTime_ = 5;
			config_set_int(c, "Danmaku", "StayTime", dmStayTime_);
		}
		uint fs = config_get_uint(c, "Danmaku", "FontSize");
		if (fs)
			dmFontSizeStr_ = QString::number(fs);
		else{
			dmFontSizeStr_ = QString::number(20);
			config_set_uint(c, "Danmaku", "FontSize", 20);
		}
	}
	else{
		//top danmaku
	}

	autoStart_ = true;
	if (autoStart_)
		isAutoStartDM_ = true;
}

void DanmakuOpt::sltSetDMDetailSettings(QObject *detailSettings) {


	bool isOk = false;

	int val = detailSettings->property("dmStayTime").toInt(&isOk);
	if (isOk)
		changeStayTime_(val);

	QString fontSize = detailSettings->property("dmFontSize").toString();
	dmFontSizeStr_ = fontSize;
	changeFontSize_(fontSize);
}

void DanmakuOpt::sltSetDMDisplaySettings(int displaySettings) {

	int dmDisplaySetting = displaySettings;

	autoStart_ = (dmDisplaySetting & 0x8) >> 3;
	showLiveStatus_ = (dmDisplaySetting & 0x4) >> 2;
	itemAndLaoyeMsg_ = (dmDisplaySetting & 0x2) >> 1;
	sysAnnounce_ = (dmDisplaySetting & 0x1);
}

void DanmakuOpt::setNetState_(int state) {

	if (dmWid_)
		dmWid_->setNetState_(state);
}

void DanmakuOpt::setNetUpSpeed_(int speed) {

	if (dmWid_)
		dmWid_->setNetUpSpeed_(speed);
}

void DanmakuOpt::setFrameLostRate_(float rate) {

	if (dmWid_)
		dmWid_->setFrameLostRate_(rate);
}

void DanmakuOpt::setNumOfAudience_(int num) {

	if (dmWid_)
		dmWid_->setNumOfAudience_(num);
}

void DanmakuOpt::setNumOfFans_(int num) {

	if (dmWid_)
		dmWid_->setNumOfFans_(num);
}

void DanmakuOpt::sltSetDMDisplaySysAnnounceOnOff(int index) {
	sysAnnounce_ = (index == -2) ? true : false;
}

void DanmakuOpt::sltSetDMDisplayCurLiveStateOnOff(int index) {
	showLiveStatus_ = (index == -2) ? true : false;
}

void DanmakuOpt::sltSetDMDisplayPropsAndLaoYeOnOff(int index) {
	itemAndLaoyeMsg_ = (index == -2) ? true : false;
}

void DanmakuOpt::sltDanmakuOpacityChanged(int opacity) {
	changeOpacity_(opacity);
}

void DanmakuOpt::sltDanmakuStayTimeChanged(int second) {
	changeStayTime_(second);
}

void DanmakuOpt::sltDanmakuFontSizeChanged(QString fontSize) {
	changeFontSize_(fontSize);
}