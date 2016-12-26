#define DEFINE_IMPL_PROPDLG

#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <Dwmapi.h>

#include "BiLiApp.h"
#include "BiLiOBSMainWid.h"
#include "bili_area_cap.h"
#include "BiLiPropertyDlg.h"

extern "C" {
#define class xClass
#include "../plugins/win-capture/window-helpers.c"
#include "../plugins/win-capture/obfuscate.c"
#undef class
};

#pragma comment(lib, "dwmapi.lib")

IMPL_PROPDLG(BiLiWindowSourcePropertyDlg);
QComboBox* WindowNameComboBox;
QCheckBox* MouseCapCheckBox;
QCheckBox* ScreenShotCheckBox;

void mSltRefreshWindowList();
void mOnScreenShotCheckBoxChanged(int state);
void mOnScreenShotComplete(bool hasSelect);
END_IMPL_PROPDLG(BiLiWindowSourcePropertyDlg, "window_capture");

BiLiWindowSourcePropertyDlg::~BiLiWindowSourcePropertyDlg() {}

void BiLiWindowSourcePropertyDlg::setupSourcePropertiesUI() {
	//注意：添加控件时，记得根据需要在最后添加控件的变动通知监视！
	//否则可能导致点了确定之后设置没有保存进去

	ui.PropertyNameLab->setText(tr("Window Property")); 

	auto propertyHandleHLayout = new QHBoxLayout();
	WindowNameComboBox = new QComboBox(ui.PropertyWid);
	WindowNameComboBox->setObjectName(QStringLiteral("WindowNameComboBox"));
	WindowNameComboBox->setMinimumSize(QSize(95, 28));
	WindowNameComboBox->setMaximumSize(QSize(16777215, 28));
	propertyHandleHLayout->addWidget(WindowNameComboBox);

	QSpacerItem *spacer_h0 = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
	propertyHandleHLayout->addItem(spacer_h0);

	auto BrushBtn = new QPushButton(ui.PropertyWid);
	BrushBtn->setObjectName(QStringLiteral("BrushBtn"));
	QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(BrushBtn->sizePolicy().hasHeightForWidth());
	BrushBtn->setSizePolicy(sizePolicy);
	BrushBtn->setMinimumSize(QSize(70, 28));
	BrushBtn->setMaximumSize(QSize(70, 28));
	propertyHandleHLayout->addWidget(BrushBtn);

	auto aeroBtn = new QPushButton(ui.PropertyWid);
	aeroBtn->setObjectName(QStringLiteral("AeroBtn"));
	QSizePolicy aeroBtnSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	aeroBtnSizePolicy.setHorizontalStretch(0);
	aeroBtnSizePolicy.setVerticalStretch(0);
	aeroBtnSizePolicy.setHeightForWidth(aeroBtn->sizePolicy().hasHeightForWidth());
	aeroBtn->setSizePolicy(sizePolicy);
	aeroBtn->setMinimumSize(QSize(70, 20));
	aeroBtn->setMaximumSize(QSize(70, 20));
	QObject::connect(aeroBtn, &QPushButton::clicked, [this, aeroBtn](){

		BOOL enableAero = true;
		DwmIsCompositionEnabled(&enableAero);
		if (enableAero) { 
			DwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
			aeroBtn->setText(QApplication::translate("WindowAddForm", "Enable Aero", 0));
		}
		else{
            DwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
			aeroBtn->setText(QApplication::translate("WindowAddForm", "Disable Aero", 0));
		}
	});

	ScreenShotCheckBox = new QCheckBox(ui.PropertyWid);
    ScreenShotCheckBox->setFixedHeight(13);
	MouseCapCheckBox = new QCheckBox(ui.PropertyWid);
    MouseCapCheckBox->setFixedHeight(13);
    auto CapOptSpacer0 = new QSpacerItem(30, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    auto CapOptSpacer1 = new QSpacerItem(20, 10, QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto CapOptSpacer2 = new QSpacerItem(22, 10, QSizePolicy::Expanding, QSizePolicy::Fixed);
	ScreenShotCheckBox->setObjectName(QStringLiteral("ScreenShotCheckBox"));
	MouseCapCheckBox->setObjectName(QStringLiteral("MouseCapCheckBox"));
	auto CapOptHLaout = new QHBoxLayout();
	CapOptHLaout->addWidget(ScreenShotCheckBox);
    CapOptHLaout->addItem(CapOptSpacer0);
	CapOptHLaout->addWidget(MouseCapCheckBox);
	CapOptHLaout->addItem(CapOptSpacer1);
	CapOptHLaout->addWidget(aeroBtn);
	CapOptHLaout->addItem(CapOptSpacer2);

	auto PropertyTipGroupBox = new QGroupBox(ui.PropertyWid);
	PropertyTipGroupBox->setObjectName(QStringLiteral("PropertyTipGroupBox"));
	PropertyTipGroupBox->setMinimumHeight(70);
	PropertyTipGroupBox->setMaximumHeight(70);

	auto PropertyTip = new QLabel(PropertyTipGroupBox);
	PropertyTip->setObjectName(QStringLiteral("PropertyTip"));
    PropertyTip->move(14, 21);
	PropertyTip->setMinimumSize(QSize(28, 28));
	PropertyTip->setMaximumSize(QSize(28, 28));

	auto PropertyTipTxt = new QLabel(PropertyTipGroupBox);
	PropertyTipTxt->setObjectName(QStringLiteral("PropertyTipTxt"));
	PropertyTipTxt->setWordWrap(true);
	PropertyTipTxt->setStyleSheet(QLatin1String("\n"
		"QLabel#PropertyTipTxt{\n"
		"    font-size: 10pt;\n"
		"    color: #8FC5E3;\n"
		"}\n" ));

    PropertyTipTxt->setGeometry(56, 14, 270, 50);

	PropertyTipTxt->setAlignment(Qt::AlignVCenter  | Qt::AlignLeading | Qt::AlignLeft);
	PropertyTipTxt->setWordWrap(true);

	auto mainVLayout = new QVBoxLayout(ui.PropertyWid);
    mainVLayout->setSpacing(0);
	QSpacerItem *spacer_v0 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *spacer_v1 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);

	mainVLayout->addLayout(propertyHandleHLayout);
	mainVLayout->addItem(spacer_v0);
	mainVLayout->addLayout(CapOptHLaout);
    mainVLayout->addItem(spacer_v1);
	mainVLayout->addWidget(PropertyTipGroupBox);
	mainVLayout->setContentsMargins(13, 20, 13, 30);

	BrushBtn->setText(QApplication::translate("WindowAddForm", "Refresh", 0));
	PropertyTipTxt->setText(QApplication::translate("WindowAddForm", "Please make sure the window for added is not minimized", 0));
	ScreenShotCheckBox->setText(QApplication::translate("WindowAddForm", "ScreenShot", 0));
	MouseCapCheckBox->setText(QApplication::translate("WindowAddForm", "MouseCap", 0));

	BOOL enableAero = true;
	DwmIsCompositionEnabled(&enableAero);
	if (enableAero)
		aeroBtn->setText(QApplication::translate("WindowAddForm", "Disable Aero", 0));
	else
		aeroBtn->setText(QApplication::translate("WindowAddForm", "Enable Aero", 0));
	this->setProperty("ScreenShotCB", qVPtr<QCheckBox>::toVariant(ScreenShotCheckBox));
	QMetaObject::connectSlotsByName(ui.PropertyWid);
	QObject::connect(ScreenShotCheckBox, &QCheckBox::stateChanged, this, &BiLiWindowSourcePropertyDlg::mOnScreenShotCheckBoxChanged);
	QObject::connect(BrushBtn, &QPushButton::clicked, this, &BiLiWindowSourcePropertyDlg::mSltRefreshWindowList);

	mSltRefreshWindowList();

	obs_data_t* settings = obs_source_get_settings(mSrc);
	DataToWidget(BILI_DATA_STRING(), WindowNameComboBox, settings, "window");
	DataToWidget(BILI_DATA_BOOL(), MouseCapCheckBox, settings, "cursor");
	DataToWidget(BILI_DATA_BOOL(), ScreenShotCheckBox, settings, "is_limit_rect");
	obs_data_release(settings);

	//添加监听控件变动
	mChangeEvnetFilter->Watch({ WindowNameComboBox, MouseCapCheckBox, ScreenShotCheckBox });
	connect(mChangeEvnetFilter.get(), SIGNAL(OnChangedSignal()), this, SLOT(mSltOnSettingChanged()));
}

int BiLiWindowSourcePropertyDlg::acceptSourceProperties() {
	obs_data_t* settings = obs_source_get_settings(mSrc);
	WidgetToData(BILI_DATA_STRING(), WindowNameComboBox, settings, "window");
	WidgetToData(BILI_DATA_BOOL(), MouseCapCheckBox, settings, "cursor");
	WidgetToData(BILI_DATA_BOOL(), ScreenShotCheckBox, settings, "is_limit_rect");
	if (IsLimitRectSelected())
	{
		auto cap = App()->mGetMainWindow()->mAreaCap;
		QRect rect = cap->mSelectedWidRect;
		rect.moveTo(rect.topLeft() + cap->mSelectedWidRectOrgin);
		obs_data_set_int(settings, "limit_x", rect.x());
		obs_data_set_int(settings, "limit_y", rect.y());
		obs_data_set_int(settings, "limit_cx", rect.width());
		obs_data_set_int(settings, "limit_cy", rect.height());
		obs_data_set_bool(settings, "is_screen_rect", true);
	}
	obs_source_update(mSrc, settings);
	obs_data_release(settings);

	return (QDialog::Accepted);
}

void BiLiWindowSourcePropertyDlg::mSltRefreshWindowList()
{
	obs_properties_t* props = obs_source_properties(mSrc);
	DataToWidget(BILI_PROP_LIST_STRING(), WindowNameComboBox, props, "window");
	obs_properties_destroy(props);
}

void BiLiWindowSourcePropertyDlg::mOnScreenShotCheckBoxChanged(int state)
{
	emit mSglScreenShotState(state);

	if (state == Qt::Checked)
	{
		//自动选择窗口
		QObject::connect(App()->mGetMainWindow()->mAreaCap, &bili_area_cap::mSglSelectComplite, this, &BiLiWindowSourcePropertyDlg::mOnScreenShotComplete);
	}
}

void BiLiWindowSourcePropertyDlg::mOnScreenShotComplete(bool hasSelect)
{
	if (hasSelect)
	{
		HWND hwnd = App()->mGetMainWindow()->mAreaCap->mGuessSelectedWindow();
		if (!hwnd)
			return;

		int wndCnt = WindowNameComboBox->count();
		if (!wndCnt)
			return;

		obs_data_t* srcSettings = obs_source_get_settings(mSrc);
		window_priority priority = (window_priority)obs_data_get_int(srcSettings, "priority");
		obs_data_release(srcSettings);

		//匹配窗口列表
		bool exitLoopFlag = false;
		for (int i = 0; i < wndCnt && exitLoopFlag == false; ++i)
		{
			QString curStr = WindowNameComboBox->itemData(i).toString();
			char* cls;
			char* title;
			char* exe;
			build_window_strings(curStr.toUtf8().data(), &cls, &title, &exe);

			if (hwnd == find_window(EXCLUDE_MINIMIZED, (window_priority)priority, cls, title, exe))
			{
				WindowNameComboBox->setCurrentIndex(i);
				exitLoopFlag = true;
			}

			bfree(cls);
			bfree(title);
			bfree(exe);
		}

		mSltOnSettingChanged();
	}
}
