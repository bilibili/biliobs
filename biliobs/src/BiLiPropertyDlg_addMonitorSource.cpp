#define DEFINE_IMPL_PROPDLG

#include "BiLiPropertyDlg.h"
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>

#include "BiLiApp.h"
#include "BiLiOBSMainWid.h"
#include "bili_area_cap.h"

#include <regex>

IMPL_PROPDLG(BiLiMonitorSourcePropertyDlg);
QComboBox* MonitorNameComboBox;
QCheckBox* MouseCapCheckBox;
QCheckBox* ScreenShotCheckBox;

void OnScreenShotCheckBoxChanged(int state);
void OnScreenShotCompelte(bool hasSelect);
END_IMPL_PROPDLG(BiLiMonitorSourcePropertyDlg, "monitor_capture");

BiLiMonitorSourcePropertyDlg::~BiLiMonitorSourcePropertyDlg() {}

void BiLiMonitorSourcePropertyDlg::setupSourcePropertiesUI() {
	//注意：添加控件时，记得根据需要在最后添加控件的变动通知监视！
	//否则可能导致点了确定之后设置没有保存进去

	ui.PropertyNameLab->setText(tr("Monitor Capture"));

	auto MonitorHandleHLayout = new QHBoxLayout();
	MonitorNameComboBox = new QComboBox(ui.PropertyWid);
	MonitorNameComboBox->setObjectName(QStringLiteral("MonitorNameComboBox"));
	MonitorNameComboBox->setMinimumSize(QSize(0, 30));
	MonitorNameComboBox->setMaximumSize(QSize(16777215, 30));
	MonitorHandleHLayout->addWidget(MonitorNameComboBox);

    QSpacerItem *MonitorHandleHLayout_spacer = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    MonitorHandleHLayout->addItem(MonitorHandleHLayout_spacer);

	auto BrushBtn = new QPushButton(ui.PropertyWid);
	BrushBtn->setObjectName(QStringLiteral("BrushBtn"));
	BrushBtn->setMinimumSize(QSize(80, 30));
	BrushBtn->setMaximumSize(QSize(80, 30));
	MonitorHandleHLayout->addWidget(BrushBtn);
	MonitorHandleHLayout->setSpacing(0);

	ScreenShotCheckBox = new QCheckBox(ui.PropertyWid);
    ScreenShotCheckBox->setFixedHeight(13);
    ScreenShotCheckBox->setObjectName(QStringLiteral("ScreenShotCheckBox"));


	MouseCapCheckBox = new QCheckBox(ui.PropertyWid);
    MouseCapCheckBox->setFixedHeight(13);
	MouseCapCheckBox->setObjectName(QStringLiteral("MouseCapCheckBox"));

    QSpacerItem* CapMidSpacer = new QSpacerItem(30, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    auto CapOptSpacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);

	auto CapOptHLaout = new QHBoxLayout();
	CapOptHLaout->addWidget(ScreenShotCheckBox);
    CapOptHLaout->addItem(CapMidSpacer);
	CapOptHLaout->addWidget(MouseCapCheckBox);
	CapOptHLaout->addItem(CapOptSpacer);

	auto PropertyTipGroupBox = new QGroupBox(ui.PropertyWid);
	PropertyTipGroupBox->setObjectName(QStringLiteral("PropertyTipGroupBox"));
	QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
	sizePolicy1.setHorizontalStretch(0);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(PropertyTipGroupBox->sizePolicy().hasHeightForWidth());
	PropertyTipGroupBox->setSizePolicy(sizePolicy1);
	PropertyTipGroupBox->setMinimumHeight(76);
	PropertyTipGroupBox->setMaximumHeight(76);

	auto PropertyTip = new QLabel(PropertyTipGroupBox);
	PropertyTip->setObjectName(QStringLiteral("PropertyTip"));
    PropertyTip->setGeometry(17, 25, 28, 28);
	PropertyTip->setStyleSheet(QStringLiteral(""));

	auto PropertyTipTxt = new QLabel(PropertyTipGroupBox);
	PropertyTipTxt->setObjectName(QStringLiteral("PropertyTipTxt"));
	PropertyTipTxt->setFixedSize(270, 55);
    PropertyTipTxt->move(55, 10);
	PropertyTipTxt->setAlignment(Qt::AlignVCenter  | Qt::AlignLeading | Qt::AlignLeft);
	PropertyTipTxt->setWordWrap(true);
	PropertyTipTxt->setStyleSheet(QLatin1String("\n"
		"QLabel#PropertyTipTxt{\n"
		"    font-size: 10pt;\n"
		"    color: #8FC5E3;\n"
		"}\n" ));

	auto MainVLayout = new QVBoxLayout(ui.PropertyWid);
    MainVLayout->setSpacing(0);
	MainVLayout->setContentsMargins(13, 20, 13, 30);
    auto VSpacerDownCombox = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *VSpacerDownCheckBtn = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
	MainVLayout->addLayout(MonitorHandleHLayout);
	MainVLayout->addItem(VSpacerDownCombox);
	MainVLayout->addLayout(CapOptHLaout);
    MainVLayout->addItem(VSpacerDownCheckBtn);
	MainVLayout->addWidget(PropertyTipGroupBox);
	MainVLayout->setSpacing(0);

	BrushBtn->setText(QApplication::translate("MonitorAddForm", "Refresh", 0));
	PropertyTipTxt->setText(QApplication::translate("MonitorAddFormForm", "Unless you really want to record the entire desktop or use the \"Record game\" so the performance will be much better", 0));
	ScreenShotCheckBox->setText(QApplication::translate("MonitorAddForm", "ScreenShot", 0));
	MouseCapCheckBox->setText(QApplication::translate("MonitorAddForm", "MouseCap", 0));
	//QObject::connect(ScreenShotCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(mSglScreenShotState(int)));
	this->setProperty("ScreenShotCB", qVPtr<QCheckBox>::toVariant(ScreenShotCheckBox));

	//截屏勾选时的行为
	QObject::connect(ScreenShotCheckBox, &QCheckBox::stateChanged, this, &BiLiMonitorSourcePropertyDlg::OnScreenShotCheckBoxChanged);
	QMetaObject::connectSlotsByName(ui.PropertyWid);

	obs_properties_t* props = obs_source_properties(mSrc);
	DataToWidget(BILI_PROP_LIST_INT(), MonitorNameComboBox, props, "monitor");
	obs_properties_destroy(props);

	obs_data_t* settings = obs_source_get_settings(mSrc);
	DataToWidget(BILI_DATA_INT(), MonitorNameComboBox, settings, "monitor");
	DataToWidget(BILI_DATA_BOOL(), MouseCapCheckBox, settings, "cursor");
	DataToWidget(BILI_DATA_BOOL(), ScreenShotCheckBox, settings, "is_limit_rect");
	obs_data_release(settings);

	//添加监听控件变动
	mChangeEvnetFilter->Watch({ MonitorNameComboBox, MouseCapCheckBox, ScreenShotCheckBox });
	connect(mChangeEvnetFilter.get(), SIGNAL(OnChangedSignal()), this, SLOT(mSltOnSettingChanged()));
}

int BiLiMonitorSourcePropertyDlg::acceptSourceProperties() {
	obs_data_t* settings = obs_source_get_settings(mSrc);
	WidgetToData(BILI_DATA_INT(), MonitorNameComboBox, settings, "monitor");
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
	}
	obs_source_update(mSrc, settings);
	obs_data_release(settings);

	return (QDialog::Accepted);
}

void BiLiMonitorSourcePropertyDlg::OnScreenShotCheckBoxChanged(int state)
{
	emit mSglScreenShotState(state);

	if (state == Qt::Checked)
	{
		//监视屏幕截取操作，自动切换显示器
		QObject::connect(App()->mGetMainWindow()->mAreaCap, &bili_area_cap::mSglSelectComplite, this, &BiLiMonitorSourcePropertyDlg::OnScreenShotCompelte);
	}
}

void BiLiMonitorSourcePropertyDlg::OnScreenShotCompelte(bool hasSelect)
{
	if (hasSelect)
	{
		auto areaCap = App()->mGetMainWindow()->mAreaCap;
		if (areaCap == nullptr)
			return;

		std::regex resolutionRegex("[^@]*@ (-?\\d+,-?\\d+)( \\([^\\)]*\\))?$");

		//获取当前选取区域所属的显示器的左上角绝对坐标
		QPoint orgin = areaCap->mSelectedWidRectOrgin;
		//遍历显示器列表
		int itemCount = MonitorNameComboBox->count();
		for (int i = 0; i < itemCount; ++i)
		{
			//获取列表中某一项所表示的显示器的坐标位置
			std::string itemText = MonitorNameComboBox->itemText(i).toUtf8().constData();
			std::smatch resolutionMatch;
			if (std::regex_match(itemText, resolutionMatch, resolutionRegex))
			{
				int posX, posY;
				if (sscanf(resolutionMatch[1].str().c_str(), "%d,%d", &posX, &posY) == 2)
				{
					//判断坐标位置是不是目标显示器
					if (posX == orgin.x() && posY == orgin.y())
					{
						MonitorNameComboBox->setCurrentIndex(i);
						break;
					}
				}
			}
		}

		mSltOnSettingChanged();
	}
}
