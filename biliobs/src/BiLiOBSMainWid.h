#ifndef BILIOBSMAINWID_H
#define BILIOBSMAINWID_H

#include "../common/bili-stdmutex.h"
#include "ui_BiLiOBSMainWid.h"
#include "util/util.hpp"
#include "obs.h"
#include "obs.hpp"
#include "util/platform.h"
#include "BiLiSceneListWidgetOperator.hpp"
#include "BiliDanmakuHime.hpp"
#include <unordered_map>
#include <QWidget>
#include <QPixmap>
#include "../src_obs/window-basic-preview.hpp"
#include "../src_obs/window-basic-main-outputs.hpp"
#include "bili-thread-worker.h"
#include <QTimer>
#include "danmakuopt.h"

#include "wcs_weak_ref.h"

#define SIMPLE_ENCODER_X264	"x264"

class QTranslator;
class QMenu;
class QAction;
class BiliSceneListWidgetOperator;
class BiliBroadcastButtonOperator;
class BiliRecordButtonOperator;
class IBiliAPI;
class BiLiAudioDevSettingWid;
class AudioDevControl;
class BiLiUserInfoWid;
class bili_area_cap;
class BiliCounterTrigger;
class BiLiMsgDlg;
class SystemRetInfoDlg;
class NetStatusWgt;
class DanmakuDisplaySettingWid;
class OutputSignalMonitor
{
public:
	OutputSignalMonitor()
		: nextOutputSignalMonitor(0) {
	}
	virtual OutputSignalMonitor* RecordingStart() {
		nextOutputSignalMonitor = nextOutputSignalMonitor->RecordingStart();
		return this;
	}
	virtual OutputSignalMonitor* RecordingStop() {
		nextOutputSignalMonitor = nextOutputSignalMonitor->RecordingStop();
		return this;
	}
	virtual OutputSignalMonitor* RecordingStarting() {
		nextOutputSignalMonitor = nextOutputSignalMonitor->RecordingStarting();
		return this;
	}
	virtual OutputSignalMonitor* RecordingStopping() {
		nextOutputSignalMonitor = nextOutputSignalMonitor->RecordingStarting();
		return this;
	}
	virtual OutputSignalMonitor* StreamDelayStarting(int sec) {
		nextOutputSignalMonitor = nextOutputSignalMonitor->StreamDelayStarting(sec);
		return this;
	}
	virtual OutputSignalMonitor* StreamDelayStopping(int sec) {
		nextOutputSignalMonitor = nextOutputSignalMonitor->StreamDelayStopping(sec);
		return this;
	}
	virtual OutputSignalMonitor* StreamingStart() {
		nextOutputSignalMonitor = nextOutputSignalMonitor->StreamingStart();
		return this;
	}
	virtual OutputSignalMonitor* StreamingStop(int errorcode) {
		nextOutputSignalMonitor = nextOutputSignalMonitor->StreamingStop(errorcode);
		return this;
	}

	OutputSignalMonitor* nextOutputSignalMonitor;
};

class OutputSignalMonitor_DoReconnect;

class BiLiOBSMainWid : public QWidget, public OutputSignalMonitor {
	Q_OBJECT

		friend class OutputSignalMonitor_DoReconnect;

public:
	BiLiOBSMainWid(IBiliAPI* papi, QWidget *parent = 0);
	~BiLiOBSMainWid();

	void closeEvent(QCloseEvent *e);
	void mShow();
	void mOBSInit();

protected:
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	bool eventFilter(QObject *obj, QEvent *e);

    void showEvent(QShowEvent *e) override;

private:
	Ui::BiLiOBSMainWid ui;

	pthread_t uiThread;

	QLabel *usr_face_lbl_;
	QLabel *usr_name_lbl_;
	QSpacerItem *usr_face_btn_spacer0_;
    QSpacerItem *usr_face_btn_spacer1_;

	//Main Interface
	bool isRightTabShow_;
	bool mIsChineseLang;
	QTranslator *mTrans;
	bool mIsPressed;
	QPoint mPoint;

	void mSetupUI();
	void mSetupConnection();

	QList<QPushButton *> mFuncBtnList;

	//Audio Device Setting
	BiLiAudioDevSettingWid *mAudioDevSettingWid;
	//QWidget *mAudioSettingShadowWid;
	std::vector<AudioDevControl *> mAudioDevCtrlItemsV;

	//User Info
	//QWidget *mUserInfoWidShadow;
	BiLiUserInfoWid *mBiLiUserInfoWid;

	//More Menu 
	QMenu *mMoreMenu;
	QVector<QAction *> mMoreMenuActV;
	QAction *mLiveCommentingAct;
	QAction *mUsingBootAct;
	QAction *mHelpAct;
	QAction *mUpdateCheckAct;
	QAction *mLogoutAct;
	void mCreateRightMenu();
	void mShowRightMenu(QPoint pos);

	//Monitor And Window Menu 
	QMenu *monitorAndWinMenu_;

	//DanmakuHime Setting
	QMenu *danmakuSettingMenu_;

	//Danmaku
	DanmakuOpt *dmOpt_;
	bool setDMInfoForStart_(DanmakuOpt *dmOpt);

	//OBS
	OBSScene mGetCurrentScene();
	ConfigFile    mBasicConfig;
	os_inhibit_t  *mSleepInhibitor = nullptr;
	OBSService    mService;
	std::unordered_map<obs_source_t*, int> mSourceSceneRefs;

	bool mLoaded = false;
	long mDisableSaving = 1;
	bool mProjectChanged = false;

	void mInitOBSAudioCallbacks();
	bool mInitBasicConfig();
	bool mInitBasicConfigDefaults();

public:
	int  mResetVideo();
	bool mResetAudio();

private:
	void mGetConfigFPS(uint32_t &num, uint32_t &den) const;
	void mGetFPSInteger(uint32_t &num, uint32_t &den) const;
	void mGetFPSFraction(uint32_t &num, uint32_t &den) const;
	void mGetFPSNanoseconds(uint32_t &num, uint32_t &den) const;
	void mGetFPSCommon(uint32_t &num, uint32_t &den) const;

	//Init
	void mResizePreview(uint32_t cx, uint32_t cy);
	bool mInitService();
	bool mLoadService();
	void mInitPrimitives();
	void mLoad(const char *file);
	void loadImgAtNoItems();
	void mCreateDefaultScene(bool firstStart);
	void mClearSceneData();
	void mSaveProject();
	void mSaveProjectDeferred();
	void mSave(const char *file);
	obs_data_array_t *mSaveSceneListOrder();
	void mLoadSceneListOrder(obs_data_array_t *array);

	private slots:
	void OnUserInfoGot(QString userName, QPixmap userFace);
	void OnErrorMessage(QString msg);
	void on_UserInfoBtn_clicked();

	public slots:
	void OnOpenRoomClicked();
	void OnRoomMangerClicked();
	void OnInfomodifyClicked();

	public slots:
	void OnMinimizeButtonClicked();
	void OnDanmakuButtonClicked();
	void OnWinAndMonitorSourceButtonClicked();

#if WebSwitchRoom	
	void sltSwitchRoom_(int roomId);
#endif

protected:
	bool isRoomIdGot;

private:
	//OBS Operator 
	std::unique_ptr<BiliSceneListWidgetOperator> sceneListWidgetOperator;
	std::unique_ptr<BiliBroadcastButtonOperator> broadcastButtonOperator;
	std::unique_ptr<BiliRecordButtonOperator> recordButtonOperator;

	//User Info
	QString mUserName;
	QPixmap mUserFace;

public:
	int  mGetProfilePath(char *path, size_t size, const char *file) const;
	//OBS Audio Callbacks
	static void SourceActivated(void *data, calldata_t *params);
	static void SourceDeactivated(void *data, calldata_t *params);

	//stream state infomation
	int mTotalSeconds = 0;
	int mDelaySecTotal = 0;
	int mDelaySecStarting = 0;
	int mDelaySecStopping = 0;
	int mBitrateUpdateSeconds = 0;
	uint64_t mLastBytesSent = 0;
	uint64_t mLastBytesSentTime = 0;
	bool mActive = false;
	QTimer *mRefreshTimer = NULL;
	void mActivateStatusInfo();
	void mDeactivateStatusInfo();
	void mUpdateDelayMsg();

	//Screen Shot
	bili_area_cap *mAreaCap;

	void onScreenShotStateChanged(int state);
	void onScreenShotComplete(bool hasSelect);

	void mInvokeProcdure(BiliThreadWorker::TaskT&& proc);
	void mPostTask(BiliThreadWorker::TaskT&& proc);

private:
    bool on_screen_shot_;

	public slots:
	//Run In UI Thread
	void mSltInvokeProcdure(QVariant proc);

	//Title Button
	void mSltZoomBtnClicked();
	void mSltMoreBtnClicked();
	void mSltChangeLang();
	//More Menu Action
	void mSltMoreMenuActClicked(const QString &actTxt);
	void mSltGotoMyRoomAct();
	void mSltLiveCommentingSetAct();
	void mSltUsingBootAct();
	void mSltHelpAct();
	void mSltUpdateCheckAct();
	void mSltLogoutAct();
	void mSltQuitAct();
	//Source Add 
	void mSltAddSourceButtonClicked();
	void mSltAddSourceActionTriggered(bool checked);

	//Setting, Start Broadcast
	void mSltValumeSettingBtn();
	void mSltSettingBtn();

	void mSltTogglePreview();		//OBS
	void mSltDeactivateAudioSource(OBSSource source);
	void mSltActivateAudioSource(OBSSource source);

	//stream state infomation
	void mSltStreamDelayStarting(int sec);
	void mSltStreamDelayStopping(int sec);
	void mSltStreamingStart();
	void mSltStreamingStop(int errorcode);

	public slots: //录制按钮所需的槽
	void mSltOnRecordClicked();
	void mSltOnRecordStartingClicked();
	void mSltOnRecordingClicked();
	void mSltOnRecordStoppingClicked();
	void mSltOnRecordFailClicked();

	void OnRecordTimerTick();

	void mSltRecordingStart();
	void mSltRecordingStop();
	void mSltRecordingStarting();
	void mSltRecordingStopping();

	void mRequestReconnect();

protected: //OutputSignalMonitor的虚函数
	bili::recursive_mutex outputSignalMonitorMutex;
	OutputSignalMonitor*  outputSignalMonitor;

	OutputSignalMonitor* RecordingStart() override;
	OutputSignalMonitor* RecordingStop() override;
	OutputSignalMonitor* RecordingStarting() override;
	OutputSignalMonitor* RecordingStopping() override;
	OutputSignalMonitor* StreamDelayStarting(int sec) override;
	OutputSignalMonitor* StreamDelayStopping(int sec) override;
	OutputSignalMonitor* StreamingStart() override;
	OutputSignalMonitor* StreamingStop(int errorcode) override;

	public slots:
	void onSettingUpdated();        //设置改变的时候更新界面，例如：用了自定义推流，获取不到直播间信息也能推
	//bool onBroadcastRoomRequested();   //需要直播间的地方调用这个，如果没直播间则提示用户并返回false
	//bool onBroadcastRoomRequested(QString noRoomMsg);
    /*
      code:
        0 —— 开播失败
        1 —— 尚未开通直播间
     */
	bool onBroadcastRoomRequested(int code);

public:								//Source Add
	void mOnSourceAdded_DoAdjust(obs_sceneitem_t* newSceneItem, bool isTimeOut);

public slots: //开始直播按钮所需的槽
	void OnRetryButtonClicked();
	void OnStartBroadcastButtonClicked();
	void OnStopBroadcastButtonClicked();
	void OnWorkingInProgressClicked();
	void StartBroadcast();

	void OnBroadcastTimerTick();

	void OnBroadcastReconnect();
	void OnBroadcastReconnected();

	//stream state infomation
	void mSltUpdateStreamStateInfo();

protected: //开始直播停止直播开始录制停止录制的快捷键
	void OnBroadcastHotkey(bool isPressed);
	void OnRecordHotkey(bool isPressed);

public slots: //弹幕姬控制的槽
	void mSltDanmakuSetting();
	void sltAutoStartDanmaku();

private: //弹幕来的回调
	void OnDanmakuAudienceCount(int audienceCount);
	void OnDanmakuMessage(const BiliJsonPtr& jsonPtr);

private slots: //弹幕服务器来的各种消息
	void* DanmakuShowCutoffMessageBoxTask();
	void DanmakuMessageBox(const QString title, const QString text);

private:
	//for output stream
	std::unique_ptr<BasicOutputHandler> outputHandler;

	std::auto_ptr<OBSSignal> broadcastReconnectSignal;
	static void OnBroadcastReconnectCallback(BiLiOBSMainWid* This, calldata_t* params);

	std::auto_ptr<OBSSignal> broadcastReconnectedSignal;
	static void OnBroadcastReconnectedCallback(BiLiOBSMainWid* This, calldata_t* params);

	//stream state infomation
	void requestRoomInfo();
	void *roomInfoRequestCb();
	private slots:
	void informInfoRequested();

private:
	void mUpdateBandwidth();
	void mUpdateDroppedFrames();
	void mUpdateSessionTime();
	void mUpdateAudienceCount();
	void mUpdateFansCount();

	struct RequestInfo {
		int fansNum;
		int audience;
		bool on_request;
	} request_info_;
	bool on_request_;

private: //for network api
	IBiliAPI* biliApi;
	BiliThreadWorker worker;
	std::unique_ptr<BiliDanmakuHime> danmakuHime;

	void mStartDanmakuHime();
	void mStopDanmakuHime();

private: //network request task
	void* mGetUserFacePixmapTask();
	static void* mGetUserFacePixmapTaskWrapper(wcs::WeakRef<BiLiOBSMainWid> own, void*(BiLiOBSMainWid::*task)(void));
	void* mUpdateRoomPresentCountTask();
	static void* mUpdateRoomPresentCountTaskWrapper(wcs::WeakRef<BiLiOBSMainWid> own, void*(BiLiOBSMainWid::*task)(void));
	void* mTurnOnRoomAndGetRTMPAddrTask();
	void* mStopDanmakuHimeTask(BiliCounterTrigger*);
	void* mStopStreamingTask(BiliCounterTrigger* counterTrigger);
	void* mTurnOffRoomTask(BiliCounterTrigger*);
	void* mRetriveUserInfo();
    void* mCheckNewVersion(SystemRetInfoDlg* msgDlg);

	void* mAskForOpenNewVersionPage(bool shouldAsk, QString url);

private:
	time_t mBroadcastStartTime;
	QTimer mBroadcastTickTimer;
	void mStartBroadcastTimer();
	void mStopBroadcastTimer();

	time_t mRecordStartTime;
	QTimer mRecordTickTimer;
	void mStartRecordTimer();
	void mStopRecordTimer();

public: //for preview
	int           mPreviewX = 0, mPreviewY = 0;
	float         mPreviewScale = 0.0f;
	gs_vertbuffer_t *mBox = nullptr;
	gs_vertbuffer_t *mCircle = nullptr;

	OBSBasicPreview* obsPreview;
	void InitPreview();
	int sourceWidth;
	int sourceHeight;
	OBSScene GetCurrentScene();
	static void RenderPreview(void* data, uint32_t cx, uint32_t cy);

	public slots:
	void ResetPreview();
	void sltResetPreviewWid();
	void sltResetSource();

	private slots: //preview context menu
	void OnPreviewContextMenu(QPoint mousePos);


private: //for save and load scene, audio device configuration
	void LoadScene();
	void SaveScene();

	void LoadAudioDeviceConfig();
	void SaveAudioDeviceConfig();

    void loadDanmakuHistoryConfig();
	void SaveFrontendHotkeys();
	void LoadFrontendHotkeys();

	void doOnBeginNetStateCatch();
	bool net_ok_by_frame_losed_;

	/*0 ~ 600*/
	int colock_count_;
	int frames_losed_at_begin_of_6_s_;
	bool on_reconnected_;
	private slots:
	void onNetDialog(const QString&);

private slots:
	void onTestDmSignal();

public:
	inline config_t* Config() { return mBasicConfig; }
	inline obs_service_t* GetService() { return mService; }

private:
	wcs::WeakRef<BiLiOBSMainWid> weak_ref_;




};

#endif // BILIOBSMAINWID_H
