#define DEFINE_IMPL_PROPDLG
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QStyledItemDelegate>
#include <QStackedLayout>
#include <QAction>
#include <QColorDialog>

#include "../biliapi/IBiliApi.h"
#include "BiLiPropertyDlg.h"
#include "BiLiMsgDlg.h"
#include "BiliOBSUtility.hpp"
#include "circle_slider_slider.h"

#include "system_ret_info_dlg.h"
IMPL_PROPDLG(BiLiDShowSourcePropertyDlg);
	QComboBox* DevNameComboBox;
	QCheckBox* VCamCheckBox;
	QCheckBox* HCamCheckBox;
	QComboBox* formatComboBox;
	CircleSliderSlider* beautySlider;

	QVBoxLayout *MainVLayout;
	obs_source_t* chromaKeyFilter_;
	QCheckBox *chromaKeyCheckBox_;
	CircleSliderSlider *opacitySlider_;
	CircleSliderSlider *similaritySlider_;
	CircleSliderSlider *smoothnessSlider_;
	CircleSliderSlider *spillSlider_;
	QLabel *colorLab_;
	QPushButton *colorChangeBtn_;
	QLabel *opacityLab_;
	QLabel *similarityLab_;
	QLabel *smoothnessLab_;
	QLabel *spillLab_;
	QLabel *opacityVolLab_;
	QLabel *similarityVolLab_;
	QLabel *smoothnessVolLab_;
	QLabel *spillVolLab_;
	QVBoxLayout *chromaKeyVLayout_;
	QStackedLayout *chromaKeySLayout_;
	QVBoxLayout *chromaKeyCtrlVLayout_;
	QWidget *chromaKeyCtrlWid_;

	void createChromakeySettings_();
	void initChromakeyCtrls_();
	void setChromaKeyLayout_(bool isShow);
	bool hasCreateSettingCtrls_ = false;
	void sltColorChangeBtn_();
	void sltSliderChanged_(int val);

	void showEvent(QShowEvent * event) override;
	void checkDeviceExists(bool);

	void onAdjustDevClicked();

	QAction* checkDeviceAction;
END_IMPL_PROPDLG(BiLiDShowSourcePropertyDlg, "dshow_input");

static const char* beauty_filter_id = "beauty_filter";
static const char* chromeKeyFilterId = "chroma_key_filter";

BiLiDShowSourcePropertyDlg::~BiLiDShowSourcePropertyDlg() {}
static std::string removeKakko(const std::string& x)
{
	std::string r = x;

	std::string::size_type index = r.find('(');
	if (index != std::string::npos)
	{
		r = r.substr(0, index);
	}
	
	if (r.empty() == false)
	{
		std::string::iterator begin = r.begin(), end = r.end() - 1;
		while (begin != r.end() && *begin == ' ')
			++begin;
		while (end != begin && *end == ' ')
			--end;
		++end;

		if (begin != end)
			return std::string(begin, end);
		else
			return std::string();
	}
	else
		return std::string();
}

void BiLiDShowSourcePropertyDlg::setupSourcePropertiesUI() {
	//注意：添加控件时，记得根据需要在最后添加控件的变动通知监视！
	//否则可能导致点了确定之后设置没有保存进去

	ui.PropertyNameLab->setText(tr("Camera Property")); 
	QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();

	auto DevComboxHLayout = new QHBoxLayout();
	auto DevLab = new QLabel(ui.PropertyWid);
	DevLab->setObjectName(QStringLiteral("DevLab"));
	QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(DevLab->sizePolicy().hasHeightForWidth());
	DevLab->setSizePolicy(sizePolicy);
	DevLab->setMinimumSize(QSize(63, 30));
	DevLab->setMaximumSize(QSize(63, 30));
	DevLab->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	DevComboxHLayout->addWidget(DevLab);

	QSpacerItem *DevSpacer = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
	DevComboxHLayout->addItem(DevSpacer);

	DevNameComboBox = new QComboBox(ui.PropertyWid);
	DevNameComboBox->setObjectName(QStringLiteral("DevNameComboBox"));
	DevNameComboBox->setMinimumSize(QSize(0, 30));
	DevNameComboBox->setMaximumSize(QSize(16777215, 30));
	DevComboxHLayout->addWidget(DevNameComboBox);
	DevNameComboBox->setItemDelegate(itemDelegate);

	QSpacerItem *DevSpacer2 = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
	DevComboxHLayout->addItem(DevSpacer2);
	QPushButton* devSettingButton = new QPushButton(ui.PropertyWid);
	devSettingButton->setObjectName("DevAdjustButton");
	devSettingButton->setText(tr("Adjust"));
	devSettingButton->setFixedSize(80, 30);
	QObject::connect(devSettingButton, &QPushButton::clicked, this, &BiLiDShowSourcePropertyDlg::onAdjustDevClicked);
	DevComboxHLayout->addWidget(devSettingButton);

    DevComboxHLayout->setSpacing(0);
    DevComboxHLayout->setContentsMargins(0, 0, 0, 0);

    auto StyleHSpacer1 = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    auto StyleHSpacer2 = new QSpacerItem(30, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    auto StyleHSpacer3 = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed);
	auto CamDirectionHLayout = new QHBoxLayout();
	auto CamDirectionLab = new QLabel(ui.PropertyWid);
	CamDirectionLab->setObjectName(QStringLiteral("CamDirectionLab"));
	sizePolicy.setHeightForWidth(CamDirectionLab->sizePolicy().hasHeightForWidth());
	CamDirectionLab->setSizePolicy(sizePolicy);
	CamDirectionLab->setMinimumSize(QSize(63, 14));
	CamDirectionLab->setMaximumSize(QSize(63, 14));
	CamDirectionLab->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	CamDirectionHLayout->addWidget(CamDirectionLab);
	CamDirectionHLayout->addItem(StyleHSpacer1);

	VCamCheckBox = new QCheckBox(ui.PropertyWid);
    VCamCheckBox->setFixedHeight(14);
	VCamCheckBox->setObjectName(QStringLiteral("VCamCheckBox"));
	
	CamDirectionHLayout->addWidget(VCamCheckBox);
    CamDirectionHLayout->addItem(StyleHSpacer2);

	HCamCheckBox = new QCheckBox(ui.PropertyWid);
	HCamCheckBox->setObjectName(QStringLiteral("HCamCheckBox"));
    HCamCheckBox->setFixedHeight(14);
	CamDirectionHLayout->addWidget(HCamCheckBox);
    CamDirectionHLayout->addItem(StyleHSpacer3);
    CamDirectionHLayout->setSpacing(0);
    CamDirectionHLayout->setContentsMargins(0, 0, 0, 0);

	MainVLayout = new QVBoxLayout(ui.PropertyWid);
	MainVLayout->setContentsMargins(13, 18, 13, 30);
    MainVLayout->setSpacing(0);

	//美颜设置的框，左边是美颜的文字标签，右边是一个组合的widget
	QHBoxLayout* beautySettingLayout = new QHBoxLayout(ui.PropertyWid);
	beautySettingLayout->setContentsMargins(0, 0, 0, 0);
	beautySettingLayout->setSpacing(0);
	QLabel* beautyLabel = new QLabel(ui.PropertyWid);
	beautyLabel->setFixedSize(62, 30);
	beautyLabel->setText(tr("Beauty Level:"));
	QWidget* beautySliderContainer = new QWidget(ui.PropertyWid);
	beautySliderContainer->setFixedSize(252, 28); //高度是整个slider外加label的大小，下面0~5的label会靠下对齐
	beautySlider = new CircleSliderSlider(beautySliderContainer);
	beautySlider->setRange(0, 4, 0);
	beautySlider->setFixedHeight(16);
	beautySlider->setFixedWidth(252);
	beautySlider->move(0, 0);

	QSpacerItem* beautySpacer = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
	beautySettingLayout->addWidget(beautyLabel);
	beautySettingLayout->addItem(beautySpacer);
	//beautySettingLayout->addWidget(beautyComboBox);
	beautySettingLayout->addWidget(beautySliderContainer);

	QWidget* beautySettingTextWidget = new QWidget(beautySliderContainer);
	beautySettingTextWidget->setFixedWidth(beautySliderContainer->width());
	beautySettingTextWidget->setFixedHeight(beautySliderContainer->height());
	beautySettingTextWidget->setAttribute(Qt::WA_TransparentForMouseEvents);

	QHBoxLayout* beautySettingTextInternalLayout = new QHBoxLayout(beautySettingTextWidget);
	beautySettingTextInternalLayout->setContentsMargins(5, beautySliderContainer->height() - beautySlider->height(), 5, 0);
	beautySettingTextInternalLayout->setSpacing(0);

	beautySettingTextInternalLayout->addWidget(new QLabel(tr("0"), beautySettingTextWidget));
	for (int i = 1; i <= 4; ++i)
	{
		beautySettingTextInternalLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
		beautySettingTextInternalLayout->addWidget(new QLabel(lexical_cast<std::string>(i).c_str(), beautySettingTextWidget));
	}

	QSpacerItem* beautySettingTextSpacer = new QSpacerItem(6, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);

	QHBoxLayout* formatSettingLayout = new QHBoxLayout();
	formatSettingLayout->setContentsMargins(0, 0, 0, 0);
	formatSettingLayout->setSpacing(0);
	QLabel* formatLabel = new QLabel(ui.PropertyWid);
	formatLabel->setFixedSize(62, 30);
	formatLabel->setText(tr("Force Resolution:"));
	formatComboBox = new QComboBox(ui.PropertyWid);
	formatComboBox->setItemDelegate(itemDelegate);
	formatComboBox->setFixedHeight(30);
	formatComboBox->setEditable(true);
	QSpacerItem* formatSpacer = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
	formatSettingLayout->addWidget(formatLabel);
	formatSettingLayout->addItem(formatSpacer);
	formatSettingLayout->addWidget(formatComboBox);

	formatComboBox->addItem(tr("(disabled)"));
	formatComboBox->addItem("640x480");
	formatComboBox->addItem("720x480");
	formatComboBox->addItem("720x576");
	formatComboBox->addItem("1024x768");
	formatComboBox->addItem("1280x720");
	formatComboBox->addItem("1280x768");
	formatComboBox->addItem("1280x800");
	formatComboBox->addItem("1440x900");
	formatComboBox->addItem("1920x1080");


	QHBoxLayout* filterLayout = new QHBoxLayout();
	filterLayout->setContentsMargins(0, 0, 0, 0);
	filterLayout->setSpacing(0);
	auto filtersLab = new QLabel(ui.PropertyWid);
	filtersLab->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	filtersLab->setFixedSize(60, 30);
	chromaKeyCheckBox_ = new QCheckBox(ui.PropertyWid);
	QSpacerItem* filtersSpacer = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
	filterLayout->addWidget(filtersLab);
	filterLayout->addItem(filtersSpacer);
	filterLayout->addWidget(chromaKeyCheckBox_);

	QGroupBox* tipGrpBox = CreateTipGroupBox(tr("Hint: Only use when necessary."));
	tipGrpBox->setFixedHeight(100);
    tipGrpBox->setStyleSheet("margin-left:3px;");


	QSpacerItem *spacer_v0 = new QSpacerItem(10, 18, QSizePolicy::Fixed, QSizePolicy::Fixed);
	QSpacerItem *spacer_v1 = new QSpacerItem(10, 18, QSizePolicy::Fixed, QSizePolicy::Fixed);
	QSpacerItem *spacer_v2 = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
	QSpacerItem *spacer_v3 = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
	QSpacerItem *spacer_v4 = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);

	MainVLayout->addLayout(DevComboxHLayout);
	MainVLayout->addItem(spacer_v0);
	MainVLayout->addLayout(CamDirectionHLayout);
	MainVLayout->addItem(spacer_v1);
	MainVLayout->addLayout(beautySettingLayout);
	MainVLayout->addItem(spacer_v2);
	MainVLayout->addLayout(formatSettingLayout);
	MainVLayout->addItem(spacer_v3);
	MainVLayout->addLayout(filterLayout);
	MainVLayout->addItem(spacer_v4);
	MainVLayout->addWidget(tipGrpBox);

	DevLab->setText(QApplication::translate("CameraAddForm", "Devices:", 0));
	CamDirectionLab->setText(QApplication::translate("CameraAddForm", "Camera Direction:", 0));
	VCamCheckBox->setText(QApplication::translate("CameraAddForm", "Vertical Reversal", 0));
	HCamCheckBox->setText(QApplication::translate("CameraAddForm", "Horizontal Reversal", 0));
	filtersLab->setText(QApplication::translate("CameraAddForm", "Chroma:", 0));
	chromaKeyCheckBox_->setText(QApplication::translate("CameraAddForm", "Chroma Key", 0));

	this->setProperty("FlipHorizontallyCheckbox", qVPtr<QCheckBox>::toVariant(HCamCheckBox));

	QMetaObject::connectSlotsByName(ui.PropertyWid);

	obs_properties_t* props = obs_source_properties(mSrc);
	DataToWidget(BILI_PROP_LIST_STRING(), DevNameComboBox, props, "video_device_id");
	obs_properties_destroy(props);

	//开始-加载设置
	obs_data_t* settings = obs_source_get_settings(mSrc);
	DataToWidget(BILI_DATA_STRING(), DevNameComboBox, settings, "video_device_id");
	DataToWidget(BILI_DATA_BOOL(), VCamCheckBox, settings, "flip_vertically");

	DataToWidget(BILI_DATA_BOOL(), chromaKeyCheckBox_, settings, "chroma_key");
	if (chromaKeyCheckBox_->isChecked()){
		chromaKeyFilter_ = obs_source_get_filter_by_name(mSrc, chromeKeyFilterId);
		if (chromaKeyFilter_){
			createChromakeySettings_();
			initChromakeyCtrls_();
		}
	}

	//对水平翻转特殊处理
	vec2 itemScale;
	obs_sceneitem_get_scale(mSceneItem, &itemScale);
	if (itemScale.x >= 0.0f)
	{
		HCamCheckBox->setChecked(false);
	}
	else
	{
		HCamCheckBox->setChecked(true);
	}
	//格式
	int isCustomFormat = obs_data_get_int(settings, "res_type");
	if (isCustomFormat == 0)
	{
		formatComboBox->setCurrentIndex(0);
	}
	else
	{
		DataToWidget(BILI_DATA_STRING(), formatComboBox, settings, "resolution");
	}
	obs_data_release(settings);

	FilterDataToWidget(BILI_DATA_INT(), beautySlider, mSrc, beauty_filter_id, "beauty_level");

	//结束-加载设置

	//添加监听控件变动
	mChangeEvnetFilter->Watch({VCamCheckBox, HCamCheckBox, DevNameComboBox, beautySlider, formatComboBox, chromaKeyCheckBox_});
	connect(mChangeEvnetFilter.get(), SIGNAL(OnChangedSignal()), this, SLOT(mSltOnSettingChanged()));

	checkDeviceAction = new QAction(this);
	QObject::connect(checkDeviceAction, &QAction::triggered, this, &BiLiDShowSourcePropertyDlg::checkDeviceExists, Qt::ConnectionType::QueuedConnection);
}

void BiLiDShowSourcePropertyDlg::initChromakeyCtrls_() {

	obs_properties_t* props = obs_source_properties(chromaKeyFilter_);
	if (props) {
		obs_property_t *similarityProp = obs_properties_get(props, "similarity");
		if (similarityProp) {
			int minVal = obs_property_int_min(similarityProp);
			int maxVal = obs_property_int_max(similarityProp);
			int stepVal = obs_property_int_step(similarityProp);
			similaritySlider_->setRange(minVal, maxVal, maxVal);
		}
		obs_property_t *smoothnessProp = obs_properties_get(props, "smoothness");
		if (smoothnessProp) {
			int minVal = obs_property_int_min(smoothnessProp);
			int maxVal = obs_property_int_max(smoothnessProp);
			int stepVal = obs_property_int_step(smoothnessProp);
			smoothnessSlider_->setRange(minVal, maxVal, maxVal);
		}
		obs_property_t *spillProp = obs_properties_get(props, "spill");
		if (spillProp) {
			int minVal = obs_property_int_min(spillProp);
			int maxVal = obs_property_int_max(spillProp);
			int stepVal = obs_property_int_step(spillProp);
			spillSlider_->setRange(minVal, maxVal, maxVal);
		}
		obs_property_t *opacityProp = obs_properties_get(props, "opacity");
		if (opacityProp) {
			int minVal = obs_property_int_min(opacityProp);
			int maxVal = obs_property_int_max(opacityProp);
			int stepVal = obs_property_int_step(opacityProp);
			spillSlider_->setRange(minVal, maxVal, maxVal);
		}
		obs_properties_destroy(props);

		obs_data_t *filterSettings = obs_source_get_settings(chromaKeyFilter_);
		similaritySlider_->setValue(obs_data_get_int(filterSettings, "similarity"));
		smoothnessSlider_->setValue(obs_data_get_int(filterSettings, "smoothness"));
		spillSlider_->setValue(obs_data_get_int(filterSettings, "spill"));
		opacitySlider_->setValue(obs_data_get_int(filterSettings, "opacity"));

		uint32_t  key_color = (uint32_t)obs_data_get_int(filterSettings, "key_color");
		SetPushButtonBackgroundColor(colorChangeBtn_, key_color|0xff000000);

		similarityVolLab_->setText(QString("%1").arg(similaritySlider_->value()));
		smoothnessVolLab_->setText(QString("%1").arg(smoothnessSlider_->value()));
		spillVolLab_->setText(QString("%1").arg(spillSlider_->value()));
		opacityVolLab_->setText(QString("%1").arg(opacitySlider_->value()));

		obs_data_release(filterSettings);
	}

	WidgetToFilterData(BILI_DATA_INT(), similaritySlider_, mSrc, chromeKeyFilterId, "similarity");
	WidgetToFilterData(BILI_DATA_INT(), smoothnessSlider_, mSrc, chromeKeyFilterId, "smoothness");
	WidgetToFilterData(BILI_DATA_INT(), spillSlider_, mSrc, chromeKeyFilterId, "spill");
	WidgetToFilterData(BILI_DATA_INT(), opacitySlider_, mSrc, chromeKeyFilterId, "opacity");
	WidgetToFilterData(BILI_DATA_INT(), colorChangeBtn_, mSrc, chromeKeyFilterId, "key_color");
}

int BiLiDShowSourcePropertyDlg::acceptSourceProperties() {

	obs_data_t* settings = obs_source_get_settings(mSrc);
	WidgetToData(BILI_DATA_STRING(), DevNameComboBox, settings, "video_device_id");
	WidgetToData(BILI_DATA_BOOL(), VCamCheckBox, settings, "flip_vertically");

	// chroma Key
	WidgetToData(BILI_DATA_BOOL(), chromaKeyCheckBox_, settings, "chroma_key");
	obs_source_t *chromaKeyFilter = obs_source_get_filter_by_name(mSrc, chromeKeyFilterId);
	if (chromaKeyCheckBox_->isChecked()){
		//add chroma key
		if (!chromaKeyFilter){
			chromaKeyFilter_ = obs_source_create(OBS_SOURCE_TYPE_FILTER, chromeKeyFilterId, chromeKeyFilterId, 0, 0);

			obs_data_t *filterSettings = obs_source_get_settings(chromaKeyFilter_);
			obs_data_set_string(filterSettings, "key_color_type", "custom");
			obs_source_update(chromaKeyFilter_, filterSettings);
			obs_source_filter_add(mSrc, chromaKeyFilter_);

			createChromakeySettings_();
			initChromakeyCtrls_();
			obs_data_release(filterSettings);
			setChromaKeyLayout_(true);
		}
	}else{
		//delete chroma key
		if (chromaKeyFilter) {
			obs_source_filter_remove(mSrc, chromaKeyFilter);
			obs_source_release(chromaKeyFilter);
			setChromaKeyLayout_(false);
		}
	}

	//对水平翻转特殊处理
	vec2 itemScale;
	obs_sceneitem_get_scale(mSceneItem, &itemScale);
	if ((itemScale.x < 0 && HCamCheckBox->isChecked() == false)
		|| (itemScale.x > 0 && HCamCheckBox->isChecked() == true)
		)
	{
		itemScale.x = -itemScale.x;
		obs_sceneitem_set_scale(mSceneItem, &itemScale);
		vec2 itemPos;
		int sourceWidth;
		obs_sceneitem_get_pos(mSceneItem, &itemPos);
		sourceWidth = obs_source_get_base_width(mSrc);
		itemPos.x -= sourceWidth * itemScale.x;
		obs_sceneitem_set_pos(mSceneItem, &itemPos);
	}

	std::string formatString = formatComboBox->currentText().toStdString();
	formatString = removeKakko(formatString);
	if (formatString.empty())
	{
		obs_data_set_int(settings, "res_type", 0);
	}
	else
	{
		obs_data_set_int(settings, "res_type", 1);
		obs_data_set_string(settings, "resolution", formatString.c_str());
	}
	obs_source_update(mSrc, settings);
	obs_data_release(settings);

	//if (beautyComboBox->currentData().toInt() == 0)
	if (beautySlider->value() == 0)
	{
		//删除美颜filter
		obs_source_t* beautyFilter = obs_source_get_filter_by_name(mSrc, beauty_filter_id);
		if (beautyFilter)
		{
			obs_source_filter_remove(mSrc, beautyFilter);
			obs_source_release(beautyFilter);
		}
	}
	else
	{
		//添加美颜filter
		obs_source_t* beautyFilter = obs_source_get_filter_by_name(mSrc, beauty_filter_id);
		if (!beautyFilter)
		{
			beautyFilter = obs_source_create(OBS_SOURCE_TYPE_FILTER, beauty_filter_id, beauty_filter_id, 0, 0);
			obs_data_t* settings = obs_data_create();
			obs_data_set_int(settings, "beauty_level", 0);
			obs_source_update(beautyFilter, settings);
			obs_source_filter_add(mSrc, beautyFilter);
			obs_data_release(settings);
		}
		WidgetToFilterData(BILI_DATA_INT(), beautySlider, mSrc, beauty_filter_id, "beauty_level");
	}

	obs_source_t* filter = obs_source_get_filter_by_name(mSrc, beauty_filter_id);
	obs_source_update(filter, 0);
	obs_source_release(filter);

	return (QDialog::Accepted);
}

void BiLiDShowSourcePropertyDlg::showEvent(QShowEvent * event)
{
	checkDeviceAction->trigger();
	BiLiPropertyDlg::showEvent(event);
}

void BiLiDShowSourcePropertyDlg::checkDeviceExists(bool isChecked)
{
	if (DevNameComboBox->count() == 0) {

        SystemRetInfoDlg msgDlg;
        msgDlg.setTitle("");
        msgDlg.setSubTitle(tr("Error!"));
        msgDlg.setDetailInfo(tr("No Camera detected!"));
        msgDlg.exec();

		done(QDialog::Rejected);
	}
}

void BiLiDShowSourcePropertyDlg::onAdjustDevClicked()
{
	obs_properties_t* props = obs_source_properties(mSrc);
	obs_property_t* prop = obs_properties_get(props, "video_config");
	obs_property_button_clicked(prop, mSrc);
}

void BiLiDShowSourcePropertyDlg::setChromaKeyLayout_(bool isShow){

	if (isShow){
		chromaKeyCtrlWid_->setHidden(false);
	}
	else{
		if (hasCreateSettingCtrls_) {
			chromaKeyCtrlWid_->setHidden(true);
			ui.PropertyWid->adjustSize();
			adjustSize();
		}
	}
}

static unsigned shift(unsigned val, int shift) {
	return ((val & 0xff) << shift);
}

static inline long long color_to_int(QColor color) {
	return  shift(color.red(), 0) |
		shift(color.green(),  8) |
		shift(color.blue(),  16) |
		shift(color.alpha(), 24);
}

void BiLiDShowSourcePropertyDlg::sltColorChangeBtn_() {

	obs_data_t *filterSettings = obs_source_get_settings(chromaKeyFilter_);

	uint32_t key_color = (uint32_t)obs_data_get_int(filterSettings, "key_color");
	QColor c = QColorDialog::getColor(key_color, this, tr("Select color"), QColorDialog::ShowAlphaChannel);
	if (!c.isValid()){
		obs_data_release(filterSettings);
		return;
	}

	uint32_t colorVal = color_to_int(c);
	obs_data_set_int(filterSettings, "key_color", colorVal);

	SetPushButtonBackgroundColor(colorChangeBtn_, colorVal);
	obs_source_update(chromaKeyFilter_, filterSettings);
	obs_data_release(filterSettings);
}

void BiLiDShowSourcePropertyDlg::sltSliderChanged_(int val){

	QWidget *w = qobject_cast<QWidget *>(sender());
	char *s = (char *)malloc(32);
	memset(s, 0, 32);
	memcpy(s, w->objectName().toUtf8(), w->objectName().size());

	if (strcmp(s, "similarity") == 0)
		similarityVolLab_->setText(QString("%1").arg(val));
	else if (strcmp(s, "smoothness") == 0)
		smoothnessVolLab_->setText(QString("%1").arg(val));
	else if (strcmp(s, "spill") == 0)
		spillVolLab_->setText(QString("%1").arg(val));
	else if (strcmp(s, "opacity") == 0)
		opacityVolLab_->setText(QString("%1").arg(val));

	obs_data_t *filterSettings = obs_source_get_settings(chromaKeyFilter_);
	obs_data_set_int(filterSettings, s, val);
	obs_source_update(chromaKeyFilter_, filterSettings);
	obs_data_release(filterSettings);

	free(s);
}

void BiLiDShowSourcePropertyDlg::createChromakeySettings_(){

	if (!hasCreateSettingCtrls_) {
		
		similarityLab_ = new QLabel();
		smoothnessLab_ = new QLabel();
		spillLab_ = new QLabel();
		opacityLab_ = new QLabel();
		similarityLab_->setObjectName("similarityLab_");
		similarityLab_->setText(QApplication::translate("CameraAddForm", "Similarity :", 0));
	    similarityLab_->setFixedSize(63, 16);
	    similarityLab_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		smoothnessLab_->setObjectName("Smoothness");
		smoothnessLab_->setText(QApplication::translate("CameraAddForm", "Smoothness :", 0));
	    smoothnessLab_->setFixedSize(63, 16);
	    smoothnessLab_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		spillLab_->setObjectName("spillLab_");
		spillLab_->setText(QApplication::translate("CameraAddForm", "Spill :", 0));
	    spillLab_->setFixedSize(63, 16);
	    spillLab_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		opacityLab_->setObjectName("opacityLab_el");
		opacityLab_->setText(QApplication::translate("CameraAddForm", "Opacity :", 0));
	    opacityLab_->setFixedSize(63, 16);
	    opacityLab_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

		colorLab_ = new QLabel(ui.PropertyWid);
		colorLab_->setObjectName(QStringLiteral("colorLab_"));
		colorLab_->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
		colorLab_->setFixedSize(60, 30);
		colorLab_->setText(QApplication::translate("TextAddForm", "Color: ", 0));
		colorChangeBtn_ = new QPushButton(ui.PropertyWid);
		colorChangeBtn_->setObjectName(QStringLiteral("ColorChangeBtn"));
		colorChangeBtn_->setMinimumSize(QSize(50, 25));
		colorChangeBtn_->setMaximumSize(QSize(50, 25));

		opacitySlider_ = new CircleSliderSlider();
	    opacitySlider_->setFixedWidth(150);
		opacitySlider_->setFixedHeight(16);
		opacitySlider_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		similaritySlider_ = new CircleSliderSlider();
	    similaritySlider_->setFixedWidth(150);
		similaritySlider_->setFixedHeight(16);
		similaritySlider_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		smoothnessSlider_ = new CircleSliderSlider();
	    smoothnessSlider_->setFixedWidth(150);
		smoothnessSlider_->setFixedHeight(16);
		smoothnessSlider_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		spillSlider_ = new CircleSliderSlider();
	    spillSlider_->setFixedWidth(150);
		spillSlider_->setFixedHeight(16);
		spillSlider_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

		opacityVolLab_ = new QLabel();
		opacityVolLab_->setFixedWidth(44);
	    opacityVolLab_->setFixedHeight(16);
		opacityVolLab_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		opacityVolLab_->setObjectName("OpacityValLabel");
		similarityVolLab_ = new QLabel();
		similarityVolLab_->setFixedWidth(44);
	    similarityVolLab_->setFixedHeight(16);
		similarityVolLab_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		similarityVolLab_->setObjectName("OpacityValLabel");
		smoothnessVolLab_ = new QLabel();
		smoothnessVolLab_->setFixedWidth(44);
	    smoothnessVolLab_->setFixedHeight(16);
		smoothnessVolLab_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		smoothnessVolLab_->setObjectName("OpacityValLabel");
		spillVolLab_ = new QLabel();
		spillVolLab_->setFixedWidth(44);
	    spillVolLab_->setFixedHeight(16);
		spillVolLab_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		spillVolLab_->setObjectName("OpacityValLabel");

		auto colorHLayout = new QHBoxLayout();
		colorHLayout->setSpacing(0);
		colorHLayout->setContentsMargins(0, 0, 0, 0);
		colorHLayout->addWidget(colorLab_);
		auto *colorSpacer0 = new QSpacerItem(4, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
		auto *colorSpacer1 = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed);
		colorHLayout->addItem(colorSpacer0);
		colorHLayout->addWidget(colorChangeBtn_);
		colorHLayout->addItem(colorSpacer1);

		QSpacerItem *spacer0 = new QSpacerItem(2, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		QSpacerItem *spacer1 = new QSpacerItem(2, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		QSpacerItem *spacer2 = new QSpacerItem(2, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		auto opacityHLayout = new QHBoxLayout();
		opacityHLayout->addWidget(opacityLab_);
		opacityHLayout->addItem(spacer0);
		opacityHLayout->addWidget(opacitySlider_);
		opacityHLayout->addItem(spacer1);
		opacityHLayout->addWidget(opacityVolLab_);
		opacityHLayout->addItem(spacer2);
		opacityHLayout->setSpacing(0);
		opacityHLayout->setContentsMargins(1, 1, 0, 0);

		QSpacerItem *spacer3 = new QSpacerItem(4, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		QSpacerItem *spacer4 = new QSpacerItem(4, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		QSpacerItem *spacer5 = new QSpacerItem(4, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		auto similarityHLayout = new QHBoxLayout();
		similarityHLayout->addWidget(similarityLab_);
		similarityHLayout->addItem(spacer3);
		similarityHLayout->addWidget(similaritySlider_);
		similarityHLayout->addItem(spacer4);
		similarityHLayout->addWidget(similarityVolLab_);
		similarityHLayout->addItem(spacer5);
		similarityHLayout->setSpacing(0);
		similarityHLayout->setContentsMargins(1, 1, 0, 0);

		QSpacerItem *spacer7 = new QSpacerItem(4, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		QSpacerItem *spacer8 = new QSpacerItem(4, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		QSpacerItem *spacer9 = new QSpacerItem(4, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		auto smoothnessHLayout = new QHBoxLayout();
		smoothnessHLayout->addWidget(smoothnessLab_);
		smoothnessHLayout->addItem(spacer7);
		smoothnessHLayout->addWidget(smoothnessSlider_);
		smoothnessHLayout->addItem(spacer8);
		smoothnessHLayout->addWidget(smoothnessVolLab_);
		smoothnessHLayout->addItem(spacer9);
		smoothnessHLayout->setSpacing(0);
		smoothnessHLayout->setContentsMargins(1, 1, 0, 0);

		QSpacerItem *spacer10 = new QSpacerItem(2, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		QSpacerItem *spacer11 = new QSpacerItem(2, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		QSpacerItem *spacer12 = new QSpacerItem(2, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		auto spillHLayout = new QHBoxLayout();
		spillHLayout->addWidget(spillLab_);
		spillHLayout->addItem(spacer10);
		spillHLayout->addWidget(spillSlider_);
		spillHLayout->addItem(spacer11);
		spillHLayout->addWidget(spillVolLab_);
		spillHLayout->addItem(spacer12);
		spillHLayout->setSpacing(0);
		spillHLayout->setContentsMargins(1, 1, 0, 0);

		chromaKeyVLayout_ = new QVBoxLayout();
		QSpacerItem *spacer_v0 = new QSpacerItem(4, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		QSpacerItem *spacer_v1 = new QSpacerItem(4, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		QSpacerItem *spacer_v2 = new QSpacerItem(4, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		QSpacerItem *spacer_v3 = new QSpacerItem(4, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
		QSpacerItem *spacer_v4 = new QSpacerItem(4, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);

		chromaKeyCtrlWid_ = new QWidget(ui.PropertyWid);
		chromaKeyVLayout_->addLayout(colorHLayout);
		chromaKeyVLayout_->addItem(spacer_v4);
		chromaKeyVLayout_->addLayout(similarityHLayout);
		chromaKeyVLayout_->addItem(spacer_v0);
		chromaKeyVLayout_->addLayout(smoothnessHLayout);
		chromaKeyVLayout_->addItem(spacer_v1);
		chromaKeyVLayout_->addLayout(spillHLayout);
		chromaKeyVLayout_->addItem(spacer_v2);
		chromaKeyVLayout_->addLayout(opacityHLayout);
		chromaKeyCtrlWid_->setLayout(chromaKeyVLayout_);

		MainVLayout->insertWidget(9, chromaKeyCtrlWid_);

		similaritySlider_->setObjectName(QString("similarity"));
		smoothnessSlider_->setObjectName(QString("smoothness"));
		spillSlider_->setObjectName(QString("spill"));
		opacitySlider_->setObjectName(QString("opacity"));

		connect(colorChangeBtn_, &QPushButton::clicked, this, &BiLiDShowSourcePropertyDlg::sltColorChangeBtn_);
		connect(similaritySlider_, &CircleSliderSlider::valueChanged, this, &BiLiDShowSourcePropertyDlg::sltSliderChanged_);
		connect(smoothnessSlider_, &CircleSliderSlider::valueChanged, this, &BiLiDShowSourcePropertyDlg::sltSliderChanged_);
		connect(spillSlider_, &CircleSliderSlider::valueChanged, this, &BiLiDShowSourcePropertyDlg::sltSliderChanged_);
		connect(opacitySlider_, &CircleSliderSlider::valueChanged, this, &BiLiDShowSourcePropertyDlg::sltSliderChanged_);

		hasCreateSettingCtrls_ = true;
	}
}