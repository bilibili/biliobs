#define DEFINE_IMPL_PROPDLG

#include "BiLiPropertyDlg.h"
#include <QLabel>
#include <QPlainTextEdit>
#include <QFontComboBox>
#include <QCheckBox>
#include <QColorDialog>
#include <QRadioButton>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>

#include "BiliUIConfigSync.hpp"
#include "BiliFilterUtility.hpp"
#include "BiliOBSUtility.hpp"

#include "circle_slider_slider.h"

static const char* scroll_filter_id = "scroll_filter";

IMPL_PROPDLG(BiLiTextSourcePropertyDlg);
QFontComboBox* FontComboBox; 
QPushButton* ColorChangeBtn;
QPlainTextEdit* PlainTextEdit;
QCheckBox *FromFileCheckBox;
QPlainTextEdit* FilePathEdit;
CircleSliderSlider* ScrollSpeedSlider;
QLabel* OpacityValLabel;
QLabel* ScrollSpeedValLabel;
obs_source_t* scrollFilter;

void mSltFontComboxChanged(const QString &text);
void mSltColorChangeBtn();
void mSltTxtChanged();

void OnOpacitySliderChanged(int val);
void OnScrollSpeedSliderChanged(int val);
END_IMPL_PROPDLG(BiLiTextSourcePropertyDlg, "text_ft2_source");

BiLiTextSourcePropertyDlg::~BiLiTextSourcePropertyDlg() {}

static inline QColor color_from_int(long long val) {

	return QColor( val        & 0xff,
		      (val >>  8) & 0xff,
		      (val >> 16) & 0xff,
		      (val >> 24) & 0xff);
}

static unsigned shift(unsigned val, int shift)
{
	return ((val & 0xff) << shift);
}

static inline long long color_to_int(QColor color) {
	return  shift(color.red(), 0) |
		shift(color.green(),  8) |
		shift(color.blue(),  16) |
		shift(color.alpha(), 24);
}

void BiLiTextSourcePropertyDlg::setupSourcePropertiesUI() {
	//注意：添加控件时，记得根据需要在最后添加控件的变动通知监视！
	//否则可能导致点了确定之后设置没有保存进去

	ui.PropertyNameLab->setText(tr("Text Property")); 


	//select from file
	auto FileTxtLayout = new QHBoxLayout();
	FileTxtLayout->setSpacing(0);
    FileTxtLayout->setContentsMargins(0, 0, 0, 0);

	FromFileCheckBox = new QCheckBox(ui.PropertyWid);
    FromFileCheckBox->setFixedHeight(13);
	FromFileCheckBox->setObjectName(QStringLiteral("FromFileCheckBox"));
	FromFileCheckBox->setProperty("FromFileCheckBox", qVPtr<QCheckBox>::toVariant(FromFileCheckBox) );

	auto BrowseFileBtn = new QPushButton(ui.PropertyWid);
	BrowseFileBtn->setObjectName(QStringLiteral("BrowseFileBtn"));
	QSizePolicy sizePolicyBrowseFileBtn(QSizePolicy::Fixed, QSizePolicy::Fixed);
	sizePolicyBrowseFileBtn.setHeightForWidth(BrowseFileBtn->sizePolicy().hasHeightForWidth());
	BrowseFileBtn->setSizePolicy(sizePolicyBrowseFileBtn);
	BrowseFileBtn->setMinimumSize(QSize(50, 25));
	BrowseFileBtn->setMaximumSize(QSize(50, 25));

	FilePathEdit = new QPlainTextEdit();
	FilePathEdit->setObjectName(QStringLiteral("FilePathEdit"));
	QSizePolicy sizePolicyFilePathEdit(QSizePolicy::Expanding, QSizePolicy::Fixed);
	sizePolicyFilePathEdit.setHorizontalStretch(0);
	sizePolicyFilePathEdit.setVerticalStretch(0);
	sizePolicyFilePathEdit.setHeightForWidth(FilePathEdit->sizePolicy().hasHeightForWidth());
	FilePathEdit->setSizePolicy(sizePolicyFilePathEdit);
	FilePathEdit->setFixedHeight(30);
	FilePathEdit->setReadOnly(true);
	connect(BrowseFileBtn, &QPushButton::clicked, [this](){
		QString path = QFileDialog::getOpenFileName(this, tr("Select File"),
			QDir::currentPath(), tr("TxtFile (*.txt *.int *.log )"));
		if (path.isEmpty())
			return;
		QFileInfo fileInfo(path);
		if (!fileInfo.isFile())
			return;
		FilePathEdit->setPlainText(path);
	});

    QSpacerItem *fileSpacer0 = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *fileSpacer1 = new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    //QSpacerItem *fileSpacer2 = new QSpacerItem(4, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
	FileTxtLayout->addWidget(FromFileCheckBox);
    FileTxtLayout->addItem(fileSpacer0);
	FileTxtLayout->addWidget(BrowseFileBtn);
    FileTxtLayout->addItem(fileSpacer1);
	FileTxtLayout->addWidget(FilePathEdit);

	PlainTextEdit = new QPlainTextEdit();
	PlainTextEdit->setObjectName(QStringLiteral("PlainTextEdit"));
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(PlainTextEdit->sizePolicy().hasHeightForWidth());
	PlainTextEdit->setSizePolicy(sizePolicy);
	PlainTextEdit->setFixedHeight(60);
	connect(PlainTextEdit, SIGNAL(textChanged()), this, SLOT(mSltTxtChanged()));

	auto FontHLayout = new QHBoxLayout();
    FontHLayout->setSpacing(0);
    FontHLayout->setContentsMargins(0, 0, 27, 0);
	auto FontLab = new QLabel(ui.PropertyWid);
	FontLab->setObjectName(QStringLiteral("FontLab"));
	QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
	sizePolicy1.setHorizontalStretch(0);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(FontLab->sizePolicy().hasHeightForWidth());
	FontLab->setSizePolicy(sizePolicy1);
	FontLab->setMinimumSize(QSize(62, 30));
	FontLab->setMaximumSize(QSize(62, 30));
	FontLab->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	FontHLayout->addWidget(FontLab);

    QSpacerItem *FontHLayout_spacer_h0 = new QSpacerItem(4, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    FontHLayout->addItem(FontHLayout_spacer_h0);

	FontComboBox = new QFontComboBox(ui.PropertyWid);
    FontComboBox->setFixedSize(142, 30);
	FontComboBox->setObjectName(QStringLiteral("FontComboBox"));
	FontComboBox->setStyleSheet(QStringLiteral(""));
	FontHLayout->addWidget(FontComboBox);
	connect(FontComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(mSltFontComboxChanged(const QString &)));

    QSpacerItem *FontHLayout_spacer_h1 = new QSpacerItem(30, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    FontHLayout->addItem(FontHLayout_spacer_h1);

	auto ColorLab = new QLabel(ui.PropertyWid);
	ColorLab->setObjectName(QStringLiteral("ColorLab"));
	sizePolicy1.setHeightForWidth(ColorLab->sizePolicy().hasHeightForWidth());
	ColorLab->setSizePolicy(sizePolicy1);
    ColorLab->setMinimumSize(QSize(40, 30));
    ColorLab->setMaximumSize(QSize(40, 30));
	ColorLab->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	FontHLayout->addWidget(ColorLab);

    QSpacerItem *FontHLayout_spacer_h2 = new QSpacerItem(4, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    FontHLayout->addItem(FontHLayout_spacer_h2);

    QVBoxLayout *ColorChangeBtn_layout = new QVBoxLayout();
    ColorChangeBtn_layout->setSpacing(0);
    ColorChangeBtn_layout->setContentsMargins(0, 5, 0, 5);
	ColorChangeBtn = new QPushButton(ui.PropertyWid);
	ColorChangeBtn->setObjectName(QStringLiteral("ColorChangeBtn"));
	sizePolicy1.setHeightForWidth(ColorChangeBtn->sizePolicy().hasHeightForWidth());
	ColorChangeBtn->setSizePolicy(sizePolicy1);
	ColorChangeBtn->setMinimumSize(QSize(30, 20));
	ColorChangeBtn->setMaximumSize(QSize(30, 20));
    ColorChangeBtn_layout->addWidget(ColorChangeBtn);
    FontHLayout->addLayout(ColorChangeBtn_layout);
	connect(ColorChangeBtn, &QPushButton::clicked, this, &BiLiTextSourcePropertyDlg::mSltColorChangeBtn);

    auto StyleHSpacer1 = new QSpacerItem(4, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    auto StyleHSpacer2 = new QSpacerItem(30, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
	auto StyleHLayout = new QHBoxLayout();
    StyleHLayout->setContentsMargins(0, 0, 151, 0);
    StyleHLayout->setSpacing(0);
	auto StyleLab = new QLabel(ui.PropertyWid);
	StyleLab->setObjectName(QStringLiteral("StyleLab"));
	sizePolicy1.setHeightForWidth(StyleLab->sizePolicy().hasHeightForWidth());
	StyleLab->setSizePolicy(sizePolicy1);
	StyleLab->setMinimumSize(QSize(62, 13));
	StyleLab->setMaximumSize(QSize(62, 13));
	StyleLab->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	StyleHLayout->addWidget(StyleLab);
	StyleHLayout->addItem(StyleHSpacer1);

	auto BoldStyleCheckBox = new QCheckBox(ui.PropertyWid);
    BoldStyleCheckBox->setFixedHeight(13);
	BoldStyleCheckBox->setObjectName(QStringLiteral("BoldStyleCheckBox"));
	StyleHLayout->addWidget(BoldStyleCheckBox);
	auto ItalicCheckBox = new QCheckBox(ui.PropertyWid);
    ItalicCheckBox->setFixedHeight(13);
	ItalicCheckBox->setObjectName(QStringLiteral("ItalicCheckBox"));
	StyleHLayout->addItem(StyleHSpacer2);
	StyleHLayout->addWidget(ItalicCheckBox);
	FontComboBox->setProperty("BoldStyleCheckBox", qVPtr<QCheckBox>::toVariant(BoldStyleCheckBox) );
	FontComboBox->setProperty("ItalicCheckBox", qVPtr<QCheckBox>::toVariant(ItalicCheckBox) );

	auto OpacitySliderHLayout = new QHBoxLayout();
    OpacitySliderHLayout->setSpacing(0);
    OpacitySliderHLayout->setContentsMargins(0, 0, 0, 0);
	auto OpacityLabel = new QLabel();
	OpacityLabel->setObjectName("OpacityLabel");
	OpacityLabel->setText(QApplication::translate("TextAddForm", "Opacity:", 0));
	OpacityLabel->setMinimumSize(QSize(62, 16));
	OpacityLabel->setMaximumSize(QSize(62, 16));
	OpacityLabel->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	OpacityValLabel = new QLabel();
	OpacityValLabel->setObjectName("OpacityValLabel");
	OpacityValLabel->setFixedSize(38, 16);
	OpacityValLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	auto OpacitySlider = new CircleSliderSlider(this);
    OpacitySlider->setFixedWidth(150);
	OpacitySlider->setRange(0, 255, 0); 
    QSpacerItem *OpacitySliderHLayout_spacer0 = new QSpacerItem(4, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *OpacitySliderHLayout_spacer1 = new QSpacerItem(8, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *OpacitySliderHLayout_spacer2 = new QSpacerItem(8, 10, QSizePolicy::Expanding, QSizePolicy::Fixed);

	OpacitySliderHLayout->addWidget(OpacityLabel);
    OpacitySliderHLayout->addItem(OpacitySliderHLayout_spacer0);
	OpacitySliderHLayout->addWidget(OpacitySlider);
    OpacitySliderHLayout->addItem(OpacitySliderHLayout_spacer1);
	OpacitySliderHLayout->addWidget(OpacityValLabel);
    OpacitySliderHLayout->addItem(OpacitySliderHLayout_spacer2);
	ColorChangeBtn->setProperty("OpacitySlider", qVPtr<CircleSliderSlider>::toVariant(OpacitySlider));
	ColorChangeBtn->setProperty("OpacityValLabel", qVPtr<QLabel>::toVariant(OpacityValLabel) );

    QSpacerItem *ScrollSpeedSliderHLayout_spacer0 = new QSpacerItem(4, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *ScrollSpeedSliderHLayout_spacer1 = new QSpacerItem(8, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *ScrollSpeedSliderHLayout_spacer2 = new QSpacerItem(8, 10, QSizePolicy::Expanding, QSizePolicy::Fixed);
	auto ScrollSpeedSliderHLayout = new QHBoxLayout();
    ScrollSpeedSliderHLayout->setSpacing(0);
    ScrollSpeedSliderHLayout->setContentsMargins(0, 0, 0, 0);
	auto ScrollSpeedLabel = new QLabel();
	ScrollSpeedLabel->setObjectName("ScrollSpeedLabel");
	ScrollSpeedLabel->setText(QApplication::translate("TextAddForm", "ScrollSpeed:", 0));
	ScrollSpeedLabel->setMinimumSize(QSize(62, 16));
	ScrollSpeedLabel->setMaximumSize(QSize(62, 16));
	ScrollSpeedLabel->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

	ScrollSpeedValLabel = new QLabel();
	ScrollSpeedValLabel->setObjectName("ScrollSpeedValLabel");
	ScrollSpeedSlider = new CircleSliderSlider(nullptr);
    ScrollSpeedSlider->setFixedWidth(150);
	ScrollSpeedValLabel->setFixedWidth(38);
    ScrollSpeedValLabel->setFixedHeight(16);
	ScrollSpeedValLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	ScrollSpeedSlider->setRange(0, 300, 0);

	ScrollSpeedSliderHLayout->addWidget(ScrollSpeedLabel);
    ScrollSpeedSliderHLayout->addItem(ScrollSpeedSliderHLayout_spacer0);
	ScrollSpeedSliderHLayout->addWidget(ScrollSpeedSlider);
    ScrollSpeedSliderHLayout->addItem(ScrollSpeedSliderHLayout_spacer1);
	ScrollSpeedSliderHLayout->addWidget(ScrollSpeedValLabel);
    ScrollSpeedSliderHLayout->addItem(ScrollSpeedSliderHLayout_spacer2);

	auto MainVLayout = new QVBoxLayout(ui.PropertyWid);
	MainVLayout->setObjectName(QStringLiteral("MainVLayout"));
	MainVLayout->setContentsMargins(14, 20, 14, 30);
    MainVLayout->setSpacing(0);

	QSpacerItem *spacer_v0 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *spacer_v1 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *spacer_v2 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *spacer_v3 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *spacer_v4 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);

	MainVLayout->addWidget(PlainTextEdit);
	MainVLayout->addItem(spacer_v0);
	MainVLayout->addLayout(FileTxtLayout);
	MainVLayout->addItem(spacer_v4);
	MainVLayout->addLayout(FontHLayout);
    MainVLayout->addItem(spacer_v1);
	MainVLayout->addLayout(StyleHLayout);
    MainVLayout->addItem(spacer_v2);
	MainVLayout->addLayout(OpacitySliderHLayout);
    MainVLayout->addItem(spacer_v3);
	MainVLayout->addLayout(ScrollSpeedSliderHLayout);

	FontLab->setText(QApplication::translate("TextAddForm", "Font:", 0));
	ColorLab->setText(QApplication::translate("TextAddForm", "Color:", 0));
	StyleLab->setText(QApplication::translate("TextAddForm", "Style:", 0));
	BoldStyleCheckBox->setText(QApplication::translate("TextAddForm", "Bold", 0));
	ItalicCheckBox->setText(QApplication::translate("TextAddForm", "Italic", 0));
	FromFileCheckBox->setText(QApplication::translate("TextAddForm", "FromFile", 0));
	BrowseFileBtn->setText(QApplication::translate("TextAddForm", "Browse", 0));

	QMetaObject::connectSlotsByName(ui.PropertyWid);

	obs_data_t* settings = obs_source_get_settings(mSrc);
	DataToWidget(BILI_DATA_FONT(), FontComboBox, settings, "font");
	DataToWidget(BILI_DATA_INT(), ColorChangeBtn, settings, "color1");
	DataToWidget(BILI_DATA_STRING(), PlainTextEdit, settings, "text");
	DataToWidget(BILI_DATA_STRING(), FilePathEdit, settings, "text_file");
	DataToWidget(BILI_DATA_BOOL(), FromFileCheckBox, settings, "from_file");
	obs_data_release(settings);

	//为文字源创建filter
	scrollFilter = obs_source_get_filter_by_name(mSrc, scroll_filter_id);
	if (!scrollFilter)
	{
		scrollFilter = obs_source_create(OBS_SOURCE_TYPE_FILTER, scroll_filter_id, scroll_filter_id, 0, 0);
		obs_source_filter_add(mSrc, scrollFilter);
		obs_data_t* settings = obs_data_create();
		obs_data_set_bool(settings, "disable_repeat_if_no_speed", true);
		obs_source_update(scrollFilter, settings);
		obs_data_release(settings);
	}

	QObject::connect(ScrollSpeedSlider, &CircleSliderSlider::valueChanged, this, &BiLiTextSourcePropertyDlg::OnScrollSpeedSliderChanged);

	QObject::connect(OpacitySlider, &CircleSliderSlider::valueChanged, this, &BiLiTextSourcePropertyDlg::OnOpacitySliderChanged);

	//读取filter设置
	FilterDataToWidget(BILI_DATA_DOUBLE(), ScrollSpeedSlider, mSrc, scroll_filter_id, "speed_x");
	//读取透明度设置
	settings = obs_source_get_settings(mSrc);
	int64_t color1 = obs_data_get_int(settings, "color1");
	obs_data_release(settings);
	int opacity = (color1 >> 24) & 0xff;
	OpacitySlider->setValue(opacity);

	ScrollSpeedValLabel->setText(QString("%1%").arg(ScrollSpeedSlider->value()));
	OpacityValLabel->setText(QString("%1%").arg(OpacitySlider->value() * 100 / 255));

	//添加监听控件变动
	mChangeEvnetFilter->Watch({ PlainTextEdit, FontComboBox, /*ColorChangeBtn, */BoldStyleCheckBox, ItalicCheckBox, OpacitySlider, ScrollSpeedSlider, FromFileCheckBox, FilePathEdit  });
	connect(mChangeEvnetFilter.get(), SIGNAL(OnChangedSignal()), this, SLOT(mSltOnSettingChanged()));
}

int BiLiTextSourcePropertyDlg::acceptSourceProperties() {
	obs_data_t* settings = obs_source_get_settings(mSrc);
	WidgetToData(BILI_DATA_FONT(), FontComboBox, settings, "font");
	WidgetToData(BILI_DATA_INT(), ColorChangeBtn, settings, "color1");
	WidgetToData(BILI_DATA_INT(), ColorChangeBtn, settings, "color2");
	WidgetToData(BILI_DATA_STRING(), PlainTextEdit, settings, "text");
	WidgetToData(BILI_DATA_BOOL(), FromFileCheckBox, settings, "from_file");
	WidgetToData(BILI_DATA_STRING(), FilePathEdit, settings, "text_file");
	obs_source_update(mSrc, settings);
	obs_data_release(settings);

	WidgetToFilterData(BILI_DATA_DOUBLE(), ScrollSpeedSlider, mSrc, scroll_filter_id, "speed_x");

	return (QDialog::Accepted);
}

void BiLiTextSourcePropertyDlg::mSltTxtChanged() {

	QPlainTextEdit *edit = static_cast<QPlainTextEdit*>(sender());
	obs_data_t *settingText = obs_source_get_settings(mSrc);
	obs_data_set_string(settingText, "text", edit->toPlainText().toUtf8().data());
	obs_data_release(settingText);
}

void BiLiTextSourcePropertyDlg::mSltColorChangeBtn() {

	QPushButton *btn = qobject_cast<QPushButton *>(sender());

	int64_t colorVal = btn->property("color1ColorInt64").toLongLong();

	QColor color1Color = color_from_int(colorVal);

	QColorDialog::ColorDialogOptions options = QColorDialog::ShowAlphaChannel;
#ifdef __APPLE__
		options |= QColorDialog::DontUseNativeDialog;
#endif
	color1Color = QColorDialog::getColor(color1Color, this, tr("Select color"), options);
	if (!color1Color.isValid())
		return;


	colorVal = color_to_int(color1Color);
	//set alpha = 255，强制去除透明
	//color1Val |= 0xff000000;
	btn->setProperty("color1ColorInt64", colorVal);
	SetPushButtonBackgroundColor(btn, colorVal);

	mChangeEvnetFilter->OnChanged();
}

void BiLiTextSourcePropertyDlg::mSltFontComboxChanged(const QString &text) {

	QFont font = qobject_cast<QFontComboBox *>(sender())->currentFont();

	obs_data_t *fontObj = obs_data_create();

	obs_data_set_string(fontObj, "face", (font.family()).toUtf8().data());
	obs_data_set_string(fontObj, "style", (font.styleName()).toUtf8().data());
	obs_data_set_int(fontObj, "size", font.pointSize());

	int flags  = font.bold() ? OBS_FONT_BOLD : 0;
	flags |= font.italic() ? OBS_FONT_ITALIC : 0;
	flags |= font.underline() ? OBS_FONT_UNDERLINE : 0;
	flags |= font.strikeOut() ? OBS_FONT_STRIKEOUT : 0;
	obs_data_set_int(fontObj, "flags", flags);

	obs_data_t *settingFont = obs_source_get_settings(mSrc);
	obs_data_set_obj(settingFont, "font", fontObj);
	obs_data_release(fontObj);
	obs_data_release(settingFont);
}

void BiLiTextSourcePropertyDlg::OnOpacitySliderChanged(int val)
{
	OpacityValLabel->setText(QString("%1%").arg(val * 100 / 255));

	obs_data_t *settings = obs_source_get_settings(mSrc);
	int color = obs_data_get_int(settings, "color1");
	val = (color & 0x00FFFFFF) | (val << 24);

	obs_data_set_int(settings, "color1", val);
	obs_data_set_int(settings, "color2", val);
	obs_source_update(mSrc, settings);
	obs_data_release(settings);
}

void BiLiTextSourcePropertyDlg::OnScrollSpeedSliderChanged(int val)
{
	obs_data_t* filterSettings = obs_source_get_settings(scrollFilter);
	obs_data_set_int(filterSettings, "speed_x", val);
	obs_source_update(scrollFilter, filterSettings);
	obs_data_release(filterSettings);

	ScrollSpeedValLabel->setText(QString("%1%").arg(val));
}
