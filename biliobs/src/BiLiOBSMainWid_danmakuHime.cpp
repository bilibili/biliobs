#include "BiLiOBSMainWid.h"
#include "../biliapi/IBiliApi.h"
#include "BiLiMsgDlg.h"
#include "BiliGlobalDatas.hpp"
#include "BiliOBSUtility.hpp"
#include "danmakusettingdlg.h"
#include "danmakudisplaysettingwid.h"
#include "danmakudetailsettingwid.h"
#include <stdint.h>
#include "net_status_wgt.h"
#include "danmakuhistorysettingwid.h"

#include "oper_tip_dlg.h"
#include "oper_tip_dlg_factory.h"

#include "system_ret_info_dlg.h"

void BiLiOBSMainWid::mStartDanmakuHime()
{
	danmakuHime.reset(new BiliDanmakuHime(gBili_roomId, lexical_cast<int>(gBili_mid)));

	danmakuHime->SetOnAudienceCountProc(std::bind(&BiLiOBSMainWid::OnDanmakuAudienceCount, this, std::placeholders::_1));

	danmakuHime->SetOnJsonProc(std::bind(&BiLiOBSMainWid::OnDanmakuMessage, this, std::placeholders::_1));

	danmakuHime->Start();
}

void BiLiOBSMainWid::mStopDanmakuHime()
{
	if (static_cast<bool>(danmakuHime))
	{
		danmakuHime->Stop();
		danmakuHime.reset();
	}
}

void BiLiOBSMainWid::OnDanmakuAudienceCount(int audienceCount)
{
	gBili_audienceCount = audienceCount;
}

void BiLiOBSMainWid::OnDanmakuMessage(const BiliJsonPtr& jsonPtr)
{
	std::string cmd = jsonPtr->GetVal<JSON_STRING>({"cmd"});
	if (cmd == "SEND_GIFT")
	{
		gBili_rcost = jsonPtr->GetVal<JSON_INTEGER>({ "data", "rcost" });
	}
	else if (cmd == "CUT_OFF")
	{
		worker.AddTask(BiliThreadWorker::TaskT(std::bind(&BiLiOBSMainWid::DanmakuShowCutoffMessageBoxTask, this)));
	}
	else if (cmd == "ROOM_LOCK")
	{
		DanmakuMessageBox(tr("System Notification"), tr("Your room is locked."));
	}
}

void* BiLiOBSMainWid::DanmakuShowCutoffMessageBoxTask()
{
	try
	{
		BiliJsonPtr reasonJson = biliApi->GetCutoffReason(lexical_cast<int>(gBili_roomId));
		std::string msg = reasonJson->GetVal<JSON_STRING>({ "msg" });

		DanmakuMessageBox(tr("System Notification"), msg.c_str());

		return 0;
	}
	catch (CUrlNetworkException&)
	{
		SaveHttpLogToFile(biliApi->GetLastUrl(), "Network error.");
		//网络错误
	}
	catch (JsonDataError&)
	{
		SaveHttpLogToFile(biliApi->GetLastUrl(), biliApi->GetLastContent());
		//服务器内部错
	}

	DanmakuMessageBox(tr("System Notification"), tr("Broadcast is cut off by administrator."));

	return 0;
}
void BiLiOBSMainWid::DanmakuMessageBox(const QString title, const QString text)
{
	if (pthread_equal(uiThread, pthread_self()))
	{
        //OperTipDlg *dlg = OperTipDlgFactory::makeDlg(OperTipDlgFactory::BROADCAST_IS_CUT_OFF);
        //dlg->exec();
        //delete dlg;

        SystemRetInfoDlg dlg;
        dlg.setTitle(title);
        dlg.setSubTitle(tr("Broadcast is cut off by administrator."));
        dlg.setDetailInfo(text);
        dlg.exec();
	}
	else
	{
		QMetaObject::invokeMethod(this, "DanmakuMessageBox", Qt::QueuedConnection, Q_ARG(QString, title), Q_ARG(QString, text));
	}
}

void BiLiOBSMainWid::sltAutoStartDanmaku() {
	
	//必须要有直播间才可以使用
	if (onBroadcastRoomRequested(1)) {
		if (!dmOpt_->startAndShowDM_()){
			SystemRetInfoDlg dlg;
			dlg.setTitle("");
			dlg.setSubTitle(tr("Warning"));
			dlg.setDetailInfo(tr("Can not connect to Danmaku!"));
			dlg.resize(dlg.sizeHint());
			move_widget_to_center(&dlg, this);
			dlg.exec();
		}
	}
}

bool BiLiOBSMainWid::setDMInfoForStart_(DanmakuOpt *dmOpt){

	dmOpt->dmRoomID_ = gBili_roomId;
	dmOpt->dmUID_ = 201606;
	return true;
}

void BiLiOBSMainWid::mSltDanmakuSetting()
{
	auto dmSettingDlg = new DanmakuSettingDlg(static_cast<config_t*>(mBasicConfig));

	auto danmakuDisplaySetWid = new DanmakuDisplaySettingWid(mBasicConfig);
	danmakuDisplaySetWid->setObjectName("DanmakuDisplaySettingWid");
	dmSettingDlg->addStackedPageWid_(0, danmakuDisplaySetWid);

	QObject::connect(danmakuDisplaySetWid, SIGNAL(sglSetDMDisplaySettings(int)), dmOpt_, SLOT(sltSetDMDisplaySettings(int)));
	QObject::connect(danmakuDisplaySetWid, SIGNAL(sglSysAnnounceOnOff(int)), dmOpt_, SLOT(sltSetDMDisplaySysAnnounceOnOff(int)));
	QObject::connect(danmakuDisplaySetWid, SIGNAL(sglCurLiveStateOnOff(int)), dmOpt_, SLOT(sltSetDMDisplayCurLiveStateOnOff(int)));
	QObject::connect(danmakuDisplaySetWid, SIGNAL(sglPropsAndLaoYeOnOff(int)), dmOpt_, SLOT(sltSetDMDisplayPropsAndLaoYeOnOff(int)));
	QObject::connect(danmakuDisplaySetWid, SIGNAL(sglTestDM()), this, SLOT(onTestDmSignal()));
	QObject::connect(danmakuDisplaySetWid, SIGNAL(DanmuOpacityChanged(int)), dmOpt_, SLOT(sltDanmakuOpacityChanged(int)));

	auto danmakuDetailSetWid = new DanmakuDetailSettingWid(mBasicConfig);
	danmakuDetailSetWid->setObjectName("DanmakuDetailSettingWid");
	dmSettingDlg->addStackedPageWid_(1, danmakuDetailSetWid);

	QObject::connect(danmakuDetailSetWid, SIGNAL(sglDanmakuStayTimeChanged(int)), dmOpt_, SLOT(sltDanmakuStayTimeChanged(int)));
	QObject::connect(danmakuDetailSetWid, SIGNAL(sglDanmakuFontSizeChanged(QString)), dmOpt_, SLOT(sltDanmakuFontSizeChanged(QString)));
	QObject::connect(danmakuDetailSetWid, SIGNAL(sglSetDMDetailSettings(QObject *)), dmOpt_, SLOT(sltSetDMDetailSettings(QObject *)));
	QObject::connect(danmakuDetailSetWid, SIGNAL(sglTestDM()), this, SLOT(onTestDmSignal()));

    auto danmaku_history_setting_wid = new DanmakuHistorySettingWid();
    danmaku_history_setting_wid->setObjectName("DanmakuHistorySettingWid");
    dmSettingDlg->addStackedPageWid_(2, danmaku_history_setting_wid);
    QObject::connect(danmaku_history_setting_wid, &DanmakuHistorySettingWid::modeChanged, ui.history_wgt, &BarrageHistory::setFreshMode);

	dmSettingDlg->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
	dmSettingDlg->setWindowFlags(Qt::FramelessWindowHint);
	move_widget_to_center(dmSettingDlg, this);

	dmSettingDlg->show_();
}

void BiLiOBSMainWid::loadDanmakuHistoryConfig()
{
    ui.history_wgt->initByConfig(mBasicConfig);
}

