#define DEFINE_IMPL_PROPDLG

#include "BiLiPropertyDlg.h"
#include <QGroupBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileInfo>

IMPL_PROPDLG(BiLiMediaSourcePropertyDlg);
END_IMPL_PROPDLG(BiLiMediaSourcePropertyDlg, "ffmpeg_source");

BiLiMediaSourcePropertyDlg::~BiLiMediaSourcePropertyDlg() {}

void BiLiMediaSourcePropertyDlg::setupSourcePropertiesUI() {
	//注意：添加控件时，记得根据需要在最后添加控件的变动通知监视！
	//否则可能导致点了确定之后设置没有保存进去

	ui.PropertyNameLab->setText(tr("Media Property"));


	auto ComboxBtnHLayout = new QHBoxLayout();
	auto MediaNameLineEdit = new QLineEdit(ui.PropertyWid);
	MediaNameLineEdit->setMinimumSize(QSize(0, 30));
	MediaNameLineEdit->setMaximumSize(QSize(16777215, 30));
	ComboxBtnHLayout->addWidget(MediaNameLineEdit);

	QSpacerItem *spacer_h0 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
	ComboxBtnHLayout->addItem(spacer_h0);

	auto BrowserBtn = new QPushButton(ui.PropertyWid);
	BrowserBtn->setObjectName(QStringLiteral("BrowserBtn"));
	BrowserBtn->setMinimumSize(QSize(80, 30));
	BrowserBtn->setMaximumSize(QSize(80, 30));
	ComboxBtnHLayout->addWidget(BrowserBtn);



	/** begin
	 * 提示框
	 */
	QGroupBox *PropertyTipGroupBox = new QGroupBox();
	PropertyTipGroupBox->setObjectName(QStringLiteral("PropertyTipGroupBox"));
	QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
	sizePolicy1.setHorizontalStretch(0);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(PropertyTipGroupBox->sizePolicy().hasHeightForWidth());
	PropertyTipGroupBox->setSizePolicy(sizePolicy1);
	PropertyTipGroupBox->setMinimumHeight(60);
	PropertyTipGroupBox->setMaximumHeight(60);


	QLabel *PropertyTipIcon = new QLabel(PropertyTipGroupBox);
	PropertyTipIcon->setObjectName(QStringLiteral("PropertyTip"));
	PropertyTipIcon->setGeometry(QRect(16, 16, 28, 28));
	PropertyTipIcon->setStyleSheet(QStringLiteral(""));

	QLabel *PropertyTipTxt = new QLabel(PropertyTipGroupBox);
	PropertyTipTxt->setObjectName(QStringLiteral("PropertyTipTxt"));
	PropertyTipTxt->setMinimumHeight(35);
	PropertyTipTxt->setMaximumHeight(35);
    PropertyTipTxt->setFixedWidth(270);
    PropertyTipTxt->move(55, 12);
    PropertyTipTxt->setAlignment(Qt::AlignTop | Qt::AlignLeading | Qt::AlignLeft);
	PropertyTipTxt->setWordWrap(true);
	PropertyTipTxt->setStyleSheet(QLatin1String("\n"
		"QLabel#PropertyTipTxt{\n"
		"    font-size: 10pt;\n"
		"    color: #8FC5E3;\n"
		"}\n"));
	PropertyTipTxt->setText(QApplication::translate("MediaAddForm", "The state is temporarily unable to preview video preview source volume. Please live test the actual effect on the studio page.", 0));


	mFileNameEdit = MediaNameLineEdit;

	/** end
	* 提示框
	*/

	auto MainVLayout = new QVBoxLayout(ui.PropertyWid);
	MainVLayout->setSpacing(0);
	MainVLayout->setContentsMargins(13, 18, 13, 30);


    //ui.PropertyWid->setFixedHeight(146);
	QSpacerItem *space_v1 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
	MainVLayout->addLayout(ComboxBtnHLayout);
	MainVLayout->addItem(space_v1);
	MainVLayout->addWidget(PropertyTipGroupBox);

	

	BrowserBtn->setText(QApplication::translate("MediaAddForm", "Browse", 0));

	connect(BrowserBtn, SIGNAL(clicked()), this, SLOT(mSltBrowserBtn()));

	QMetaObject::connectSlotsByName(ui.PropertyWid);

	obs_data_t *settingDefaultValue = obs_data_create();
	obs_data_set_bool(settingDefaultValue, "looping", true);
	obs_source_update(mSrc, settingDefaultValue);
	obs_data_release(settingDefaultValue);

	obs_data_t* settings = obs_source_get_settings(mSrc);
	if (obs_data_get_bool(settings, "is_local_file") == true)
		DataToWidget(BILI_DATA_STRING(), mFileNameEdit, settings, "local_file");
	else
		DataToWidget(BILI_DATA_STRING(), mFileNameEdit, settings, "input");
	obs_data_release(settings);

	//添加监听控件变动
	mChangeEvnetFilter->Watch({ MediaNameLineEdit });
}

int BiLiMediaSourcePropertyDlg::acceptSourceProperties() {

	QString filePath = mFileNameEdit->text();
	if (filePath.isEmpty())
		return QDialog::Rejected;

	//判断是否需要回复原比例和更新
	std::string oldFileName = obs_data_get_string(mBackupSettings, "local_file");
	if (filePath != oldFileName.c_str())
	{
		QString protoFile = "file://";
		QString protoNet = "://";

		obs_data_t* settings = obs_source_get_settings(mSrc);

		if (filePath.contains(protoNet) && !filePath.contains(protoFile))
		{
			obs_data_set_bool(settings, "is_local_file", false);
			WidgetToData(BILI_DATA_STRING(), mFileNameEdit, settings, "input");
		}
		else
		{
			obs_data_set_bool(settings, "is_local_file", true);
			WidgetToData(BILI_DATA_STRING(), mFileNameEdit, settings, "local_file");
		}

		obs_source_update(mSrc, settings);
		obs_data_release(settings);

		vec2 oneScale;
		oneScale.x = oneScale.y = 1.0f;
		obs_sceneitem_set_scale(mSceneItem, &oneScale);
	}

	return (QDialog::Accepted);
}
