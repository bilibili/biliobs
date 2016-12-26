#define DEFINE_IMPL_PROPDLG

#include "BiLiPropertyDlg.h"
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileInfo>
#include "BiLiMsgDlg.h"

#include "circle_slider_slider.h"

#include "system_ret_info_dlg.h"

IMPL_PROPDLG(BiLiImageSourcePropertyDlg);
CircleSliderSlider* OpacitySlider;
QLabel* OpacityValLabel;
obs_source_t* chromaKeyFilter;

void mAddGifToMedia(QString gifPathStr);
void OnOpacitySliderChanged(int val);
END_IMPL_PROPDLG(BiLiImageSourcePropertyDlg, "image_source");

BiLiImageSourcePropertyDlg::~BiLiImageSourcePropertyDlg() {}

static const char* chromeKeyFilterId = "chroma_key_filter";

void BiLiImageSourcePropertyDlg::setupSourcePropertiesUI() {
	//注意：添加控件时，记得根据需要在最后添加控件的变动通知监视！
	//否则可能导致点了确定之后设置没有保存进去

	ui.PropertyNameLab->setText(tr("Image Property"));

	auto ComboxBtnHLayout = new QHBoxLayout();
	ComboxBtnHLayout->setSpacing(0);

	auto ImageNameLineEdit = new QLineEdit(ui.PropertyWid);
	ImageNameLineEdit->setMinimumSize(QSize(0, 30));
	ImageNameLineEdit->setMaximumSize(QSize(16777215, 30));
	ComboxBtnHLayout->addWidget(ImageNameLineEdit);

	QSpacerItem *spacer_h0 = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
	ComboxBtnHLayout->addItem(spacer_h0);

	auto BrowserBtn = new QPushButton(ui.PropertyWid);
	BrowserBtn->setObjectName(QStringLiteral("BrowserBtn"));
	BrowserBtn->setMinimumSize(QSize(80, 30));
	BrowserBtn->setMaximumSize(QSize(80, 30));
	ComboxBtnHLayout->addWidget(BrowserBtn);

	auto OpacitySliderHLayout = new QHBoxLayout();

    QSpacerItem *OpacityLayout_spacer0 = new QSpacerItem(9, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *OpacityLayout_spacer1 = new QSpacerItem(4, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *OpacityLayout_spacer2 = new QSpacerItem(4, 10, QSizePolicy::Expanding, QSizePolicy::Fixed);
	auto OpacityLabel = new QLabel();
	OpacityLabel->setObjectName("OpacityLabel");
	OpacityLabel->setText(QApplication::translate("ImageAddForm", "Opacity :", 0));
    OpacityLabel->setFixedSize(59, 16);
    OpacityLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	OpacityValLabel = new QLabel();
	OpacityValLabel->setFixedWidth(40);
    OpacityValLabel->setFixedHeight(16);
	OpacityValLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	OpacityValLabel->setObjectName("OpacityValLabel");
	OpacitySlider = new CircleSliderSlider();
    OpacitySlider->setFixedWidth(150);
    OpacitySlider->setFixedHeight(16);
	//OpacitySlider->init();
	OpacitySlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	OpacitySliderHLayout->addWidget(OpacityLabel);
    OpacitySliderHLayout->addItem(OpacityLayout_spacer0);
	OpacitySliderHLayout->addWidget(OpacitySlider);
    OpacitySliderHLayout->addItem(OpacityLayout_spacer1);
	OpacitySliderHLayout->addWidget(OpacityValLabel);
    OpacitySliderHLayout->addItem(OpacityLayout_spacer2);
	OpacitySliderHLayout->setSpacing(0);
    OpacitySliderHLayout->setContentsMargins(0, 20, 0, 0);


	//添加滤镜
	chromaKeyFilter = obs_source_get_filter_by_name(mSrc, chromeKeyFilterId);
	if (!chromaKeyFilter)
	{
		chromaKeyFilter = obs_source_create(OBS_SOURCE_TYPE_FILTER, chromeKeyFilterId, chromeKeyFilterId, 0, 0);
		obs_source_filter_add(mSrc, chromaKeyFilter);

		obs_data_t *filterSettings = obs_source_get_settings(chromaKeyFilter);

		obs_data_set_int(filterSettings, "similarity", 1);
		obs_data_set_int(filterSettings, "smoothness", 1);
		obs_data_set_int(filterSettings, "spill", 1);
		obs_data_set_int(filterSettings, "opacity", 100);
		obs_source_update(chromaKeyFilter, filterSettings);
		obs_data_release(filterSettings);
	}

	//设置滑动条限制
	obs_properties_t* props = obs_source_properties(mSrc);
	if (props)
	{
		obs_property_t *prop = obs_properties_get(props, "opacity");
		if (prop)
		{
			int minVal = obs_property_int_min(prop);
			int maxVal = obs_property_int_max(prop);
			int stepVal = obs_property_int_step(prop);
			OpacitySlider->setRange(minVal, maxVal, maxVal);
		}
		obs_properties_destroy(props);
	}
	QObject::connect(OpacitySlider, &CircleSliderSlider::valueChanged, this, &BiLiImageSourcePropertyDlg::OnOpacitySliderChanged);

	auto MainVLayout = new QVBoxLayout(ui.PropertyWid);
	MainVLayout->setSpacing(0);
	MainVLayout->setContentsMargins(13, 20, 13, 30);
	MainVLayout->addLayout(ComboxBtnHLayout);
	MainVLayout->addLayout(OpacitySliderHLayout);
	mFileNameEdit = ImageNameLineEdit;

	BrowserBtn->setText(QApplication::translate("ImageAddForm", "Browse", 0));

	connect(BrowserBtn, SIGNAL(clicked()), this, SLOT(mSltBrowserBtn()));

	QMetaObject::connectSlotsByName(ui.PropertyWid);

	obs_data_t* settings = obs_source_get_settings(mSrc);
	DataToWidget(BILI_DATA_STRING(), mFileNameEdit, settings, "file");
	FilterDataToWidget(BILI_DATA_INT(), OpacitySlider, mSrc, chromeKeyFilterId, "opacity");
	obs_data_release(settings);

	OpacityValLabel->setText(QString("%1%").arg(OpacitySlider->value()));

	//添加监听控件变动
	mChangeEvnetFilter->Watch({ ImageNameLineEdit, OpacitySlider });
}

int BiLiImageSourcePropertyDlg::acceptSourceProperties() {
	QString filePath = mFileNameEdit->text();
	if (filePath.isEmpty())
		return QDialog::Rejected;

	QFileInfo picFileInfo(filePath);

	obs_data_t* settings = obs_source_get_settings(mSrc);
	WidgetToData(BILI_DATA_STRING(), mFileNameEdit, settings, "file");
	WidgetToFilterData(BILI_DATA_INT(), OpacitySlider, mSrc, chromeKeyFilterId, "opacity");
	obs_source_update(mSrc, settings);
	obs_data_release(settings);

	//图片如果动了，因为图片是允许不按照比例缩放的，所以就直接恢复成和来源一样的大小
	std::string oldFileName = obs_data_get_string(mBackupSettings, "file");
	if (filePath != oldFileName.c_str()) {
		vec2 oneScale;
		oneScale.x = oneScale.y = 1.0f;
		obs_sceneitem_set_scale(mSceneItem, &oneScale);
	}

	return (QDialog::Accepted);
}

void BiLiImageSourcePropertyDlg::mAddGifToMedia(QString gifPathStr)
{

	if (!mCheckSourceNameLegal()) {
        SystemRetInfoDlg errDlg;
        errDlg.setDetailInfo(tr("Duplicated name!"));
        errDlg.setSubTitle(tr("Error"));
        errDlg.exec();
		return;
	}

	obs_source_t *sceneSource = obs_get_output_source(0);
	if (!sceneSource)
		return;
	obs_scene_t *scene = obs_scene_from_source(sceneSource); //不会添加scene或者sceneSource的引用
	if (!scene)
		return;

	obs_source_t *newSource = obs_source_create(OBS_SOURCE_TYPE_INPUT,
		"ffmpeg_source", mSourceName.toUtf8(), NULL, nullptr);
	
	obs_add_source(newSource);
	obs_sceneitem_t* sceneItem = obs_scene_add(scene, newSource);

	obs_data_t *settingDefaultValue = obs_data_create();
	obs_data_set_bool(settingDefaultValue, "is_local_file", true);
	obs_data_set_bool(settingDefaultValue, "looping", true);
	obs_source_update(newSource, settingDefaultValue);
	obs_data_release(settingDefaultValue);

	obs_data_t *settings = obs_source_get_settings(newSource);
	obs_data_set_string(settings, "local_file", gifPathStr.toUtf8().data());
	obs_source_update(newSource, settings);
	obs_data_release(settings);

	obs_source_release(newSource);
	obs_source_release(sceneSource); //在obs_scene_release里面会把source给释放
}

void BiLiImageSourcePropertyDlg::OnOpacitySliderChanged(int val)
{
	obs_data_t* filterSettings = obs_source_get_settings(chromaKeyFilter);
	obs_data_set_int(filterSettings, "opacity", val);
	obs_source_update(chromaKeyFilter, filterSettings);
	obs_data_release(filterSettings);

	OpacityValLabel->setText(QString("%1%").arg(val));
}
