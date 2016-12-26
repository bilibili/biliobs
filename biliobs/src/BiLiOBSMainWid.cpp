#include "BiLiOBSMainWid.h"
#include "BiliGlobalDatas.hpp"

#include "ShadowDlg.h"
#include "BiLiMsgDlg.h"
#include "BiLiPropertyDlg.h"
#include "BiliNameDialog.hpp"
#include "bili-icon-msgdlg.h"
#include "BiliOBSUtility.hpp"
#include "display-helpers.hpp"
#include "bili_obs_source_helper.h"
#include "BiliRecordButtonOperator.hpp"
#include "BiliBroadcastButtonOperator.hpp"
#include "HotkeyManager.h"

#include "bili_area_cap.h"
#include "net_status_wgt.h"
#include "BiLiUserInfoWid.h"
#include "AudioDeviceSettingWid.h"
#include "BiLiAudioDevSettingWid.h"

#include "VideoSettingWid.h"
#include "HotkeySettingWid.h"
#include "AdvancedSettingWid.h"
#include "PushStreamSettingWid.h"

#include "danmakuopt.h"
#include "../biliapi/IBiliApi.h"
#include "BiLiApp.h"

#include <QTranslator>
#include <QCloseEvent>
#include <QSignalMapper>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QGraphicsDropShadowEffect>

#include <QMenu>
#include <QTimer>
#include <QAction>
#include <QThread>
#include <QPainter>
#include <QPicture>
#include <QCheckBox>
#include <QListWidget>
#include <QDateTime>
#include <QBitmap>
#include <QShortcut>
#include <qapplication.h>
#include <QShowEvent>
#include <functional>

#include "oper_tip_dlg_factory.h"
#include "oper_tip_dlg.h"
#include "system_ret_info_dlg.h"

#include <qdatetime.h>
int get_pos_desktop_index(const QPoint &ps);


extern "C" {
#include "base64.h"
};

#ifdef _WIN32
#define IS_WIN32 1
#else
#define IS_WIN32 0
#endif

#define PREVIEW_EDGE_SIZE 0
#define MAIN_SEPARATOR \
	"====================================================================="
#define SERVICE_PATH "service.json"


extern const char* BILI_HOTKEY_BROADCAST_NAME;
extern const char* BILI_HOTKEY_RECORD_NAME;

//////////////////////////// OBSBasic Static Method ////////////////////////////
static inline enum video_format GetVideoFormatFromName(const char *name) {

	if (astrcmpi(name, "I420") == 0)
		return VIDEO_FORMAT_I420;
	else if (astrcmpi(name, "NV12") == 0)
		return VIDEO_FORMAT_NV12;
	else if (astrcmpi(name, "I444") == 0)
		return VIDEO_FORMAT_I444;
#if 0 //currently unsupported
	else if (astrcmpi(name, "YVYU") == 0)
		return VIDEO_FORMAT_YVYU;
	else if (astrcmpi(name, "YUY2") == 0)
		return VIDEO_FORMAT_YUY2;
	else if (astrcmpi(name, "UYVY") == 0)
		return VIDEO_FORMAT_UYVY;
#endif
	else
		return VIDEO_FORMAT_RGBA;
}

static inline enum obs_scale_type GetScaleType(ConfigFile &basicConfig) {

	const char *scaleTypeStr = config_get_string(basicConfig,
		"Video", "ScaleType");

	if (astrcmpi(scaleTypeStr, "bilinear") == 0)
		return OBS_SCALE_BILINEAR;
	else if (astrcmpi(scaleTypeStr, "lanczos") == 0)
		return OBS_SCALE_LANCZOS;
	else
		return OBS_SCALE_BICUBIC;
}

static inline int AttemptToResetVideo(struct obs_video_info *ovi) {

	return obs_reset_video(ovi);
}

static void AddExtraModulePaths() {

	char base_module_dir[512];
	int ret = GetUserDataPath(base_module_dir, sizeof(base_module_dir), "common/plugins/%module%");
	if (ret <= 0)
		return;

	string path = (char*)base_module_dir;
#if defined(__APPLE__)
	obs_add_module_path((path + "/bin").c_str(), (path + "/data").c_str());
#elif ARCH_BITS == 64
	obs_add_module_path((path + "/bin/64bit").c_str(),
		(path + "/data").c_str());
#else
	obs_add_module_path((path + "/bin/32bit").c_str(),
		(path + "/data").c_str());
#endif
}


////////////////////////////////////////////////////////////////////////////////

BiLiOBSMainWid::BiLiOBSMainWid(IBiliAPI* papi, QWidget *parent)
	: QWidget(parent)
	, mIsChineseLang(true)
	, isRightTabShow_(true)
	, biliApi(papi)
	, isRoomIdGot(false)
	, mAreaCap(NULL)
	, colock_count_(0)
	, dmOpt_(nullptr)
	, outputSignalMonitor(this),


	on_reconnected_(false),

	mIsPressed(false),

	on_screen_shot_(false)
{
	uiThread = pthread_self();

	mSetupUI();
	mSetupConnection();

	installEventFilter(this);
	ui.SceneOneListWid->installEventFilter(this);

	worker.Start();

	request_info_.on_request = false;

	ui.TitleUserFaceLab->hide();

	QHBoxLayout *usr_info_btn_layout = new QHBoxLayout(ui.UserInfoBtn);
	usr_info_btn_layout->setMargin(0);
	usr_info_btn_layout->setSpacing(0);
	usr_face_btn_spacer0_ = new QSpacerItem(0, 40, QSizePolicy::Fixed, QSizePolicy::Fixed);
	usr_face_btn_spacer1_ = new QSpacerItem(14, 40, QSizePolicy::Fixed, QSizePolicy::Fixed);
	usr_face_lbl_ = new QLabel();
	usr_face_lbl_->setFixedSize(40, 40);
	usr_face_lbl_->setStyleSheet("QLabel { image: url(:/FucBtn/UserFace); }");


	usr_name_lbl_ = new QLabel();
	usr_name_lbl_->setFixedHeight(40);
	usr_name_lbl_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	usr_name_lbl_->setAlignment(Qt::AlignCenter);
	usr_name_lbl_->setStyleSheet("color:#FFFFFF");
	QFont font;
	font.setBold(true);
	font.setFamily(QString("Microsoft YaHei"));
	font.setWeight(75);
	usr_name_lbl_->setFont(font);

	usr_info_btn_layout->addItem(usr_face_btn_spacer1_);
	usr_info_btn_layout->addWidget(usr_name_lbl_);
	usr_info_btn_layout->addItem(usr_face_btn_spacer0_);
	usr_info_btn_layout->addWidget(usr_face_lbl_);

	ui.UserInfoBtn->setFixedSize(40 + usr_face_btn_spacer0_->geometry().width(), 40);
}

BiLiOBSMainWid::~BiLiOBSMainWid() {
	weak_ref_.invalid();
}

void BiLiOBSMainWid::mInvokeProcdure(BiliThreadWorker::TaskT&& proc)
{
	QMetaObject::invokeMethod(this, "mSltInvokeProcdure", Qt::QueuedConnection, Q_ARG(QVariant, qVPtr<BiliThreadWorker::TaskT>::toVariant(new BiliThreadWorker::TaskT(proc))));
}

void BiLiOBSMainWid::mPostTask(BiliThreadWorker::TaskT&& proc)
{
	worker.AddTask(std::move(proc));
}

void BiLiOBSMainWid::mSltInvokeProcdure(QVariant proc)
{
	BiliThreadWorker::TaskT* pProc = qVPtr<BiliThreadWorker::TaskT>::toPtr(proc);
	(*pProc)();
	delete pProc;
}

void BiLiOBSMainWid::mOBSInit() {

	ProfileScope("BiLiOBSMainWid::mOBSInit");

	if (!weak_ref_.init(this)) {
		throw "Failed to init lock";
	}

	const char *sceneCollection = config_get_string(App()->mGetGlobalConfig(),
		"Basic", "SceneCollectionFile");
	if (!sceneCollection)
		throw "Failed to get scene collection name";

	qRegisterMetaType<OBSScene>("OBSScene");
	qRegisterMetaType<OBSSceneItem>("OBSSceneItem");
	qRegisterMetaType<OBSSource>("OBSSource");

	qRegisterMetaTypeStreamOperators<
		std::vector<std::shared_ptr<OBSSignal>>>(
		"std::vector<std::shared_ptr<OBSSignal>>");
	qRegisterMetaTypeStreamOperators<OBSScene>("OBSScene");
	qRegisterMetaTypeStreamOperators<OBSSceneItem>("OBSSceneItem");

	int ret;
	char fileName[512];
	char savePath[512];

	ret = snprintf(fileName, 512, QString("%1\\%2").arg(QString::fromStdString(gBili_mid)).arg("basic/scenes/%s.json").toUtf8().data(), sceneCollection);
	if (ret <= 0)
		throw "Failed to create scene collection file name";

	ret = GetUserDataPath(savePath, sizeof(savePath), fileName);
	if (ret <= 0)
		throw "Failed to get scene collection json file path";

	if (!mInitBasicConfig())
		throw "Failed to load basic.ini";

	if (!mResetAudio())
		throw "Failed to initialize audio";

	ret = mResetVideo();
	switch (ret) {
	case OBS_VIDEO_MODULE_NOT_FOUND:
		throw "Failed to initialize video:  Graphics module not found";
	case OBS_VIDEO_NOT_SUPPORTED:
		throw "Failed to initialize video:  Required graphics API "
			"functionality not found on these drivers or "
			"unavailable on this equipment";
	case OBS_VIDEO_INVALID_PARAM:
		throw "Failed to initialize video:  Invalid parameters";
	default:
		if (ret != OBS_VIDEO_SUCCESS)
			throw "Failed to initialize video:  Unspecified error";
	}

	mInitOBSAudioCallbacks();
	AddExtraModulePaths();
	obs_load_all_modules();
	blog(LOG_INFO, MAIN_SEPARATOR);
	if (!mInitService())
		throw "Failed to initialize service";
	mInitPrimitives();
	{
		ProfileScope("BiLiOBSMainWid::mLoad");
		mDisableSaving--;
		mLoad(savePath);
		loadImgAtNoItems();
		mDisableSaving++;
	}

	InitPreview();
	sltResetPreviewWid();
	ResetPreview();

	//TimedCheckForUpdates();
	mLoaded = true;

	bool previewEnabled = config_get_bool(App()->mGetGlobalConfig(),
		"BasicWindow", "PreviewEnabled");
	if (!previewEnabled)
		QMetaObject::invokeMethod(this, "mSltTogglePreview",
		Qt::QueuedConnection);

#ifdef _WIN32
	uint32_t winVer = GetWindowsVersion();
	if (winVer > 0 && winVer < 0x602) {
		bool disableAero = config_get_bool(mBasicConfig, "Video",
			"DisableAero");
		SetAeroEnabled(!disableAero);
	}
#endif

	mDisableSaving--;

#if 0
	auto addDisplay = [this] (OBSQTDisplay *window)
	{
		obs_display_add_draw_callback(window->GetDisplay(),
			OBSBasic::RenderMain, this);

		struct obs_video_info ovi;
		if (obs_get_video_info(&ovi))
			mResizePreview(ovi.base_width, ovi.base_height);
	};

	connect(ui->preview, &OBSQTDisplay::DisplayCreated, addDisplay);
#endif
	//创建推流服务
	mService = obs_service_create("rtmp_custom", "default_service", NULL, NULL);
	//已经被智能指针接管，所以要释放一下
	obs_service_release(mService);

	//创建推流对象
	this->outputHandler.reset();
	this->outputHandler.reset(
		CreateSimpleOutputHandler(this)
		);

	mSleepInhibitor = os_inhibit_sleep_create("OBS Video/audio");
	os_inhibit_sleep_set_active(mSleepInhibitor, true);
	//show();

	//获取房间信息、推流地址
	emit OnRetryButtonClicked();

	//检查更新
	worker.AddTask(std::bind(&BiLiOBSMainWid::mCheckNewVersion, this, nullptr));

	//获取头像
	worker.AddTask(std::bind(&BiLiOBSMainWid::mGetUserFacePixmapTaskWrapper, weak_ref_, &BiLiOBSMainWid::mGetUserFacePixmapTask));

	//获取房间礼物数
	worker.AddTask(std::bind(&BiLiOBSMainWid::mUpdateRoomPresentCountTaskWrapper, weak_ref_, &BiLiOBSMainWid::mUpdateRoomPresentCountTask));

	loadDanmakuHistoryConfig();
}

void BiLiOBSMainWid::mInitOBSAudioCallbacks(){

	signal_handler_connect(obs_get_signal_handler(), "source_activate", BiLiOBSMainWid::SourceActivated, this);
	signal_handler_connect(obs_get_signal_handler(), "source_deactivate", BiLiOBSMainWid::SourceDeactivated, this);

}

void BiLiOBSMainWid::SourceActivated(void *data, calldata_t *params) {

	obs_source_t *source = (obs_source_t*)calldata_ptr(params, "source");
	uint32_t     flags = obs_source_get_output_flags(source);

#if 0
	if (flags & OBS_SOURCE_AUDIO)
		QMetaObject::invokeMethod(static_cast<BiLiOBSMainWid *>(data),
		"mSltActivateAudioSource",
		Q_ARG(OBSSource, OBSSource(source)));

#else

	const char *id = obs_source_get_id(source);
	int sceneC = QString::compare(id, "scene", Qt::CaseInsensitive);
	int inCapC = QString::compare(id, "wasapi_output_capture", Qt::CaseInsensitive);
	int outCapC = QString::compare(id, "wasapi_input_capture", Qt::CaseInsensitive);
	if (!sceneC || !inCapC || !outCapC){
		if (flags & OBS_SOURCE_AUDIO)
			QMetaObject::invokeMethod(static_cast<BiLiOBSMainWid *>(data),
			"mSltActivateAudioSource",
			Q_ARG(OBSSource, OBSSource(source)));
	}
#if 0

	else {
		if (flags & OBS_SOURCE_AUDIO) {

			QThread *thread = new QThread();
			QObject::connect(thread, &QThread::started, [thread, source, data](){
				QSharedPointer<QMetaObject::Connection> con(new QMetaObject::Connection());
				QTimer *t = new QTimer();
				*con.data() = QObject::connect(t, &QTimer::timeout, [thread, source, data, con](){

					QMetaObject::invokeMethod(static_cast<BiLiOBSMainWid *>(data),
						"mSltActivateAudioSource",
						Q_ARG(OBSSource, OBSSource(source)));

					QObject::disconnect(*con.data());
					thread->quit();
				} );
				t->start(1000);

				QTimer::singleShot(1000, [thread, source, data](){

					QMetaObject::invokeMethod(static_cast<BiLiOBSMainWid *>(data),
						"mSltActivateAudioSource",
						Q_ARG(OBSSource, OBSSource(source)));

					thread->quit();
				});
			});
			thread->start();
		}
	}

#endif
#endif

}

void BiLiOBSMainWid::SourceDeactivated(void *data, calldata_t *params) {

	obs_source_t *source = (obs_source_t*)calldata_ptr(params, "source");
	uint32_t     flags = obs_source_get_output_flags(source);

	if (flags & OBS_SOURCE_AUDIO)
		QMetaObject::invokeMethod(static_cast<BiLiOBSMainWid *>(data),
		"mSltDeactivateAudioSource",
		Q_ARG(OBSSource, OBSSource(source)));
}

void BiLiOBSMainWid::mSetupUI(){

	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);

	mTrans = new QTranslator();
	mTrans->load(":/Trans/BiLi_zh");
	qApp->installTranslator(mTrans);

	ui.setupUi(this);

	ui.SrcWinBtn->setHidden(true);
	ui.SrcWinLab->setHidden(true);
	ui.SrcMonitorBtn->setHidden(true);
	ui.SrcMonitorLab->setHidden(true);
	ui.DelayInfoLab->setHidden(true);
	ui.DropFramesLab->setHidden(true);
	ui.KBPSLab->setHidden(true);

	//切换场景的按钮
	QPushButton* sceneListButton = new QPushButton();
	sceneListButton->setObjectName("SceneListBtn");
	sceneListButton->setFixedSize(30, 30);
	ui.RightSceneTabWid->tabBar()->setTabButton(0, QTabBar::ButtonPosition::RightSide, sceneListButton);

	//添加来源的按钮
	mFuncBtnList << ui.SrcCamBtn << ui.SrcGameBtn << ui.SrcMonitorBtn
		<< ui.SrcWinBtn << ui.SrcVideoBtn << ui.SrcTxtBtn << ui.SrcPicBtn;

	ui.SrcCamBtn->setProperty("source_id", "dshow_input");
	ui.SrcGameBtn->setProperty("source_id", "game_capture");
	ui.SrcMonitorBtn->setProperty("source_id", "monitor_capture");
	ui.SrcWinBtn->setProperty("source_id", "window_capture");
	ui.SrcVideoBtn->setProperty("source_id", "ffmpeg_source");
	ui.SrcTxtBtn->setProperty("source_id", "text_ft2_source");
	ui.SrcPicBtn->setProperty("source_id", "image_source");

	mCreateRightMenu();

	sceneListWidgetOperator.reset(
		new BiliSceneListWidgetOperator(ui.RightSceneTabWid, ui.SceneOneListWid, ui.SceneOneToolBar, ui.history_wgt)
		);
	broadcastButtonOperator.reset(
		new BiliBroadcastButtonOperator(ui.StartBroadcastBtn)
		);
	recordButtonOperator.reset(
		new BiliRecordButtonOperator(ui.StartRecordBtn)
		);

	auto ShortCut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
	QObject::connect(ShortCut, &QShortcut::activated, [this](){
		sceneListWidgetOperator->sceneEditMenuRemove();
	});

	HotkeyManager::GetInstance()->Register(BILI_HOTKEY_BROADCAST_NAME, CreateFrontendHotkeyCallback(this, &BiLiOBSMainWid::OnBroadcastHotkey));
	HotkeyManager::GetInstance()->Register(BILI_HOTKEY_RECORD_NAME, CreateFrontendHotkeyCallback(this, &BiLiOBSMainWid::OnRecordHotkey));

	mAudioDevSettingWid = new BiLiAudioDevSettingWid();
	mBiLiUserInfoWid = new BiLiUserInfoWid();
	mBiLiUserInfoWid->setWindowFlags(mBiLiUserInfoWid->windowFlags() | Qt::Popup);

	QObject::connect(mBiLiUserInfoWid, SIGNAL(OnUserNameOrIconClickedSignal()), this, SLOT(OnOpenRoomClicked()));
	QObject::connect(mBiLiUserInfoWid, SIGNAL(onRoomManageTriggered()), this, SLOT(OnRoomMangerClicked()));
	QObject::connect(mBiLiUserInfoWid, SIGNAL(onInfoModifyTriggered()), this, SLOT(OnInfomodifyClicked()));


	//Monitor And Window Capture
	monitorAndWinMenu_ = new QMenu(this);
	if (monitorAndWinMenu_->objectName().isEmpty())
		monitorAndWinMenu_->setObjectName("MonitorAndWinMenu");
	QAction *monitorA = new QAction(tr("Monitor"), this);
	QAction *windowA = new QAction(tr("Window"), this);
	monitorAndWinMenu_->addAction(monitorA);
	monitorAndWinMenu_->addAction(windowA);
	QObject::connect(monitorA, &QAction::triggered, std::bind(&QPushButton::clicked, ui.SrcMonitorBtn, std::placeholders::_1));
	QObject::connect(windowA, &QAction::triggered, std::bind(&QPushButton::clicked, ui.SrcWinBtn, std::placeholders::_1));


	//Danmaku 
	dmOpt_ = new DanmakuOpt;
	dmOpt_->getDMInfoForStart_ = std::bind(&BiLiOBSMainWid::setDMInfoForStart_, this, dmOpt_);
#if WebSwitchRoom	
	QObject::connect(dmOpt_, SIGNAL(sglSwitchRoom(int)), this, SLOT(sltSwitchRoom_(int)), Qt::QueuedConnection);
#endif
	QObject::connect(dmOpt_, &DanmakuOpt::sglSettingsDM, this, &BiLiOBSMainWid::mSltDanmakuSetting);

	//标题
	setWindowTitle(tr("Bilibili Broadcaster"));
}

#if WebSwitchRoom	
void BiLiOBSMainWid::sltSwitchRoom_(int roomId) {

	dmOpt_->setDMOnOff_(false);
	QTimer::singleShot(0, [this, roomId](){
		int globalRoomIdBak = gBili_roomId;
		gBili_roomId = roomId;
		dmOpt_->dmRoomID_ = roomId;
		if (!dmOpt_->setDMOnOff_(true)){
			SystemRetInfoDlg dlg;
			dlg.setTitle("");
			dlg.setSubTitle(tr("Warning"));
			dlg.setDetailInfo(QString("Can not switch to room: %1 !").arg(roomId));
			dlg.resize(dlg.sizeHint());
			move_widget_to_center(&dlg, this);
			dlg.exec();
		}
		gBili_roomId = globalRoomIdBak;
	});
}
#endif

void BiLiOBSMainWid::mSetupConnection(){

	QObject::connect(ui.MoreBtn, SIGNAL(clicked()), SLOT(mSltMoreBtnClicked()));
	QObject::connect(ui.CloseBtn, SIGNAL(clicked()), this, SLOT(close()));
	QObject::connect(ui.MiniBtn, &QPushButton::clicked, this, &BiLiOBSMainWid::OnMinimizeButtonClicked);
	QObject::connect(ui.SrcWinAndMonitorBtn, &QPushButton::clicked, this, &BiLiOBSMainWid::OnWinAndMonitorSourceButtonClicked);

	ui.DanmakuBtn->setHidden(true);
	ui.DanmakuLab->setHidden(true);

	//主界面右边栏开关
	connect(ui.ZoomBtn, SIGNAL(clicked()), SLOT(mSltZoomBtnClicked()));
	ui.ZoomBtn->setMask(QPixmap(QString::fromUtf8(":/FucBtn/ZoomMask")).createHeuristicMask());

	//添加来源的按钮
	for (int i = 0; i < mFuncBtnList.size(); i++){
		connect(mFuncBtnList.at(i), SIGNAL(clicked()), this, SLOT(mSltAddSourceButtonClicked()));
	}

	connect(ui.ValumeSettingBtn, SIGNAL(clicked()), this, SLOT(mSltValumeSettingBtn()));

	//操作器的信号
	QObject::connect(broadcastButtonOperator.get(), SIGNAL(StartBroadcastButtonClickedSignal()), this, SLOT(OnStartBroadcastButtonClicked()));
	QObject::connect(broadcastButtonOperator.get(), SIGNAL(StopBroadcastButtonClickedSignal()), this, SLOT(OnStopBroadcastButtonClicked()));
	QObject::connect(broadcastButtonOperator.get(), SIGNAL(ReconnectingClickedSignal()), this, SLOT(OnStopBroadcastButtonClicked()));
	QObject::connect(broadcastButtonOperator.get(), SIGNAL(WorkingInProgressButtonClicked()), this, SLOT(OnWorkingInProgressClicked()));
	QObject::connect(broadcastButtonOperator.get(), SIGNAL(RetryButtonClickedSignal()), this, SLOT(OnRetryButtonClicked()));
	QObject::connect(broadcastButtonOperator.get(), SIGNAL(ConnectingClickedSignal()), this, SLOT(OnWorkingInProgressClicked()));
	QObject::connect(broadcastButtonOperator.get(), SIGNAL(DisconnectingClickedSignal()), this, SLOT(OnWorkingInProgressClicked()));

	QObject::connect(recordButtonOperator.get(), SIGNAL(IdleClickedSignal()), this, SLOT(mSltOnRecordClicked()));
	QObject::connect(recordButtonOperator.get(), SIGNAL(StartingClickedSignal()), this, SLOT(mSltOnRecordStartingClicked()));
	QObject::connect(recordButtonOperator.get(), SIGNAL(RecordingClickedSignal()), this, SLOT(mSltOnRecordingClicked()));
	QObject::connect(recordButtonOperator.get(), SIGNAL(StoppingClickedSignal()), this, SLOT(mSltOnRecordStoppingClicked()));
	QObject::connect(recordButtonOperator.get(), SIGNAL(FailedClickedSignal()), this, SLOT(mSltOnRecordFailClicked()));

	//设置按钮
	QObject::connect(ui.SettingBtn, SIGNAL(clicked()), this, SLOT(mSltSettingBtn()));

	//计时
	QObject::connect(&mBroadcastTickTimer, SIGNAL(timeout()), this, SLOT(OnBroadcastTimerTick()));
	QObject::connect(&mRecordTickTimer, SIGNAL(timeout()), this, SLOT(OnRecordTimerTick()));

	//弹幕历史
	connect(dmOpt_, &DanmakuOpt::netData, this, &BiLiOBSMainWid::onNetDialog);

	//如果隐藏，会造成下面的按钮上移，最终导致点了按钮出了时间按钮位置变了
	ui.TimeLab->setText(" ");
	ui.RecTimeLab->setText(" ");

}

void BiLiOBSMainWid::closeEvent(QCloseEvent *e) {
	weak_ref_.invalid();

	OperTipDlg *dlg = OperTipDlgFactory::makeDlg(OperTipDlgFactory::APP_EXIT);

	move_widget_to_center(dlg, this);
	dlg->exec();
	if (!dlg->result()) {
		e->ignore();
		delete dlg;
		return;
	}
	delete dlg;

	if (dmOpt_){
		dmOpt_->setCfgToDMWid_(static_cast<config_t*>(mBasicConfig));
		dmOpt_->setDMOnOff_(false);
		delete dmOpt_;
		dmOpt_ = 0;
	}

	mSave(0);

	OnStopBroadcastButtonClicked();

	worker.Stop();

	obsPreview->close();
	delete obsPreview;
	obsPreview = 0;

	delete mAudioDevSettingWid;
	mAudioDevSettingWid = 0;

	delete mBiLiUserInfoWid;
	mBiLiUserInfoWid = 0;

	HotkeyManager::UninitializeInstance();

	e->accept();
}

void BiLiOBSMainWid::mShow(){

	show();
}

bool BiLiOBSMainWid::eventFilter(QObject *obj, QEvent *e) {

	if (obj == this){
		if (e->type() == QEvent::KeyPress){
			QKeyEvent *ke = static_cast<QKeyEvent *>(e);
			vec2 moveOffset;
			bool isKeyMove = false;
			switch (ke->key()){
			case Qt::Key_Up:{
				vec2_set(&moveOffset, 0, -1);
				isKeyMove = true;
				break;
			}
			case Qt::Key_Down:{
				isKeyMove = true;
				vec2_set(&moveOffset, 0, 1);
				break;
			}
			case Qt::Key_Right:{
				isKeyMove = true;
				vec2_set(&moveOffset, 1, 0);
				break;
			}
			case Qt::Key_Left:{
				isKeyMove = true;
				vec2_set(&moveOffset, -1, 0);
				break;
			}
			default:{
				break;
			}
			}
			if (isKeyMove){
				if (obsPreview){
					obsPreview->moveItemByKeyboard_(moveOffset);
					e->accept();
				}
			}
		}
	}
	if (ui.SceneOneListWid == qobject_cast<QListWidget *>(obj)) {
		if (e->type() == QEvent::KeyPress){
			e->ignore();
			return true;
		}
	}
	return QWidget::eventFilter(obj, e);
}

void BiLiOBSMainWid::mousePressEvent(QMouseEvent *e) {

	if (e->button() == Qt::LeftButton)
		mIsPressed = true;

	mPoint = pos() - e->globalPos();
	QWidget::mousePressEvent(e);
}

void BiLiOBSMainWid::mouseMoveEvent(QMouseEvent *e) {

	if (mIsPressed)
		move(e->globalPos() + mPoint);
	QWidget::mouseMoveEvent(e);
}

void BiLiOBSMainWid::mouseReleaseEvent(QMouseEvent *e) {

	if (e->button() == Qt::LeftButton)
		mIsPressed = false;


	QWidget::mouseReleaseEvent(e);
}

void BiLiOBSMainWid::OnDanmakuButtonClicked() {
}

void BiLiOBSMainWid::OnWinAndMonitorSourceButtonClicked()
{
	QPoint ps;
	if (monitorAndWinMenu_->sizeHint() != monitorAndWinMenu_->size())
		monitorAndWinMenu_->resize(monitorAndWinMenu_->sizeHint());

	ps.setY(ui.LeftCenterWid->geometry().bottom() - monitorAndWinMenu_->size().height());
	ps.setX((ui.SrcWinAndMonitorBtn->geometry().left() + ui.SrcWinAndMonitorBtn->geometry().right()) / 2
		- monitorAndWinMenu_->size().width() / 2);

	ps = mapToGlobal(ps);

	monitorAndWinMenu_->exec(ps);
}

void BiLiOBSMainWid::OnMinimizeButtonClicked() {
	this->showMinimized();
}

OBSScene BiLiOBSMainWid::mGetCurrentScene() {

	OBSScene r;
	obs_source_t* src = obs_get_output_source(0);
	if (!src)
		return 0;

	obs_scene_t* sce = obs_scene_from_source(src);
	r = sce;
	obs_source_release(src);
	return r;
}

void BiLiOBSMainWid::mSltZoomBtnClicked() {

	if (isRightTabShow_) {
		isRightTabShow_ = false;
		ui.RightSceneTabWid->setHidden(true);
	}
	else {
		isRightTabShow_ = true;
		ui.RightSceneTabWid->setHidden(false);
	}
	sltResetPreviewWid();
}

void BiLiOBSMainWid::mSltChangeLang() {
#if 0
	if (mIsChineseLang){
		qApp->removeTranslator(mTrans);  
		ui.retranslateUi(this);
		mIsChineseLang = false;
	}else{
		mTrans->load(":/Trans/BiLi_zh");
		qApp->installTranslator(mTrans);
		ui.retranslateUi(this);
		mIsChineseLang = true;
	}
#endif
}

void BiLiOBSMainWid::mSltSettingBtn() {

	bool isBroadcastButtonIdle = broadcastButtonOperator->GetStatus() == BiliBroadcastButtonOperator::NORMAL_IDLE;
	bool isOutputActived = static_cast<bool>(outputHandler) == true &&
		(outputHandler->StreamingActive() || outputHandler->RecordingActive());

	bool isDisableEncoderAndPushConfig = (!isBroadcastButtonIdle) && isOutputActived;

	auto advSetWid = new AdvancedSettingWid();

	auto VideoSetWid = new VideoSettingWid(mBasicConfig);
	VideoSetWid->setObjectName("VideoSetWid");
	advSetWid->mAddStackedPageWid(0, VideoSetWid);

	auto AudioSetWid = new AudioDeviceSettingWid();
	AudioSetWid->setObjectName("AudioSetWid");
	advSetWid->mAddStackedPageWid(1, AudioSetWid);

	auto PushStreamWid = new PushStreamSettingWid(mBasicConfig, isDisableEncoderAndPushConfig);
	PushStreamWid->setObjectName("PushStreamWid");
	advSetWid->mAddStackedPageWid(2, PushStreamWid);

	auto HotkeyWid = new HotkeySettingWid(mBasicConfig);
	HotkeyWid->setObjectName("HotkeyWid");
	advSetWid->mAddStackedPageWid(3, HotkeyWid);

	advSetWid->setWindowFlags(Qt::FramelessWindowHint);
	move_widget_to_center(advSetWid, this);
	advSetWid->mShow();

	if (advSetWid->result() == QDialog::Accepted){
		SaveAudioDeviceConfig();
		onSettingUpdated();
	}

	delete advSetWid;
}

//////////////////////////// OBSBasic Method And Var////////////////////////////

void BiLiOBSMainWid::mResizePreview(uint32_t cx, uint32_t cy) {

	QSize  targetSize;

	/* resize preview panel to fix to the top section of the window */
	targetSize = GetPixelSize(ui.LeftCenterWid);
	//targetSize = GetPixelSize(ui->preview);
	GetScaleAndCenterPos(int(cx), int(cy),
		targetSize.width() - PREVIEW_EDGE_SIZE * 2,
		targetSize.height() - PREVIEW_EDGE_SIZE * 2,
		mPreviewX, mPreviewY, mPreviewScale);

	mPreviewX += float(PREVIEW_EDGE_SIZE);
	mPreviewY += float(PREVIEW_EDGE_SIZE);
}

int  BiLiOBSMainWid::mResetVideo(){

	ProfileScope("BiLiOBSMainWid::mResetVideo");

	struct obs_video_info ovi;
	int ret;

	mGetConfigFPS(ovi.fps_num, ovi.fps_den);

	const char *colorFormat = config_get_string(mBasicConfig, "Video",
		"ColorFormat");
	const char *colorSpace = config_get_string(mBasicConfig, "Video",
		"ColorSpace");
	const char *colorRange = config_get_string(mBasicConfig, "Video",
		"ColorRange");

	ovi.graphics_module = App()->mGetRenderModule();
	ovi.base_width = (uint32_t)config_get_uint(mBasicConfig,
		"Video", "BaseCX");
	ovi.base_height = (uint32_t)config_get_uint(mBasicConfig,
		"Video", "BaseCY");
	ovi.output_width = (uint32_t)config_get_uint(mBasicConfig,
		"Video", "OutputCX");
	ovi.output_height = (uint32_t)config_get_uint(mBasicConfig,
		"Video", "OutputCY");
	ovi.output_format = GetVideoFormatFromName(colorFormat);
	ovi.colorspace = astrcmpi(colorSpace, "601") == 0 ?
	VIDEO_CS_601 : VIDEO_CS_709;
	ovi.range = astrcmpi(colorRange, "Full") == 0 ?
	VIDEO_RANGE_FULL : VIDEO_RANGE_PARTIAL;
	ovi.adapter = 0;
	ovi.gpu_conversion = true;
	ovi.scale_type = GetScaleType(mBasicConfig);

	ret = AttemptToResetVideo(&ovi);
	if (IS_WIN32 && ret != OBS_VIDEO_SUCCESS) {
		/* Try OpenGL if DirectX fails on windows */
		if (astrcmpi(ovi.graphics_module, DL_OPENGL) != 0) {
			blog(LOG_WARNING, "Failed to initialize obs video (%d) "
				"with graphics_module='%s', retrying "
				"with graphics_module='%s'",
				ret, ovi.graphics_module,
				DL_OPENGL);
			ovi.graphics_module = DL_OPENGL;
			ret = AttemptToResetVideo(&ovi);
		}
	}
	else if (ret == OBS_VIDEO_SUCCESS) {
		mResizePreview(ovi.base_width, ovi.base_height);
	}

	return ret;
}

bool BiLiOBSMainWid::mResetAudio(){

	ProfileScope("BiLiOBSMainWid::mResetAudio");

	struct obs_audio_info ai;
	ai.samples_per_sec = config_get_uint(mBasicConfig, "Audio",
		"SampleRate");

	const char *channelSetupStr = config_get_string(mBasicConfig,
		"Audio", "ChannelSetup");

	if (strcmp(channelSetupStr, "Mono") == 0)
		ai.speakers = SPEAKERS_MONO;
	else
		ai.speakers = SPEAKERS_STEREO;

	ai.buffer_ms = config_get_uint(mBasicConfig, "Audio", "BufferingTime");

	return obs_reset_audio(&ai);
}

bool BiLiOBSMainWid::mLoadService() {

#if 0
	const char *type;

	char serviceJsonPath[512];
	int ret = mGetProfilePath(serviceJsonPath, sizeof(serviceJsonPath),
		SERVICE_PATH);
	if (ret <= 0)
		return false;

	obs_data_t *data = obs_data_create_from_json_file_safe(serviceJsonPath,
		"bak");

	obs_data_set_default_string(data, "type", "rtmp_common");
	type = obs_data_get_string(data, "type");

	obs_data_t *settings = obs_data_get_obj(data, "settings");
	obs_data_t *hotkey_data = obs_data_get_obj(data, "hotkeys");

	mService = obs_service_create(type, "default_service", settings,
		hotkey_data);
	obs_service_release(mService);

	obs_data_release(hotkey_data);
	obs_data_release(settings);
	obs_data_release(data);

	return !!mService;
#else
	return true;
#endif
}

bool BiLiOBSMainWid::mInitService() {

	ProfileScope("BiLiOBSMainWid::mInitService");

	//if (mLoadService())
	//	return true;

	//mService = obs_service_create("rtmp_common", "default_service", nullptr, nullptr);
	//if (!mService)
	//	return false;
	//obs_service_release(mService);

	return true;
}

void BiLiOBSMainWid::mClearSceneData() {

	mDisableSaving++;

	obs_set_output_source(0, nullptr);
	obs_set_output_source(1, nullptr);
	obs_set_output_source(2, nullptr);
	obs_set_output_source(3, nullptr);
	obs_set_output_source(4, nullptr);
	obs_set_output_source(5, nullptr);

	for (OBSSource& src : OBSEnumSources())
		obs_source_remove(src);

	mSourceSceneRefs.clear();
	mDisableSaving--;

	blog(LOG_INFO, "All scene data cleared");
	blog(LOG_INFO, "------------------------------------------------");
}

void BiLiOBSMainWid::mCreateDefaultScene(bool firstStart) {

	mDisableSaving++;

	mClearSceneData();

	obs_scene_t  *scene = obs_scene_create(Str("Basic.Scene"));
	obs_source_t *source = obs_scene_get_source(scene);

	obs_add_source(source);

	if (firstStart)
		BiliAudioDeviceConfig::mCreateFirstRunSources();

	obs_set_output_source(0, obs_scene_get_source(scene));
	obs_scene_release(scene);

	mDisableSaving--;
}

void BiLiOBSMainWid::mSltTogglePreview() {

#if 0
	bool enabled = !obs_display_enabled(ui->preview->GetDisplay());
	obs_display_set_enabled(ui->preview->GetDisplay(), enabled);
	ui->preview->setVisible(enabled);
	ui->previewDisabledLabel->setVisible(!enabled);
#endif
}

OBSScene BiLiOBSMainWid::GetCurrentScene()
{
	obs_scene_t* outputScene = 0;
	obs_source_t* outputSource = obs_get_output_source(0);
	if (outputSource)
	{
		outputScene = obs_scene_from_source(outputSource);
		obs_source_release(outputSource);
		if (outputScene)
		{
			return outputScene;
		}
	}

	return OBSScene();
}

void BiLiOBSMainWid::OnErrorMessage(QString msg)
{
	SystemRetInfoDlg dlg;
	dlg.setTitle("");
	dlg.setSubTitle(tr("Tip"));
	dlg.setDetailInfo(msg);
	dlg.resize(dlg.sizeHint());
	move_widget_to_center(&dlg, this);
	dlg.exec();
}


void BiLiOBSMainWid::OnUserInfoGot(QString userName, QPixmap userFace)
{
	QFont font;
	font.setBold(true);
	font.setFamily(QString("Microsoft YaHei"));
	font.setWeight(10);
	int w = QFontMetrics(font).size(Qt::TextSingleLine, userName).width();

	usr_name_lbl_->setText(userName);
	ui.UserInfoBtn->setFixedSize(usr_name_lbl_->sizeHint().width() + 3 + 40 + usr_face_btn_spacer0_->geometry().width() + usr_face_btn_spacer1_->geometry().width(), 40);


	mUserName = userName;
	mUserFace = userFace.scaled(50, 50);
	mBiLiUserInfoWid->mSetUserFace(userFace);

	int width = ui.TitleUserFaceLab->width();
	int height = ui.TitleUserFaceLab->height();

	QPainterPath path;
	path.addEllipse(0, 0, width, height);

	QPixmap backPixmap(width, height);
	backPixmap.fill(QColor(79, 193, 233, 0));
	QPainter p(&backPixmap);
	p.setClipPath(path);
	p.drawPixmap(0, 0, width, height, userFace);
	ui.TitleUserFaceLab->setStyleSheet("QLabel { image: url(:/FucBtn/UserFace); }");
}

void BiLiOBSMainWid::onNetDialog(const QString &msg) {
	ui.history_wgt->insertData(msg);
}

bool BiLiOBSMainWid::onBroadcastRoomRequested(int code)
{
	if (isRoomIdGot == false || gBili_roomId == -1) {
		int dlg_ret;
		if (0 == code) {
			OperTipDlg *msgDlg = OperTipDlgFactory::makeDlg(OperTipDlgFactory::START_BROAD_FAIL);
			msgDlg->exec();

			dlg_ret = msgDlg->result();
			delete msgDlg;

		}
		else if (1 == code) {

			QDate cur_date = QDate::currentDate();
			QByteArray tmp = cur_date.toString("yyyyMMdd").toLocal8Bit();
			const char *cur_date_p = tmp;


			char const *date;
			if (date = config_get_string(mBasicConfig, "TipControl", "NoOpenLiveTodayNotRemind")) {
				if (!strcmp(cur_date_p, date)) {
					/*not remind*/
					return false;
				}
			}

			OperTipDlg *msgDlg = OperTipDlgFactory::makeDlg(OperTipDlgFactory::NO_OPEN_LIVE);
			msgDlg->exec();

			if (msgDlg->notTipChecked())
				config_set_string(mBasicConfig, "TipControl", "NoOpenLiveTodayNotRemind", cur_date_p);
			else
				config_set_string(mBasicConfig, "TipControl", "NoOpenLiveTodayNotRemind", "");

			dlg_ret = msgDlg->result();

			delete msgDlg;
		}


		if (dlg_ret == QDialog::Accepted)
		{
			//打开申请直播间页面
			std::string url = biliApi->GetRoomAdminUrl("open");
			QDesktopServices::openUrl(QUrl(url.c_str()));
		}

		return false;
	}
	else
		return true;
}