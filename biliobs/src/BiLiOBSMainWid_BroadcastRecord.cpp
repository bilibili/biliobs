#include "BiLiOBSMainWid.h"
#include "BiLiMsgDlg.h"
#include <QListWidget>
#include <QMessageBox>
#include <QTimer>
#include "BiliOBSUtility.hpp"
#include "BiliBroadcastButtonOperator.hpp"
#include "BiliRecordButtonOperator.hpp"
#include "../biliapi/IBiliApi.h"
#include "BiliGlobalDatas.hpp"
#include <time.h>
#include <QToolTip>
#include "../../libobs/util/threading.h"

#include <QDebug>

#include "bili_counter_trigger.h"
#include "bili-icon-msgdlg.h"
#include "HotkeyTriggeredNotice.h"

#undef max
#undef min

#include "../base/base_paths.h"
#include "../base/strings/string_util.h"
#include "../base/files/file_util.h"
#include "danmakudisplaysettingwid.h"

#include "net_status_wgt.h"


#include "oper_tip_dlg_factory.h"
#include "oper_tip_dlg.h"

#define BITRATE_UPDATE_SECONDS 2

const char* BILI_HOTKEY_BROADCAST_NAME = "biliobs.broadcast-onoff";
const char* BILI_HOTKEY_RECORD_NAME = "biliobs.record-onoff";

class OutputSignalMonitor_DoReconnect : public OutputSignalMonitor
{
	BasicOutputHandler* outputHandler_;
	obs_service_t* obsService_;
	BiliBroadcastButtonOperator* broadcastBtnOp_;
	BiliRecordButtonOperator* recordBtnOp_;

	bool isRecording_;
	bool isStreaming_;

	BiLiOBSMainWid* mainWid_;
	std::unique_ptr<QDialog> msgDlg_;

	bool isDisconnecting;

	volatile long leftTasks;

	static QString tr(const char* s) { return QApplication::translate("OutputSignalMonitor_DoReconnect", s, 0); }

public:
	OutputSignalMonitor_DoReconnect(BasicOutputHandler* outputHandler, obs_service_t* obsService, BiLiOBSMainWid* mainWid, BiliBroadcastButtonOperator* broadcastBtnOp, BiliRecordButtonOperator* recordBtnOp)
	{
		outputHandler_ = outputHandler;
		obsService_ = obsService;
		leftTasks = 0;
		mainWid_ = mainWid;
		broadcastBtnOp_ = broadcastBtnOp;
		recordBtnOp_ = recordBtnOp;
	}

	OutputSignalMonitor* Start(OutputSignalMonitor* next)
	{
		nextOutputSignalMonitor = next;

		isRecording_ = outputHandler_->RecordingActive();
		isStreaming_ = outputHandler_->StreamingActive();

		if (isRecording_ || isStreaming_)
		{
            msgDlg_.reset(OperTipDlgFactory::makeDlg(OperTipDlgFactory::NEED_REPUSH));
			msgDlg_->exec();

			if (msgDlg_->result() == QDialog::Accepted)
			{
				isDisconnecting = true;
				isRecording_ = outputHandler_->RecordingActive();
				isStreaming_ = outputHandler_->StreamingActive();

				if (isRecording_)
				{
					++leftTasks;
					QueueTask(std::bind(&OutputSignalMonitor_DoReconnect::StopRecordingTask, this));
				}

				if (isStreaming_)
				{
					++leftTasks;
					AsyncQueueTask(std::bind(&OutputSignalMonitor_DoReconnect::StopStreamingTask, this));
				}

                msgDlg_.reset(OperTipDlgFactory::makeDlg(OperTipDlgFactory::CUT_RETRY));
                msgDlg_->show();

				return this;
			}
			else
			{
				delete this;
				return next;
			}
		}
		else
		{
			DoReset();
			delete this;
			return next;
		}
	}

	OutputSignalMonitor* RecordingStop() override
	{
		recordBtnOp_->SetIdle();
		nextOutputSignalMonitor = nextOutputSignalMonitor->RecordingStop();
		return finishOneTask();
	}

	OutputSignalMonitor* StreamingStop(int errorcode) override
	{
		//如果不是在正在断开的时候进了这里，那么就是推流失败了……
		//就相当于本来应该进StreamingStart现在进了这里
		//那么也当作操作完成，交给原来的逻辑去显示错误信息即可
		if (isDisconnecting)
		{
			broadcastBtnOp_->SetWorkingInProgress();
			nextOutputSignalMonitor = nextOutputSignalMonitor->StreamingStop(errorcode);
		}
		return finishOneTask();
	}

	OutputSignalMonitor* RecordingStart()
	{
		nextOutputSignalMonitor = nextOutputSignalMonitor->RecordingStart();
		if (isRecording_)
			return finishOneTask();
		else
			return this;
	}

	OutputSignalMonitor* StreamingStart()
	{
		nextOutputSignalMonitor = nextOutputSignalMonitor->StreamingStart();
		if (isStreaming_)
			return finishOneTask();
		else
			return this;
	}

	void* StartStreamingTask()
	{
		outputHandler_->StartStreaming(obsService_);
		return 0;
	}

	void* StartRecordingTask()
	{
		outputHandler_->StartRecording();
		return 0;
	}

	void* StopStreamingTask()
	{
		outputHandler_->StopStreaming();
		return 0;
	}

	void* StopRecordingTask()
	{
		outputHandler_->StopRecording();
		return 0;
	}

	OutputSignalMonitor* finishOneTask()
	{
		if (os_atomic_dec_long(&leftTasks) == 0)
		{
			if (isDisconnecting == true)
			{
				isDisconnecting = false;

				DoReset();

				if (isRecording_)
				{
					++leftTasks;
					recordBtnOp_->SetStarting();
					mainWid_->mInvokeProcdure(std::bind(&OutputSignalMonitor_DoReconnect::StartRecordingTask, this));
				}

				if (isStreaming_)
				{
					++leftTasks;
					broadcastBtnOp_->SetWorkingInProgress();
					mainWid_->mInvokeProcdure(std::bind(&OutputSignalMonitor_DoReconnect::StartStreamingTask, this));
				}
				return this;
			}
			else
			{
                msgDlg_.reset(OperTipDlgFactory::makeDlg(OperTipDlgFactory::USED_PUSHED));
                msgDlg_->exec();

				QueueTask(std::bind(&OutputSignalMonitor_DoReconnect::DeleteThis, this));
				return nextOutputSignalMonitor;
			}
		}
		else
		{
			return this;
		}
	}

	void QueueTask(BiliThreadWorker::TaskT&& task)
	{
		mainWid_->mInvokeProcdure(std::move(task));
	}

	void AsyncQueueTask(BiliThreadWorker::TaskT&& task)
	{
		mainWid_->mPostTask(std::move(task));
	}

	void DoReset()
	{
		mainWid_->sltResetPreviewWid();
		mainWid_->mResetVideo();
		mainWid_->LoadAudioDeviceConfig();
		mainWid_->ResetPreview();
		mainWid_->sltResetSource();
	}

	void* DeleteThis()
	{
		delete this;
		return 0;
	}
};


void BiLiOBSMainWid::OnStartBroadcastButtonClicked()
{
	//如果没有使用自定义推流，则要求有直播间
	if (CheckUseCustomPushStream(mBasicConfig) == false)
	{
		if (onBroadcastRoomRequested(0) == false)
			return;
	}

	broadcastButtonOperator->SetWorkingInProgress();

	if (CheckUseCustomPushStream(this->mBasicConfig) == false)
		mStartDanmakuHime();

	worker.AddTask(std::bind(&BiLiOBSMainWid::mTurnOnRoomAndGetRTMPAddrTask, this));
}

void* BiLiOBSMainWid::mTurnOnRoomAndGetRTMPAddrTask()
{
	try
	{
		obs_data_t* rtmpParams = obs_data_create();

		std::string pushStreamServer;
		std::string pushStreamPath;
		//根据有没有自定义推流设置，决定是服务器上获取推流地址还是直接用设置里的推流地址
		if (CheckUseCustomPushStream(this->mBasicConfig, &pushStreamServer, &pushStreamPath) == false)
		{
			//控制推流状态
			BiliJsonPtr statusMngResult = biliApi->LiveStatusMng(gBili_roomId, true);

			if (statusMngResult->GetVal<JSON_INTEGER>({ "code" }) < 0)
			{
				//获取推流信息失败
				throw BiliCustomException(statusMngResult->GetVal<JSON_STRING>({ "msg" }));
			}

			obs_data_set_string(rtmpParams, "server", statusMngResult->GetVal<JSON_STRING>({ "data", "addr" }).c_str());
			obs_data_set_string(rtmpParams, "key", statusMngResult->GetVal<JSON_STRING>({ "data", "code" }).c_str());
		}
		else
		{
			obs_data_set_string(rtmpParams, "server", pushStreamServer.c_str());
			obs_data_set_string(rtmpParams, "key", pushStreamPath.c_str());
		}

		obs_service_update(mService, rtmpParams);

		obs_data_release(rtmpParams);

		//QMetaObject::invokeMethod(originalWindow, "on_streamButton_clicked");
		QMetaObject::invokeMethod(this, "StartBroadcast");
	}
	catch (BiliCustomException& customException)
	{
		QMetaObject::invokeMethod(this, "OnErrorMessage", Q_ARG(QString, customException.what()));
		QMetaObject::invokeMethod(broadcastButtonOperator.get(), "SetNotRetrived");
	}
	catch (CUrlNetworkException&)
	{
		SaveHttpLogToFile(biliApi->GetLastUrl(), "Network error.");

		//网络错误
		QMetaObject::invokeMethod(this, "OnErrorMessage", Q_ARG(QString, tr("Network error.")));
		QMetaObject::invokeMethod(broadcastButtonOperator.get(), "SetNotRetrived");
	}
	catch (JsonDataError&)
	{
		SaveHttpLogToFile(biliApi->GetLastUrl(), biliApi->GetLastContent());

		//服务器内部错误
		QMetaObject::invokeMethod(this, "OnErrorMessage", Q_ARG(QString, tr("Server error.")));
		QMetaObject::invokeMethod(broadcastButtonOperator.get(), "SetNotRetrived");
	}

	return 0;
}

void BiLiOBSMainWid::onSettingUpdated()
{
	if (CheckUseCustomPushStream(mBasicConfig))
	{
		//使用了自定义推流
		if (isRoomIdGot == false && broadcastButtonOperator->GetStatus() == BiliBroadcastButtonOperator::NO_PUSHSERVER)
			broadcastButtonOperator->SetNormalIdle();
	}
	else
	{
		//没有使用自定义推流
		if (isRoomIdGot == false)
			broadcastButtonOperator->SetNotRetrived();
	}

	mRequestReconnect();
}

void BiLiOBSMainWid::StartBroadcast()
{
	QMetaObject::invokeMethod(broadcastButtonOperator.get(), "SetConnecting");

	broadcastReconnectSignal.reset(
		new OBSSignal(obs_output_get_signal_handler(outputHandler->streamOutput), "reconnect", (signal_callback_t)&BiLiOBSMainWid::OnBroadcastReconnectCallback, this)
		);
	broadcastReconnectedSignal.reset(
		new OBSSignal(obs_output_get_signal_handler(outputHandler->streamOutput), "reconnect_success", (signal_callback_t)&BiLiOBSMainWid::OnBroadcastReconnectedCallback, this)
		);

	if (outputHandler->StartStreaming(mService) == false)
		OnErrorMessage(tr("Fail to start stream"));
}

void BiLiOBSMainWid::OnStopBroadcastButtonClicked()
{
	mDeactivateStatusInfo();
    if (dmOpt_) {
        dmOpt_->setNetState_(-1);
        dmOpt_->setNetUpSpeed_(-1);
    }

	if (static_cast<bool>(outputHandler) == false)
		return;

	if (static_cast<bool>(outputHandler->StreamingActive()))
	{
		auto counterTrigger = BiliCounterTrigger::Create(std::bind(&BiliBroadcastButtonOperator::SetNormalIdle, broadcastButtonOperator.get()));

		counterTrigger->StartPendingAction();
		worker.AddTask(std::bind(&BiLiOBSMainWid::mStopStreamingTask, this, counterTrigger));

		mStopBroadcastTimer(); 

		counterTrigger->StartPendingAction();
		worker.AddTask(std::bind(&BiLiOBSMainWid::mStopDanmakuHimeTask, this, counterTrigger));

		broadcastReconnectSignal.reset();
		broadcastReconnectedSignal.reset();

		broadcastButtonOperator->SetDisconnecting();
		counterTrigger->StartPendingAction();
		worker.AddTask(std::bind(&BiLiOBSMainWid::mTurnOffRoomTask, this, counterTrigger));

		counterTrigger->Activate();
	}
}

void* BiLiOBSMainWid::mStopDanmakuHimeTask(BiliCounterTrigger* counterTrigger)
{
	mStopDanmakuHime();
	counterTrigger->FinishPendingAction();
	return 0;
}

void* BiLiOBSMainWid::mStopStreamingTask(BiliCounterTrigger* counterTrigger)
{
	outputHandler->StopStreaming();
	counterTrigger->FinishPendingAction();
	return 0;
}

void* BiLiOBSMainWid::mTurnOffRoomTask(BiliCounterTrigger* counterTrigger)
{
	try
	{
		if (CheckUseCustomPushStream(this->mBasicConfig) == false && gBili_roomId > 0)
		{
			//控制推流状态
			BiliJsonPtr statusMngResult = biliApi->LiveStatusMng(gBili_roomId, false);

			if (statusMngResult->GetVal<JSON_INTEGER>({ "code" }) < 0)
			{
				//控制房间状态失败
				throw BiliCustomException(statusMngResult->GetVal<JSON_STRING>({ "msg" }));
			}
		}

		counterTrigger->FinishPendingAction();
	}
	catch (BiliCustomException& customException)
	{
		QMetaObject::invokeMethod(this, "OnErrorMessage", Q_ARG(QString, customException.what()));
		QMetaObject::invokeMethod(broadcastButtonOperator.get(), "SetNotRetrived");
	}
	catch (CUrlNetworkException&)
	{
		//网络错误
		QMetaObject::invokeMethod(this, "OnErrorMessage", Q_ARG(QString, tr("Network error.")));
		QMetaObject::invokeMethod(broadcastButtonOperator.get(), "SetNotRetrived");
	}
	catch (JsonDataError&)
	{
		//服务器内部错误
		QMetaObject::invokeMethod(this, "OnErrorMessage", Q_ARG(QString, tr("Server error.")));
		QMetaObject::invokeMethod(broadcastButtonOperator.get(), "SetNotRetrived");
	}

	return 0;
}

void BiLiOBSMainWid::OnRetryButtonClicked()
{
	broadcastButtonOperator->SetWorkingInProgress();

	worker.AddTask(std::bind(&BiLiOBSMainWid::mRetriveUserInfo, this));
}

void* BiLiOBSMainWid::mRetriveUserInfo()
{
	bool succeed = false;
	try
	{
		//获取房间信息
		BiliJsonPtr roomInfoResult = biliApi->GetRoomInfo(lexical_cast<int>(gBili_mid));

		int code = roomInfoResult->GetVal<JSON_INTEGER>({ "code" });
		if (code < 0)
		{
			//获取房间信息失败
			throw BiliCustomException(code, roomInfoResult->GetVal<JSON_STRING>({ "msg" }));
		}

		succeed = true;

		//填写用户名和房间信息
		//biliUi->usernameButton->setText(roomInfoResult->GetVal<JSON_STRING>({ "data", "uname" }).c_str());
		gBili_roomId = roomInfoResult->GetVal<JSON_INTEGER>({ "data", "roomId" });

		//获取弹幕服务器地址
		BiliJsonPtr apiRoomInfoResult = biliApi->GetAPIRoomInfo(gBili_roomId);
		gBili_danmakuServer = apiRoomInfoResult->GetVal<JSON_STRING>({ "data", "cmt" });

		QMetaObject::invokeMethod(broadcastButtonOperator.get(), "SetNormalIdle");
	}
	catch (BiliCustomException& customException)
	{
		switch (customException.code())
		{
		case -700: //用户未开通直播间时候的错误码
			succeed = false;
			isRoomIdGot = false;
			gBili_roomId = -1;
			QMetaObject::invokeMethod(this, "onBroadcastRoomRequested", Q_ARG(int, 1));
			QMetaObject::invokeMethod(broadcastButtonOperator.get(), "SetNormalIdle");
			break;
		default:
			QMetaObject::invokeMethod(this, "OnErrorMessage", Q_ARG(QString, QString(customException.what())));
			QMetaObject::invokeMethod(broadcastButtonOperator.get(), "SetNotRetrived");
			break;
		}
		return 0;
	}
	catch (CUrlNetworkException&)
	{
		//获取推流信息失败，网络错误
		QMetaObject::invokeMethod(this, "OnErrorMessage", Q_ARG(QString, QString(tr("Fail to retrieve pushstream information. Network error."))));
		QMetaObject::invokeMethod(broadcastButtonOperator.get(), "SetNotRetrived");
		return 0;
	}
	catch (JsonDataError&)
	{
		SaveHttpLogToFile(biliApi->GetLastUrl(), biliApi->GetLastContent());

		//获取推流信息失败，服务器错误
		QMetaObject::invokeMethod(this, "OnErrorMessage", Q_ARG(QString, QString(tr("Fail to retrieve pushstream information. Server error."))));
		QMetaObject::invokeMethod(broadcastButtonOperator.get(), "SetNotRetrived");
	}

	if (succeed){
		isRoomIdGot = true;
		QMetaObject::invokeMethod(this, "sltAutoStartDanmaku");
	}
	return 0;
}

void BiLiOBSMainWid::OnWorkingInProgressClicked()
{
	emit OnErrorMessage(tr("Executing network request. Please wait."));
}

void BiLiOBSMainWid::OnBroadcastTimerTick()
{
	time_t currentTime;
	time(&currentTime);

	time_t deltaTime = currentTime - mBroadcastStartTime;

	int hour = deltaTime / 3600;
	int minute = (deltaTime % 3600) / 60;
	int second = deltaTime % 60;

	char buf[64];
	sprintf(buf, "%02d:%02d:%02d", hour, minute, second);

	ui.TimeLab->setText(buf);
}

void BiLiOBSMainWid::OnBroadcastReconnectCallback(BiLiOBSMainWid* This, calldata_t* params)
{
	QMetaObject::invokeMethod(This, "OnBroadcastReconnect");
}

void BiLiOBSMainWid::OnBroadcastReconnect()
{
	on_reconnected_ = true;

	if (dmOpt_)
		dmOpt_->setNetState_(2);		//net broken

	broadcastButtonOperator->SetReconnecting();
}

void BiLiOBSMainWid::OnBroadcastReconnectedCallback(BiLiOBSMainWid* This, calldata_t* params)
{

	QMetaObject::invokeMethod(This, "OnBroadcastReconnected");
}

void BiLiOBSMainWid::OnBroadcastReconnected()
{
    doOnBeginNetStateCatch();
	broadcastButtonOperator->SetBroadcasting();
}

void BiLiOBSMainWid::mSltStreamDelayStarting(int sec)
{
	bili::lock_guard<bili::recursive_mutex> lockguard(outputSignalMonitorMutex);
	outputSignalMonitor = outputSignalMonitor->StreamDelayStarting(sec);
}

OutputSignalMonitor* BiLiOBSMainWid::StreamDelayStarting(int sec)
{
	mDelaySecTotal = mDelaySecStarting = sec;
	mUpdateDelayMsg();
	mActivateStatusInfo();

	return this;
}

void BiLiOBSMainWid::mSltStreamDelayStopping(int sec)
{
	bili::lock_guard<bili::recursive_mutex> lockguard(outputSignalMonitorMutex);
	outputSignalMonitor = outputSignalMonitor->StreamDelayStopping(sec);
}


OutputSignalMonitor* BiLiOBSMainWid::StreamDelayStopping(int sec)
{
	mDelaySecTotal = mDelaySecStopping = sec;
	mUpdateDelayMsg();

	return this;
}

void BiLiOBSMainWid::mSltStreamingStart()
{
	bili::lock_guard<bili::recursive_mutex> lockguard(outputSignalMonitorMutex);
	outputSignalMonitor = outputSignalMonitor->StreamingStart();
}

OutputSignalMonitor* BiLiOBSMainWid::StreamingStart()
{
	//retries = 0;
	mLastBytesSent = 0;
	mLastBytesSentTime = os_gettime_ns();
	mActivateStatusInfo();

	mStartBroadcastTimer();

	broadcastButtonOperator->SetBroadcasting();

	//检查同步录制
	if (config_get_bool(mBasicConfig, "SimpleOutput", "SyncRec"))
	{
		if (recordButtonOperator->GetStatus() == BiliRecordButtonOperator::IDLE)
		{
			mSltOnRecordClicked();
		}
	}

	return this;
}


void BiLiOBSMainWid::mSltStreamingStop(int errorcode)
{
	bili::lock_guard<bili::recursive_mutex> lockguard(outputSignalMonitorMutex);
	outputSignalMonitor = outputSignalMonitor->StreamingStop(errorcode);
}

OutputSignalMonitor* BiLiOBSMainWid::StreamingStop(int errorcode)
{
	const char *errorMessage;

	switch (errorcode) {
	case OBS_OUTPUT_BAD_PATH:
		errorMessage = "Output.ConnectFail.BadPath";
		break;

	case OBS_OUTPUT_CONNECT_FAILED:
		errorMessage = "Output.ConnectFail.ConnectFailed";
		break;

	case OBS_OUTPUT_INVALID_STREAM:
		errorMessage = "Output.ConnectFail.InvalidStream";
		break;

	default:
	case OBS_OUTPUT_ERROR:
		errorMessage = "Output.ConnectFail.Error";
		break;

	case OBS_OUTPUT_DISCONNECTED:
		/* doesn't happen if output is set to reconnect.  note that
		* reconnects are handled in the output, not in the UI */
		errorMessage = "Output.ConnectFail.Disconnected";
	}

	if (broadcastButtonOperator->GetStatus() == BiliBroadcastButtonOperator::CONNECTING)
	{
		broadcastButtonOperator->SetNormalIdle();
		OnErrorMessage(tr("Fail to connect to broadcast server."));
	}

	return this;
}


void BiLiOBSMainWid::mUpdateDelayMsg() {

	QString msg;

	if (mDelaySecTotal) {
		if (mDelaySecStarting && !mDelaySecStopping) {
			msg = QString("DelayStartingIn: %1").arg(QString::number(mDelaySecStarting));
		}
		else if (!mDelaySecStarting && mDelaySecStopping) {
			msg = QString("DelayStoppingIn: %1").arg(QString::number(mDelaySecStopping));
		}
		else if (mDelaySecStarting && mDelaySecStopping) {
			msg = QString("DelayStartingStoppingIn: %1-%2").arg(QString::number(mDelaySecStopping), QString::number(mDelaySecStarting));
		}
		else {
			msg = QString("DelayTotal: %1").arg(QString::number(mDelaySecTotal));
		}
	}
	ui.DelayInfoLab->setText(msg);
}

void BiLiOBSMainWid::mActivateStatusInfo() {

	if (!mActive) {

		/*网络状态窗*/
		//if (dmOpt_->showLiveStatus_)
			doOnBeginNetStateCatch();

		mRefreshTimer = new QTimer(this);
		connect(mRefreshTimer, SIGNAL(timeout()), this, SLOT(mSltUpdateStreamStateInfo()));

		mTotalSeconds = 0;
		mRefreshTimer->start(1000);

		mActive = true;
	}
}

void BiLiOBSMainWid::mDeactivateStatusInfo() {

	if (mRefreshTimer){
		delete mRefreshTimer;
		mRefreshTimer = NULL;
	}
	ui.DropFramesLab->setText("");
	ui.KBPSLab->setText("");
	ui.DelayInfoLab->setText("");

	mDelaySecTotal = 0;
	mDelaySecStarting = 0;
	mDelaySecStopping = 0;

	mActive = false;
}

void BiLiOBSMainWid::mSltUpdateStreamStateInfo() {
	if (!mActive)
		return;
	if (on_reconnected_)
		return;

	++colock_count_;
	/**********************
	 *注意：更新掉帧率后才能更新上传带宽，因为网络状态需要经过上传带宽的最后矫正
	 */
	mUpdateDroppedFrames();
	mUpdateBandwidth();

	mUpdateSessionTime();

	mUpdateAudienceCount();

	if (colock_count_ >= 600) {

		requestRoomInfo();
		colock_count_ = 0;
	}
}

void BiLiOBSMainWid::requestRoomInfo()
{
	if (request_info_.on_request)
		return;

	request_info_.on_request = true;

	worker.AddTask(std::bind(&BiLiOBSMainWid::roomInfoRequestCb, this));
}
void *BiLiOBSMainWid::roomInfoRequestCb()
{

	try
	{
		//获取房间信息
		BiliJsonPtr roomInfoResult = biliApi->GetRoomInfo(lexical_cast<int>(gBili_mid));

		if (roomInfoResult->GetVal<JSON_INTEGER>({ "code" }) < 0)
		{
			//获取房间信息失败
			throw BiliCustomException(roomInfoResult->GetVal<JSON_STRING>({ "msg" }));
		}

		//填写用户名和房间信息
		//biliUi->usernameButton->setText(roomInfoResult->GetVal<JSON_STRING>({ "data", "uname" }).c_str());
		request_info_.fansNum = roomInfoResult->GetVal<JSON_INTEGER>({ "data", "fansNum" });

	}
	catch (...) {
		request_info_.fansNum = 0;
	}

	QMetaObject::invokeMethod(this, "informInfoRequested");

	return 0;

}
void BiLiOBSMainWid::informInfoRequested()
{
	request_info_.on_request = false;

	mUpdateFansCount();
}

void BiLiOBSMainWid::mUpdateBandwidth() {

	if (!outputHandler->streamOutput)
		return;

	if (++mBitrateUpdateSeconds < BITRATE_UPDATE_SECONDS)
		return;


	uint64_t bytesSent = obs_output_get_total_bytes(outputHandler->streamOutput);
	uint64_t bytesSentTime = os_gettime_ns();

	if (bytesSent == 0)
		mLastBytesSent = 0;

	uint64_t bitsBetween = (bytesSent - mLastBytesSent) * 8;
	double timePassed = double(bytesSentTime - mLastBytesSentTime) /
		1000000000.0;
	double kbitsPerSec = double(bitsBetween) / timePassed / 1000.0;

	if (broadcastButtonOperator->GetStatus() == BiliBroadcastButtonOperator::DISCONNECTING)
		dmOpt_->setNetState_(2);	//net broken
	else if (net_ok_by_frame_losed_ && !kbitsPerSec)
		dmOpt_->setNetState_(1);	//net bad
	else if (!net_ok_by_frame_losed_) {
		dmOpt_->setNetState_(1);	//net bad
	} else
		dmOpt_->setNetState_(0);	//net good
	dmOpt_->setNetUpSpeed_(kbitsPerSec);

	if (!(colock_count_ % 3)) {
		dmOpt_->setNetUpSpeed_(kbitsPerSec);
		mLastBytesSent = bytesSent;
		mLastBytesSentTime = bytesSentTime;
		mBitrateUpdateSeconds = 0;
	}

}

void BiLiOBSMainWid::mUpdateDroppedFrames() {

	if (!outputHandler->streamOutput)
		return;

	int totalDropped = obs_output_get_frames_dropped(outputHandler->streamOutput);
	int totalFrames = obs_output_get_total_frames(outputHandler->streamOutput);
	double percent = (double)totalDropped / (double)totalFrames * 100.0;

	if (!totalFrames)
		return;

	dmOpt_->setFrameLostRate_(percent);
	if (totalDropped - frames_losed_at_begin_of_6_s_ > 0) {
		dmOpt_->setNetState_(1);
		net_ok_by_frame_losed_ = false;
	}
	else{
		dmOpt_->setNetState_(0);
		net_ok_by_frame_losed_ = true;
	}

	if (!(colock_count_ % 6)) {
		frames_losed_at_begin_of_6_s_ = obs_output_get_frames_dropped(outputHandler->streamOutput);
	}
}

void BiLiOBSMainWid::mUpdateSessionTime() {

#if 0
	mTotalSeconds++;

	int seconds      = mTotalSeconds % 60;
	int totalMinutes = mTotalSeconds / 60;
	int minutes      = totalMinutes % 60;
	int hours        = totalMinutes / 60;

	QString text;
	text.sprintf("%02d:%02d:%02d", hours, minutes, seconds);
	sessionTime->setText(text);
	sessionTime->setMinimumWidth(sessionTime->width());

	if (reconnectTimeout > 0) {
		QString msg = QTStr("Basic.StatusBar.Reconnecting");
		showMessage(msg.arg(QString::number(retries),
			QString::number(reconnectTimeout)));
		reconnectTimeout--;

	} else if (retries > 0) {
		QString msg = QTStr("Basic.StatusBar.AttemptingReconnect");
		showMessage(msg.arg(QString::number(retries)));
	}
#endif

	if (mDelaySecStopping > 0 || mDelaySecStarting > 0) {
		if (mDelaySecStopping > 0)
			--mDelaySecStopping;
		if (mDelaySecStarting > 0)
			--mDelaySecStarting;
		mUpdateDelayMsg();
	}
}

void BiLiOBSMainWid::mUpdateAudienceCount() {

	dmOpt_->setNumOfAudience_(gBili_audienceCount);
}

void BiLiOBSMainWid::mUpdateFansCount() {

	dmOpt_->setNumOfFans_(request_info_.fansNum);
}

void BiLiOBSMainWid::mSltOnRecordClicked()
{
	if (static_cast<bool>(outputHandler)/* && outputHandler->StreamingActive()*/)
	{
		bool succeedStartRecording = outputHandler->StartRecording();
		if (succeedStartRecording == false)
			emit OnErrorMessage(tr("Fail to start recording."));
	}
	else
		recordButtonOperator->SetFailed();
}

void BiLiOBSMainWid::mSltOnRecordStartingClicked()
{
	recordButtonOperator->SetStarting();
}

void BiLiOBSMainWid::mSltOnRecordingClicked()
{
	if (static_cast<bool>(outputHandler) && outputHandler->RecordingActive())
	{
		outputHandler->StopRecording();
	}
}

void BiLiOBSMainWid::mSltOnRecordStoppingClicked()
{
}

void BiLiOBSMainWid::mSltOnRecordFailClicked()
{
}

void BiLiOBSMainWid::OnRecordTimerTick()
{
	time_t currentTime;
	time(&currentTime);

	time_t deltaTime = currentTime - mRecordStartTime;

	int hour = deltaTime / 3600;
	int minute = (deltaTime % 3600) / 60;
	int second = deltaTime % 60;

	char buf[64];
	sprintf(buf, "%02d:%02d:%02d", hour, minute, second);

	ui.RecTimeLab->setText(buf);
}


void BiLiOBSMainWid::mSltRecordingStart()
{
	bili::lock_guard<bili::recursive_mutex> lockguard(outputSignalMonitorMutex);
	outputSignalMonitor = outputSignalMonitor->RecordingStart();
}

OutputSignalMonitor* BiLiOBSMainWid::RecordingStart()
{
	mStartRecordTimer();

	recordButtonOperator->SetRecording();

	return this;
}

void BiLiOBSMainWid::mSltRecordingStop()
{
	bili::lock_guard<bili::recursive_mutex> lockguard(outputSignalMonitorMutex);
	outputSignalMonitor = outputSignalMonitor->RecordingStop();
}


OutputSignalMonitor* BiLiOBSMainWid::RecordingStop()
{
	recordButtonOperator->SetIdle();

	if (static_cast<bool>(outputHandler)
		&& static_cast<obs_output_t*>(outputHandler->fileOutput))
	{
		obs_data_t* outFileConf = obs_output_get_settings(outputHandler->fileOutput);
		if (outFileConf)
		{
			//获得保存的文件的位置路径
			const char* outFileUrl = obs_data_get_string(outFileConf, "url");
			const char* outFilePath = obs_data_get_string(outFileConf, "path");
			std::string displayFilePath;
			QString savedMessage = QString(tr("Recorded file saved."));

			if (outFilePath)
				displayFilePath = outFilePath;
			else if (outFileUrl)
				displayFilePath = outFileUrl;

			obs_data_release(outFileConf);

			//修改路径中斜杠的正反
			if (!displayFilePath.empty())
			{
				std::string tmp;
				base::ReplaceChars(displayFilePath, "/", "\\", &tmp);
				displayFilePath = tmp;
			}

			//组装显示的信息
			if (!displayFilePath.empty())
			{
				savedMessage.append("\n");
				savedMessage.append(displayFilePath.c_str());
			}

			//判断文件是否存在，显示不同的信息
			if (base::PathExists(base::FilePath::FromUTF8Unsafe(displayFilePath.c_str())))
			{
				QToolTip::showText(QCursor::pos(), savedMessage);
			}
			else
			{
				QToolTip::showText(QCursor::pos(), tr("File not saved. Maybe recording time is too short."));
			}
		}
	}

	return this;
}

void BiLiOBSMainWid::mSltRecordingStarting()
{
	bili::lock_guard<bili::recursive_mutex> lockguard(outputSignalMonitorMutex);
	outputSignalMonitor = outputSignalMonitor->RecordingStarting();
}

OutputSignalMonitor* BiLiOBSMainWid::RecordingStarting()
{
	recordButtonOperator->SetStarting();
	return this;
}

void BiLiOBSMainWid::mSltRecordingStopping()
{
	bili::lock_guard<bili::recursive_mutex> lockguard(outputSignalMonitorMutex);
	outputSignalMonitor = outputSignalMonitor->RecordingStopping();
}

OutputSignalMonitor* BiLiOBSMainWid::RecordingStopping()
{
	mStopRecordTimer();
	recordButtonOperator->SetStopping();
	return this;
}

void BiLiOBSMainWid::mRequestReconnect()
{
	bili::lock_guard<bili::recursive_mutex> lockguard(outputSignalMonitorMutex);
	OutputSignalMonitor_DoReconnect* reconnectMonitor = new OutputSignalMonitor_DoReconnect(outputHandler.get(), mService, this, broadcastButtonOperator.get(), recordButtonOperator.get());
	outputSignalMonitor = reconnectMonitor->Start(outputSignalMonitor);
}

void BiLiOBSMainWid::mStartBroadcastTimer()
{
	time(&mBroadcastStartTime);
	OnBroadcastTimerTick();

	mBroadcastTickTimer.setTimerType(Qt::CoarseTimer);
	mBroadcastTickTimer.setInterval(300);
	mBroadcastTickTimer.start();
}

void BiLiOBSMainWid::mStopBroadcastTimer()
{
	mBroadcastTickTimer.stop();
	ui.TimeLab->setText(" ");
}

void BiLiOBSMainWid::mStartRecordTimer()
{
	time(&mRecordStartTime);
	OnRecordTimerTick();

	mRecordTickTimer.setTimerType(Qt::CoarseTimer);
	mRecordTickTimer.setInterval(300);
	mRecordTickTimer.start();
}

void BiLiOBSMainWid::mStopRecordTimer()
{
	mRecordTickTimer.stop();
	ui.RecTimeLab->setText(" ");
}

void  BiLiOBSMainWid::onTestDmSignal() {

	QVector<QString> dmV;
	dmV.append(QString("%1|:|%2|:|%3").arg("DANMU_MSG").arg(QString("Bilibili")).arg(QString::fromLocal8Bit("测试弹幕...")));
	dmV.append(QString("%1|:|%2|:|%3|:|%4").arg("SEND_GIFT").arg(QString("Bilibili")).arg(QString::fromLocal8Bit("B坷垃")).arg(100));
	dmV.append(QString("%1|:|%2 %3 %4").arg("WELCOME").arg(QString::fromLocal8Bit("年费老爷")).arg("Bilibili").arg(QString::fromLocal8Bit("进入房间")));
	dmV.append(QString("%1|:|%2").arg("SYS_MSG").arg(QString::fromLocal8Bit("系统公告")));

	int a = qrand() % dmV.count();
	QString dm = dmV[a];
	emit dmOpt_->sglReceiveTestDM(dm);
}

void BiLiOBSMainWid::doOnBeginNetStateCatch()
{
	frames_losed_at_begin_of_6_s_ = obs_output_get_frames_dropped(outputHandler->streamOutput);

    if (!on_reconnected_)
		dmOpt_->setNetState_(0);
    else
		dmOpt_->setNetState_(1);

	on_reconnected_ = false;
	colock_count_ = 0;
	requestRoomInfo();
}

static void* ShowNotice(QString icon, QString text)
{
	(new HotkeyTriggeredNotice(icon, text))->show();
	return 0;
}

void BiLiOBSMainWid::OnBroadcastHotkey(bool isPressed)
{
	if (isPressed)
	{
		if (broadcastButtonOperator->GetStatus() == BiliBroadcastButtonOperator::NORMAL_IDLE)
		{
			mInvokeProcdure(BiliThreadWorker::TaskT(std::bind(&ShowNotice, ":/HotkeyTriggeredNotice/broadcast-start", tr("Broadcast started"))));
			QMetaObject::invokeMethod(this, "OnStartBroadcastButtonClicked");
		}
		else if (broadcastButtonOperator->GetStatus() == BiliBroadcastButtonOperator::BROADCASTING
			|| broadcastButtonOperator->GetStatus() == BiliBroadcastButtonOperator::RECONNECTING)
		{
			mInvokeProcdure(BiliThreadWorker::TaskT(std::bind(&ShowNotice, ":/HotkeyTriggeredNotice/broadcast-stop", tr("Broadcast stopped"))));
			QMetaObject::invokeMethod(this, "OnStopBroadcastButtonClicked");
		}
	}
}

void BiLiOBSMainWid::OnRecordHotkey(bool isPressed)
{
	if (isPressed)
	{
		if (recordButtonOperator->GetStatus() == BiliRecordButtonOperator::IDLE)
		{
			mInvokeProcdure(BiliThreadWorker::TaskT(std::bind(&ShowNotice, ":/HotkeyTriggeredNotice/record-start", tr("Recorde started"))));
			QMetaObject::invokeMethod(this, "mSltOnRecordClicked");
		}
		else if (recordButtonOperator->GetStatus() == BiliRecordButtonOperator::RECORDING)
		{
			mInvokeProcdure(BiliThreadWorker::TaskT(std::bind(&ShowNotice, ":/HotkeyTriggeredNotice/record-stop", tr("Record stoppped"))));
			QMetaObject::invokeMethod(this, "mSltOnRecordingClicked");
		}
	}
}
