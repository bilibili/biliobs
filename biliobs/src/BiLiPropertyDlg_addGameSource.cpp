#define DEFINE_IMPL_PROPDLG

#include "BiLiPropertyDlg.h"
#include "BiLiApp.h"
#include "BiLiOBSMainWid.h"
#include "BiLiMsgDlg.h"
#include <QComboBox>
#include <QLabel>
#include <memory>
#include <QCheckBox>
#include <QGroupBox>

IMPL_PROPDLG(BiLiGameSourcePropertyDlg);
void mSltRefreshGameListAsync();
void mSltRefreshGameListAsync(std::function<void()>&& onCompleted);

QComboBox* GameNameComboBox;
QCheckBox* antiCheatCheckBox;

QPushButton* RefresnBtn;

std::shared_ptr<bool> isDlgClosed;

END_IMPL_PROPDLG(BiLiGameSourcePropertyDlg, "game_capture");

void BiLiGameSourcePropertyDlg::setupSourcePropertiesUI() {
	//注意：添加控件时，记得根据需要在最后添加控件的变动通知监视！
	//否则可能导致点了确定之后设置没有保存进去
	isDlgClosed.reset(new bool(false));

	ui.PropertyNameLab->setText(tr("Game Property"));
	auto Form = ui.PropertyWid;

	auto ComboxBtnHLayout = new QHBoxLayout();
    ComboxBtnHLayout->setContentsMargins(0, 0, 0, 0);
    ComboxBtnHLayout->setSpacing(0);

	GameNameComboBox = new QComboBox(Form);
	GameNameComboBox->setObjectName(QStringLiteral("GameNameComboBox"));
	GameNameComboBox->setMinimumSize(QSize(0, 30));
	GameNameComboBox->setMaximumSize(QSize(16777215, 30));
	ComboxBtnHLayout->addWidget(GameNameComboBox);

	QSpacerItem *spacer1 = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
	ComboxBtnHLayout->addItem(spacer1);

	RefresnBtn = new QPushButton(Form);
	RefresnBtn->setObjectName(QStringLiteral("BrushBtn"));
	RefresnBtn->setMinimumSize(QSize(80, 30));
	RefresnBtn->setMaximumSize(QSize(80, 30));
	ComboxBtnHLayout->addWidget(RefresnBtn);

	antiCheatCheckBox = new QCheckBox(Form);
	antiCheatCheckBox->setText(tr("anti-cheat"));
    antiCheatCheckBox->setFixedHeight(13);

	QHBoxLayout* antiCheatLayout = new QHBoxLayout();
	antiCheatLayout->addWidget(antiCheatCheckBox);
    antiCheatLayout->setContentsMargins(0, 0, 0, 0);
    antiCheatLayout->setSpacing(0);

	auto PropertyTipGroupBox = new QGroupBox(Form);
	PropertyTipGroupBox->setObjectName("PropertyTipGroupBox");
	PropertyTipGroupBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	QHBoxLayout* tipGroupBoxLayout = new QHBoxLayout();
	tipGroupBoxLayout->addWidget(PropertyTipGroupBox);

	QHBoxLayout* TipLayout = new QHBoxLayout(PropertyTipGroupBox);

	auto PropertyTip = new QLabel(PropertyTipGroupBox);
	PropertyTip->setObjectName("PropertyTip");
	PropertyTip->setFixedSize(28, 28);
	PropertyTip->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	PropertyTip->setAlignment(Qt::AlignVCenter);
	TipLayout->addWidget(PropertyTip);

	auto PropertyTipTxt = new QLabel(PropertyTipGroupBox);
	PropertyTipTxt->setObjectName("PropertyTipTxt");
	PropertyTipTxt->setWordWrap(true);
	PropertyTipTxt->setStyleSheet("\n"
		"QLabel#PropertyTipTxt{\n"
		"    font-size: 10pt;\n"
		"    color: #8FC5E3;\n"
		"}");
	PropertyTipTxt->setAlignment(Qt::AlignVCenter);
	PropertyTipTxt->setWordWrap(true);
	TipLayout->addWidget(PropertyTipTxt);

	PropertyTipTxt->setText(tr("Game run in the other graphic card will not be caputred."));

	auto MainVLayout = new QVBoxLayout(Form);
    QSpacerItem *MainVLayout_spacer0 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *MainVLayout_spacer1 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);

    MainVLayout->setSpacing(0);
	MainVLayout->setContentsMargins(13, 20, 13, 30);
	MainVLayout->addLayout(ComboxBtnHLayout);
    MainVLayout->addItem(MainVLayout_spacer0);
	MainVLayout->addLayout(antiCheatLayout);
    MainVLayout->addItem(MainVLayout_spacer1);
	MainVLayout->addLayout(tipGroupBoxLayout);

    RefresnBtn->setText(QApplication::translate("GameAddForm", "Refresh", 0));

	QMetaObject::connectSlotsByName(ui.PropertyWid);

	QObject::connect(RefresnBtn, &QPushButton::clicked, this, 
		static_cast<void (BiLiGameSourcePropertyDlg::*)()>(&BiLiGameSourcePropertyDlg::mSltRefreshGameListAsync)
		);

	OBSData settings = obs_source_get_settings(mSrc);

	std::shared_ptr<bool> refIsDlgClosed = isDlgClosed;
	mSltRefreshGameListAsync([settings, refIsDlgClosed, this]()->void{
		if (*refIsDlgClosed == false)
			DataToWidget(BILI_DATA_STRING(), GameNameComboBox, (obs_data_t*)settings, "window");
	});
	
	DataToWidget(BILI_DATA_BOOL(), antiCheatCheckBox, (obs_data_t*)settings, "anti_cheat_hook");
	obs_data_release(settings);

	obs_data_t* setCaptureAnyFullscreen = obs_data_create();
	obs_data_set_bool(setCaptureAnyFullscreen, "capture_any_fullscreen", false);
	obs_source_update(mSrc, setCaptureAnyFullscreen);
	obs_data_release(setCaptureAnyFullscreen);

	//添加监听控件变动
	mChangeEvnetFilter->Watch({ GameNameComboBox, antiCheatCheckBox });
	connect(mChangeEvnetFilter.get(), SIGNAL(OnChangedSignal()), this, SLOT(mSltOnSettingChanged()));
}

int BiLiGameSourcePropertyDlg::acceptSourceProperties(){
	obs_data_t* settings = obs_source_get_settings(mSrc);
	WidgetToData(BILI_DATA_STRING(), GameNameComboBox, settings, "window");
	WidgetToData(BILI_DATA_BOOL(), antiCheatCheckBox, settings, "anti_cheat_hook");
	obs_data_set_bool(settings, "sli_compatibility", true);
	obs_source_update(mSrc, settings);
	obs_data_release(settings);

	return (QDialog::Accepted);
}

void BiLiGameSourcePropertyDlg::mSltRefreshGameListAsync()
{
	mSltRefreshGameListAsync([]()->void{});
}

void BiLiGameSourcePropertyDlg::mSltRefreshGameListAsync(std::function<void()>&& onCompleted)
{
	RefresnBtn->setEnabled(false);

	GameNameComboBox->clear();
	GameNameComboBox->addItem(tr("Refresing..."));

	//在对话框被析构之后，这些也都没了
	std::shared_ptr<bool> refIsDlgClosed = isDlgClosed;
	obs_source_addref(mSrc);
	obs_source_t* refSrc = mSrc;
	//所以要留一份

	App()->mGetMainWindow()->mPostTask([this, refSrc, refIsDlgClosed, onCompleted]()->void*{
		obs_properties_t* props = obs_source_properties(refSrc);
		App()->mGetMainWindow()->mInvokeProcdure([this, props, refSrc, refIsDlgClosed, onCompleted]()->void*{

			if (*refIsDlgClosed == false)
			{
				DataToWidget(BILI_PROP_LIST_STRING(), GameNameComboBox, props, "window");
			}

			obs_properties_destroy(props);
			obs_source_release(refSrc);

			if (*refIsDlgClosed == false)
			{
				RefresnBtn->setEnabled(true);
			}

			onCompleted();

			return 0;
		});
		return 0;
	}
	);
}

BiLiGameSourcePropertyDlg::~BiLiGameSourcePropertyDlg()
{
	*isDlgClosed = true;
}
