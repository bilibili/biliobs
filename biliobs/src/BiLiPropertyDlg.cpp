#include "BiLiPropertyDlg.h"
#include <QMouseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QVariant>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPlainTextEdit>
#include "BiLiApp.h"
#include "BiLiMsgDlg.h"
#include "BiLiOBSMainWid.h"
#include "BiliOBSUtility.hpp"
#include "PropertyDlgVolSliderWid.h"

#include "circle_slider_slider.h"

#include "oper_tip_dlg.h"
#include "oper_tip_dlg_factory.h"

char const*const vol_slider_name = "PropertyVOlSlider";

BiliPropChangeEventFilter::BiliPropChangeEventFilter()
	: isSignalTriggered(false)
{
	QObject::connect(&applyChangeTimer, SIGNAL(timeout()), this, SLOT(OnChangedSlot()));
}

bool BiliPropChangeEventFilter::IsChangedTriggered() const
{
	return isSignalTriggered;
}

void BiliPropChangeEventFilter::Watch(const std::initializer_list<QWidget*>& beWatchedWidgets)
{
	for (QWidget* beWatched : beWatchedWidgets)
	{
		beWatched->installEventFilter(this);

		if (qobject_cast<QCheckBox*>(beWatched))
		{
			QObject::connect(beWatched, SIGNAL(toggled(bool)), this, SLOT(OnToggled(bool)));
		}
		else if (qobject_cast<QComboBox*>(beWatched))
		{
			QObject::connect(beWatched, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxIndexChanged(int)));
			QObject::connect(beWatched, SIGNAL(editTextChanged(const QString&)), this, SLOT(OnTextChanged(const QString&)));
		}
		else if (qobject_cast<QLineEdit*>(beWatched))
		{
			QObject::connect(beWatched, SIGNAL(textChanged(const QString&)), this, SLOT(OnTextChanged(const QString&)));
		}
		else if (qobject_cast<QPlainTextEdit*>(beWatched))
		{
			QObject::connect(beWatched, SIGNAL(textChanged()), this, SLOT(OnTextChanged()));
		}
        else if (qobject_cast<SliderInterface*>(beWatched))
		{
            QObject::connect(qobject_cast<SliderInterface*>(beWatched), &SliderInterface::valueChanged, this, &BiliPropChangeEventFilter::OnSilderChanged);
		}
		else //类型不支持！
			assert(0);
	}
}

bool BiliPropChangeEventFilter::eventFilter(QObject* watched, QEvent* event)
{
	//int eventType = event->type();
	//
	//if (eventType == QEvent::StyleChange || eventType == QEvent::FontChange || eventType == QEvent::PaletteChange)
	//	emit OnChanged();

	return false;
}

void BiliPropChangeEventFilter::OnChangedSlot()
{
	emit OnChangedSignal();
}

void BiliPropChangeEventFilter::OnChanged()
{
	isSignalTriggered = true;
	applyChangeTimer.stop();
	applyChangeTimer.start();
	applyChangeTimer.setSingleShot(true);
	applyChangeTimer.setInterval(300);
}

void BiliPropChangeEventFilter::OnTextChanged() { emit OnChanged(); }
void BiliPropChangeEventFilter::OnTextChanged(const QString&) { emit OnChanged(); }
void BiliPropChangeEventFilter::OnCheckBoxChanged() { emit OnChanged(); }
void BiliPropChangeEventFilter::OnToggled(bool checked) { emit OnChanged(); }
void BiliPropChangeEventFilter::OnComboBoxIndexChanged(int index) { emit OnChanged(); }
void BiliPropChangeEventFilter::OnSilderChanged(int val) { emit OnChanged(); }

BiLiPropertyDlg::BiLiPropertyDlg(QString &name, obs_sceneitem_t* pSceneItem, bool isNewSource, QWidget *parent)
	: QDialog(parent),
	mIsPressed(false),
	mSceneItem(pSceneItem),
	mSrc(obs_sceneitem_get_source(pSceneItem)),
	mSourceName(name),
	mAcceptButtonAction(0),
	vol_slider_(0),
	mChangeEvnetFilter(new BiliPropChangeEventFilter()),
	mIsLimitRectSelected(false),
	mIsNewSource(isNewSource)
{
	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

	obs_data_t* sourceSettings = obs_source_get_settings(mSrc);

	//保存进入窗口时source的设置值。不包含filter等附加属性
	obs_data_t* backupSettings = obs_data_create();
	obs_data_apply(backupSettings, sourceSettings);

	mBackupSettings = backupSettings;

	obs_sceneitem_get_pos(mSceneItem, &mBackupItemPos);
	obs_sceneitem_get_scale(mSceneItem, &mBackupItemScale);

	mBackupBoundsType = obs_sceneitem_get_bounds_type(mSceneItem);
	obs_sceneitem_get_bounds(mSceneItem, &mBackupBounds);

	obs_data_release(backupSettings);
	obs_data_release(sourceSettings);

	//备份filter数据
	mBackupFilters.reset(bili_backup_source_filter_settings(mSrc));
}

void BiLiPropertyDlg::InitUI()
{
	ui.setupUi(this);
	mSetupPropertyUI();
	mSetupConnect();

	mInitAcceptButtonAction();

	ui.mSourceName->setText(QApplication::translate("BiLiPropertyDlg", "source name : "));

	//新建源的时候已经什么都看不到了，下面这段代码洗洗睡吧
#if 0
	//新建源的时候，为了防止出现界面显示的内容和源实际内容不一致的情况
	//（比如窗口源，界面上默认不是给出一个空的选项……因为空字符串匹配不到现有窗口的
	if (mIsNewSource)
		mSltOnSettingChanged();
#endif

	//连接截屏事件到主窗口
	BiLiOBSMainWid *mainWnd = App()->mGetMainWindow();
	QObject::connect(this, &BiLiPropertyDlg::mSglScreenShotState, mainWnd, &BiLiOBSMainWid::onScreenShotStateChanged);

	move_widget_to_center(this, mainWnd);
}

BiLiPropertyDlg::~BiLiPropertyDlg() {

	if (filterMgr){
		delete filterMgr;
		filterMgr = NULL;
	}
}

#if 0
QString BiLiPropertyDlg::mGetMethodName(QString prefixStr, QString suffixStr) {

	QStringList srcIDL;
	srcIDL << "dshow_input" << "game_capture"
		<< "monitor_capture" << "window_capture"
		<< "ffmpeg_source" << "text_ft2_source" << "image_source";

	int idIndex = srcIDL.indexOf( obs_source_get_id(mSrc) );
	if (idIndex < 0)
		return QString("");

	QString methodName = srcIDL[idIndex];
	QStringList methodNameL = methodName.trimmed().split(" ");
	QString actMethod = QString("%1%2%3").arg(prefixStr).arg(methodNameL.join("")).arg(suffixStr);

	return actMethod;
}
#endif

void BiLiPropertyDlg::mSetupPropertyUI() {

	if (!mSrc)
		return;

	ui.nameEdit->setText(mSourceName);
	mChangeEvnetFilter->Watch({ui.nameEdit});

	//音量
	if (volumnAccess()) {

		QLayout *vol_layout = createVolSliderLayout();
		ui.VolSliderWid->setLayout(vol_layout);
        ui.VolSliderWid->setFixedHeight(20 + 16);
		//initVolControl();
	} else {
		ui.VolSliderWid->hide();
	}

	setupSourcePropertiesUI();
	//QMetaObject::invokeMethod(this, mGetMethodName("mAdd", "UI").toLocal8Bit().data());
}

void BiLiPropertyDlg::mSetupConnect() {
	connect(ui.CloseBtn, SIGNAL(clicked()), this, SLOT(mSltRejected()));
	connect(ui.AcceptedBtn, SIGNAL(clicked()), this, SLOT(mSltAcceptedBtn()));
	connect(ui.RejectedBtn, SIGNAL(clicked()), this, SLOT(mSltRejected()));

	connect(ui.nameEdit, &QLineEdit::textChanged, this, &BiLiPropertyDlg::mOnNameEditChanged);
}

void BiLiPropertyDlg::mSltRejected(){

	if (!mSrc)
		return;

	if (0 != vol_slider_) {
		vol_slider_->revert();
	}

	//如果设置有变化，并且不是新添加的源，那么要将源的设置还原
	if (mIsNewSource == false && mChangeEvnetFilter->IsChangedTriggered() == true)
	{
		obs_source_update(mSrc, mBackupSettings);
		obs_sceneitem_set_pos(mSceneItem, &mBackupItemPos);
		obs_sceneitem_set_scale(mSceneItem, &mBackupItemScale);

		obs_sceneitem_set_bounds_type(mSceneItem, mBackupBoundsType);
		obs_sceneitem_set_bounds(mSceneItem, &mBackupBounds);

		bili_restore_source_filter_settings(mSrc, mBackupFilters.get());
	}

	done(QDialog::Rejected);
}

void BiLiPropertyDlg::mSltAcceptedBtn() {

	if (!mSrc)
		return;

	int ret = 0;

	ret = acceptSourceProperties();
	//QMetaObject::invokeMethod(this, mGetMethodName("mAccepted", "").toLocal8Bit().data(),
	//	Q_RETURN_ARG(int, ret));

	if (2 != ret) {
		if (mCheckSourceNameLegal()) {
			std::string aux = mSourceName.toStdString();
			obs_source_set_name(mSrc, aux.c_str());
		}
		else {
#if 0
			//重名时候的处理
			QMessageBox errMsg(this);
			errMsg.setWindowTitle(tr("Error"));
			errMsg.setText(tr("Duplicated name!"));
			errMsg.exec();
#else
				//BiLiMsgDlg errDlg;
				//errDlg.mSetMsgTxtAndBtn(tr("Duplicated name!"), false);
				//errDlg.mSetTitle(tr("Error"));
				//errDlg.exec();


                OperTipDlg *errDlg = OperTipDlgFactory::makeDlg(OperTipDlgFactory::NAME_DUPLICATE);
                errDlg->exec();
                delete errDlg;

#endif
			ret = QDialog::Rejected;
		}
	}

	done(ret);
}

void BiLiPropertyDlg::mSltBrowserBtn() {

	QFileDialog fileDlg(this);
	fileDlg.setFileMode(QFileDialog::ExistingFiles);

	QStringList fileNames;
	if (fileDlg.exec())
		fileNames = fileDlg.selectedFiles();

	if (fileNames.isEmpty()){
		//QMessageBox::question(this,
		//	QString(tr("Warning")), QString(tr("You have not selected file")),
		//	QMessageBox::Yes);
		return;
	}

	mFileNameEdit->setText(fileNames.first());

	auto btn = qobject_cast<QPushButton *>(ui.AcceptedBtn);
	btn->setProperty("FileList", QVariant(fileNames));
}

void BiLiPropertyDlg::mousePressEvent(QMouseEvent *e) {

	if (e->button() & Qt::LeftButton)
		mIsPressed = true;

	mPoint = e->globalPos() - pos();
}

void BiLiPropertyDlg::mouseMoveEvent(QMouseEvent *e) {

	if (mIsPressed ) {
		QPoint point = e->globalPos();
		move(point - mPoint);
	}
}

void BiLiPropertyDlg::mouseReleaseEvent(QMouseEvent *e) {
	if (e->button() & Qt::LeftButton)
		mIsPressed = false;
}

void BiLiPropertyDlg::mSltOnSettingChanged()
{
	if (!mSrc)
		return;

	//在新添加来源的时候，因为来源内容是隐藏的，所以不需要更新
	if (mIsNewSource)
		return;

	int ret = 0;
	ret = acceptSourceProperties();
	//QMetaObject::invokeMethod(this, mGetMethodName("mAccepted", "").toLocal8Bit().data(),
	//	Q_RETURN_ARG(int, ret));
}

void BiLiPropertyDlg::mOnNameEditChanged()
{
	mSourceName = ui.nameEdit->text();
}

void BiLiPropertyDlg::mInitAcceptButtonAction()
{
	if (0 != mAcceptButtonAction)
		return;
	
	QList<QKeySequence> list;
	list.append(QKeySequence(Qt::Key_Return));
	list.append(QKeySequence(Qt::Key_Enter));
	
	mAcceptButtonAction = new QAction(this);
	mAcceptButtonAction->setShortcuts(list);
	addAction(mAcceptButtonAction);

	connect(mAcceptButtonAction, &QAction::triggered, this, &BiLiPropertyDlg::mOnAcceptButtonActionTriggered); 
}

void BiLiPropertyDlg::mOnAcceptButtonActionTriggered()
{
	if (ui.AcceptedBtn->isEnabled())
		mSltAcceptedBtn();
}

QGroupBox* BiLiPropertyDlg::CreateTipGroupBox(const QString& text)
{
	//auto PropertyTipGroupBox = new QGroupBox(ui.PropertyWid);
	auto PropertyTipGroupBox = new QGroupBox();
	PropertyTipGroupBox->setObjectName(QStringLiteral("PropertyTipGroupBox"));

	auto layout = new QHBoxLayout(PropertyTipGroupBox);

	auto PropertyTip = new QLabel(PropertyTipGroupBox);
	PropertyTip->setObjectName(QStringLiteral("PropertyTip"));
	PropertyTip->setStyleSheet(QStringLiteral(""));
	PropertyTip->setFixedSize(28, 28);
	PropertyTip->setMargin(16);

	auto PropertyTipTxt = new QLabel(PropertyTipGroupBox);
	PropertyTipTxt->setObjectName(QStringLiteral("PropertyTipTxt"));
	PropertyTipTxt->setAlignment(Qt::AlignVCenter | Qt::AlignLeading | Qt::AlignLeft);
	PropertyTipTxt->setWordWrap(true);
	PropertyTipTxt->setStyleSheet(QLatin1String("\n"
		"QLabel#PropertyTipTxt{\n"
		"    font-size: 10pt;\n"
		"    color: #8FC5E3;\n"
		"}\n"));

	PropertyTipTxt->setText(text);
	PropertyTipTxt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	PropertyTipTxt->setContentsMargins(0, 12, 0, 12);

	layout->addWidget(PropertyTip);
	layout->addWidget(PropertyTipTxt);
	PropertyTipGroupBox->setLayout(layout);

	return PropertyTipGroupBox;
}

////////////////////////////////////////
//filter mgr

/*
mask_filter 图像掩码/混合
crop_filter 剪裁
gain_filter 增益
color_filter 色彩校正
scroll_filter 滚动
color_key_filter 色值
sharpness_filter 锐化
chroma_key_filter 色度键
async_delay_filter 视频延迟(异步)
noise_gate_filter 噪音阈值
*/


#define FILTER_SIGNAL	0
BiliFilterMgr::BiliFilterMgr(obs_source_t *src):mSrc(src) {
#if FILTER_SIGNAL	
		mAddSignal = OBSSignal(obs_source_get_signal_handler(mSrc), "filter_add", BiliFilterMgr::mSourceFilterAdded, this);
		mRemoveSignal = OBSSignal(obs_source_get_signal_handler(mSrc), "filter_remove", BiliFilterMgr::mSourceFilterRemoved, this);
#endif
}
BiliFilterMgr::~BiliFilterMgr() {

	//QList<obs_source_t *> filters = mFilterMap.values();
	//for (int i = 0; i < filters.count(); i++)
	//	obs_source_release(filters.at(i));

	//QList<obs_data_t *> settings = mFilterSettingsMap.values();
	//for (int i = 0; i < settings.count(); i++)
	//	obs_data_release(settings.at(i));

	//mFilterMap.clear();
	//mFilterSettingsMap.clear();
}
void BiliFilterMgr::mSltAddFilter(OBSSource filter) { }
void BiliFilterMgr::mSltRemoveFilter(OBSSource filter){ }

obs_source_t *BiliFilterMgr::mGetFilter(char *filterName){
	if (mFilterMap.contains(filterName))
		return mFilterMap[filterName];
	return NULL;
	/*
	obs_source_t *existingFilter = obs_source_get_filter_by_name(mSrc, obs_source_get_display_name(OBS_SOURCE_TYPE_FILTER, filterName));
	if (existingFilter)
	return existingFilter;
	return NULL;
	*/
}

void BiliFilterMgr::mGetFilterProperties(char *filterName, bili_source_props callback, void *param){

	if (mFilterMap.contains(filterName)){
		obs_properties_t *props = obs_source_properties(mFilterMap[filterName]);
		if (props){
			callback(props, param);
			obs_properties_destroy(props);
		}
	}
}

obs_data_t *BiliFilterMgr::mGetFilterSettings(char *filterName){

	obs_data_t *existingSettings = obs_source_get_settings(mFilterMap[filterName]);

	mFilterSettingsMap[filterName] = obs_data_create();
	obs_data_apply(mFilterSettingsMap[filterName], existingSettings);

	obs_data_release(existingSettings);
	return existingSettings;
}

bool BiliFilterMgr::mAddFilter(char *filterName, bool isCreate){
	if (mFilterMap.contains(filterName))
		return true;

	uint32_t sourceFlags = obs_source_get_output_flags(mSrc);
	uint32_t filterFlags = obs_get_source_output_flags(OBS_SOURCE_TYPE_FILTER, filterName);
	if (!mFilterCompatible(sourceFlags, filterFlags))
		return false;

	const char *name = obs_source_get_display_name(OBS_SOURCE_TYPE_FILTER, filterName);
	obs_source_t *existingFilter = obs_source_get_filter_by_name(mSrc, name);
	if (existingFilter){
		mFilterMap[filterName] = existingFilter;
		obs_source_release(existingFilter);
		return true;
	}

	if (isCreate){
		obs_source_t *newFilter = obs_source_create(OBS_SOURCE_TYPE_FILTER, filterName,
			name, NULL, NULL);
		if (newFilter){
			obs_source_filter_add(mSrc, newFilter);
			mFilterMap[filterName] = newFilter;
			obs_source_release(newFilter);
			return true;
		}
		return false;
	}
	return false;
}

bool BiliFilterMgr::mFilterCompatible(uint32_t sourceFlags, uint32_t filterFlags, bool async){

	bool filterVideo = (filterFlags & OBS_SOURCE_VIDEO) != 0;
	bool filterAsync = (filterFlags & OBS_SOURCE_ASYNC) != 0;
	bool filterAudio = (filterFlags & OBS_SOURCE_AUDIO) != 0;
	bool audio = (sourceFlags & OBS_SOURCE_AUDIO) != 0;
	bool audioOnly = (sourceFlags & OBS_SOURCE_VIDEO) == 0;
	bool asyncSource = (sourceFlags & OBS_SOURCE_ASYNC) != 0;

	if (async && ((audioOnly && filterVideo) || (!audio && !asyncSource)))
		return false;

	return (async && (filterAudio || filterAsync)) ||
		(!async && !filterAudio && !filterAsync);
}

QLayout* BiLiPropertyDlg::createVolSliderLayout()
{
	QLabel *vol_label = new QLabel();
	vol_label->setText(tr("volumn value:"));
    vol_label->setFixedHeight(16);


	vol_slider_ = new PropertyDlgVolSliderWid(mSrc);
	vol_slider_->setFixedSize(134, 16);

	QSpacerItem *vol_spacer = new QSpacerItem(6, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);

	QHBoxLayout *vol_layout = new QHBoxLayout();
	vol_layout->setSpacing(0);
	vol_layout->setContentsMargins(41, 20, 154, 0);
	vol_layout->addWidget(vol_label);
	vol_layout->addItem(vol_spacer);
	vol_layout->addWidget(vol_slider_);


	return vol_layout;
}

bool BiLiPropertyDlg::volumnAccess()
{
	uint32_t flags = obs_source_get_output_flags(mSrc);

	//不能设置控制音量
	if (0 == (flags & OBS_SOURCE_AUDIO))
		return false;

	return true;
}

bool BiLiPropertyDlg::mCheckSourceNameLegal()
{
	QString name_copy(obs_source_get_name(mSrc));
	if (mSourceName != name_copy) {
		obs_source_t* existedSource = obs_get_source_by_name(mSourceName.toUtf8());

		if (existedSource) {
			obs_source_release(existedSource);
			return false;
		}
	}
	return true;
}

BiLiPropertyDialogFactory* GetBiLiPropertyDialogFactory(const char* sourceId)
{
	BiLiPropertyDialogFactory* f = g_BiLiPropertyDialogFactoryList;
	while (f != 0)
	{
		if (f->CheckId(sourceId))
			return f;
		f = f->mNextFactory;
	}
	return 0;
}


BiLiPropertyDialogFactory* g_BiLiPropertyDialogFactoryList = 0;
BiLiPropertyDialogFactory* g_curBiLiPropertyDialogFactoryList = 0;
