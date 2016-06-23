#include "VideoSettingWid.h"
#include "obs-config.h"
#include <algorithm>
#include <QFileDialog>
#include <QStyledItemDelegate>
#include <QToolTip>
#include "BiLiApp.h"
#include "BiLiOBSMainWid.h"
#include "BiLiMsgDlg.h"

#include "system_ret_info_dlg.h"

VideoSettingWid::VideoSettingWid(ConfigFile& configData_, QWidget *parent)
	: QWidget(parent)
	, configData(configData_)
{
	ui.setupUi(this);

	QStyledItemDelegate* itemDelegate = new QStyledItemDelegate(this);
	//ui.VBitrateValue->setItemDelegate(itemDelegate);
	//ui.FPSValue->setItemDelegate(itemDelegate);
	ui.PreviewResolutionValue->setItemDelegate(itemDelegate);
	ui.ResolutionValue->setItemDelegate(itemDelegate);
	ui.VEncoderValue->setItemDelegate(itemDelegate);
	ui.VEncoderQualityValue->setItemDelegate(itemDelegate);

	vbPresets.push_back(VBitratePreset(500, tr("500 (smooth, for bad network)")));
	vbPresets.push_back(VBitratePreset(700, tr("700 (smooth)")));
	vbPresets.push_back(VBitratePreset(1000, tr("1000 (clear)")));
	vbPresets.push_back(VBitratePreset(1200, tr("1200 (high, recommend)")));
	vbPresets.push_back(VBitratePreset(1600, tr("1600 (high)")));
	vbPresets.push_back(VBitratePreset(2000, tr("2000 (high, for fast network)")));
    vbPresets.push_back(VBitratePreset(2500, tr("2500 (high, Rocket network speed)")));

	//fpsPresets.push_back(FPSPreset(12, tr("12")));
	//fpsPresets.push_back(FPSPreset(15, tr("15")));
	//fpsPresets.push_back(FPSPreset(25, tr("25")));
	fpsPresets.push_back(FPSPreset(30, tr("30 (recommend)")));
    fpsPresets.push_back(FPSPreset(60, tr("60")));

	//resolutionPresets.push_back(ResolutionPreset(640, 480, tr("640x480 (4:3)")));
	resolutionPresets.push_back(ResolutionPreset(712, 400, tr("712x400 (16:9)")));
	resolutionPresets.push_back(ResolutionPreset(800, 600, tr("800x600 (4:3)")));
	resolutionPresets.push_back(ResolutionPreset(1280, 720, tr("1280x720 (16:9) (recommend)")));
    resolutionPresets.push_back(ResolutionPreset(1920, 1080, tr("1920x1080 (16:9)")));

	//previewResolutionPresets.push_back(ResolutionPreset(640, 480, tr("640x480 (4:3)")));
	previewResolutionPresets.push_back(ResolutionPreset(712, 400, tr("712x400 (16:9)")));
	previewResolutionPresets.push_back(ResolutionPreset(800, 600, tr("800x600 (4:3)")));
	previewResolutionPresets.push_back(ResolutionPreset(1280, 720, tr("1280x720 (16:9) (recommend)")));

	qualityPresets.push_back(QualityPreset("ultrafast", "1"));
	qualityPresets.push_back(QualityPreset("superface", "2"));
	qualityPresets.push_back(QualityPreset("veryfast", tr("3 (Default)")));
	qualityPresets.push_back(QualityPreset("faster", "4"));
	qualityPresets.push_back(QualityPreset("fast", "5"));
	//qualityPresets.push_back(QualityPreset("medium", "6"));
	//qualityPresets.push_back(QualityPreset("slow", "7"));
	//qualityPresets.push_back(QualityPreset("slower", "8"));
	//qualityPresets.push_back(QualityPreset("veryslow", "9"));
	//qualityPresets.push_back(QualityPreset("placebo", "10"));

	for (auto& x : vbPresets)
		ui.VBitrateValue->addItem(x.itemText, x.bitrate);
	
	for (auto& x : fpsPresets)
		ui.FPSValue->addItem(x.itemText, x.fps);

	for (auto& x : resolutionPresets)
		ui.ResolutionValue->addItem(x.itemText, (x.baseX << 16) | x.baseY);

	for (auto& x : previewResolutionPresets)
		ui.PreviewResolutionValue->addItem(x.itemText, (x.baseX << 16) | x.baseY);

	for (auto& x : qualityPresets)
		ui.VEncoderQualityValue->addItem(x.itemText, x.presetText);

	ui.VEncoderValue->addItem(tr("Software encoder"));

	ui.VEncoderQualityValue->setCurrentIndex(2);

	LoadConfig();







    open_path_btn_ = new QPushButton();
    open_path_btn_->setObjectName("openPathBtn");
    open_path_btn_->setFixedSize(21, 21);
    open_path_btn_->setCursor(Qt::ArrowCursor);

    QHBoxLayout *recordPathEdit_layout = new QHBoxLayout(ui.recordPathEdit);
    recordPathEdit_layout->setSpacing(0);
    recordPathEdit_layout->setContentsMargins(0, 0, 6, 0);

    QSpacerItem *left_spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);

    recordPathEdit_layout->addItem(left_spacer);
    recordPathEdit_layout->addWidget(open_path_btn_);

    connect(open_path_btn_, &QPushButton::clicked, this, &VideoSettingWid::onOpenPathBtnClicked);
    connect(ui.recordPathEdit, &QLineEdit::textChanged, this, &VideoSettingWid::onRecordEditTextChanged);

    ui.recordPathEdit;




#if 0
	if (isDisableResolution)
	{
		ui.ResolutionValue->setDisabled(true);
		ui.ResolutionValue->installEventFilter(this);

		ui.FPSValue->setDisabled(true);
		ui.FPSValue->installEventFilter(this);

		ui.VBitrateValue->setDisabled(true);
		ui.VBitrateValue->installEventFilter(this);

		ui.recordPathEdit->setDisabled(true);
		ui.recordPathEdit->installEventFilter(this);

		ui.recordPathButton->setDisabled(true);
		ui.recordPathButton->installEventFilter(this);

		ui.VEncoderValue->setDisabled(true);
		ui.VEncoderValue->installEventFilter(this);
	}
#endif
}

VideoSettingWid::~VideoSettingWid() {
}

bool VideoSettingWid::eventFilter(QObject* watched, QEvent* event)
{
	QWidget* w = qobject_cast<QWidget*>(watched);
	if (w != 0)
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			if (w->isEnabled() == false)
			{
				//BiLiMsgDlg dlg;
				//dlg.mSetTitle(tr("Error"));
				//dlg.mSetMsgTxtAndBtn(tr("Can't change during broadcasting or recording."), false);
				//dlg.exec();

                SystemRetInfoDlg dlg;
                dlg.setSubTitle(tr("Error"));
                dlg.setDetailInfo(tr("Can't change during broadcasting or recording."));
                dlg.setTitle("");
                dlg.exec();

				return true;
			}
		}
	}

	return false;
}

void VideoSettingWid::LoadConfig() {

	//DataToWidget(BILI_CONFIG_UINT(), ui.FPSValue, static_cast<config_t*>(configData), "Video\0FPSInt");
	//DataToWidget(BILI_CONFIG_UINT(), ui.VBitrateValue, static_cast<config_t*>(configData), "SimpleOutput\0VBitrate");

    if (config_has_user_value(configData, "Video", "FPSInt")) {
        uint fps = config_get_uint(configData, "Video", "FPSInt");
        int idx = ui.FPSValue->findData(fps);
        if (idx >= 0)
            ui.FPSValue->setCurrentIndex(idx);
        else
            ui.FPSValue->setEditText(QString::number(fps));
    } else {
        uint fps = config_get_default_uint(configData, "Video", "FPSInt");
        int idx = ui.FPSValue->findData(fps);
        if (idx >= 0)
            ui.FPSValue->setCurrentIndex(idx);
        else
            ui.FPSValue->setCurrentIndex(0);
    }

    if (config_has_user_value(configData, "SimpleOutput", "VBitrate")) {
        uint bps = config_get_uint(configData, "SimpleOutput", "VBitrate");
        int idx = ui.VBitrateValue->findData(bps);
        if (idx >= 0)
            ui.VBitrateValue->setCurrentIndex(idx);
        else
            ui.VBitrateValue->setEditText(QString::number(bps));
    }
    else {
        uint bps = config_get_default_uint(configData, "SimpleOutput", "VBitrate");
        int idx = ui.VBitrateValue->findData(bps);
        if (idx >= 0)
            ui.VBitrateValue->setCurrentIndex(idx);
        else
            ui.VBitrateValue->setCurrentIndex(0);
    }


	DataToWidget(BILI_CONFIG_STRING(), ui.recordPathEdit, static_cast<config_t*>(configData), "SimpleOutput\0FilePath");
	DataToWidget(BILI_CONFIG_BOOL(), ui.SyncRecCheckBox, static_cast<config_t*>(configData), "SimpleOutput\0SyncRec");
	DataToWidget(BILI_CONFIG_STRING(), ui.VEncoderQualityValue, static_cast<config_t*>(configData), "SimpleOutput\0Preset");

	bool foundFlag = false;
	outputX_ = config_get_uint(configData, "Video", "OutputCX");
	outputY_ = config_get_uint(configData, "Video", "OutputCY");
	//分辨率
	foundFlag = false;
	for (auto x : resolutionPresets) {
		if (x.baseX == outputX_ && x.baseY == outputY_) {
			ui.ResolutionValue->setCurrentText(x.itemText);
			foundFlag = true;
			break;
		}
	}
	if (!foundFlag)
		ui.ResolutionValue->setCurrentText(resolutionPresets[0].itemText);


	viewX_ = config_get_uint(configData, "Video", "ViewX");
	viewY_ = config_get_uint(configData, "Video", "ViewY");
	//预览窗口分辨率
	foundFlag = false;
	for (auto x : previewResolutionPresets) {
		if (x.baseX == viewX_  && x.baseY == viewY_ ) {
			ui.PreviewResolutionValue->setCurrentText(x.itemText);
			foundFlag = true;
			break;
		}
	}
	if (!foundFlag)
		ui.PreviewResolutionValue->setCurrentText(previewResolutionPresets[0].itemText);
}

bool VideoSettingWid::SaveConfig() {

	int prevX = outputX_;
	int prevY = outputY_;
	config_set_uint(configData, "Video", "PrevOutputCX", outputX_);
	config_set_uint(configData, "Video", "PrevOutputCY", outputY_);
	std::string val = ui.ResolutionValue->currentText().toUtf8().data();
	for (auto x : resolutionPresets){
		if (x.itemText == val.c_str()) {
			outputX_ = x.baseX;
			outputY_ = x.baseY;
		}
	}
	config_set_uint(configData, "Video", "OutputCX", outputX_);
	config_set_uint(configData, "Video", "OutputCY", outputY_);
	config_set_uint(configData, "Video", "BaseCX", outputX_);
	config_set_uint(configData, "Video", "BaseCY", outputY_);

	prevX = viewX_;
	prevY = viewY_;
	config_set_uint(configData, "Video", "PrevViewX", viewX_);
	config_set_uint(configData, "Video", "PrevViewY", viewY_);
	val = ui.PreviewResolutionValue->currentText().toUtf8().data();
	for (auto x : previewResolutionPresets) {
		if (x.itemText == val.c_str()) {
			viewX_ = x.baseX;
			viewY_ = x.baseY;
		}
	}
	config_set_uint(configData, "Video", "ViewX", viewX_);
	config_set_uint(configData, "Video", "ViewY", viewY_);

	//WidgetToData(BILI_CONFIG_UINT(), ui.FPSValue, static_cast<config_t*>(configData), "Video\0FPSInt");
    //WidgetToData(BILI_CONFIG_UINT(), ui.VBitrateValue, static_cast<config_t*>(configData), "SimpleOutput\0VBitrate");

    
    int iii = ui.FPSValue->findText(ui.FPSValue->currentText());
    if (iii >= 0) {
        QVariant ori = ui.FPSValue->currentData();
        config_set_int(configData, "Video", "FPSInt", ori.toInt());
    }
    else {
        bool ok;
        int v = ui.FPSValue->currentText().toInt(&ok);
        if (ok) {
            if (v > 0)
                config_set_int(configData, "Video", "FPSInt", v);

        }
    }

    iii = ui.VBitrateValue->findText(ui.VBitrateValue->currentText());
    if (iii >= 0) {
        QVariant ori = ui.VBitrateValue->currentData();
        config_set_int(configData, "SimpleOutput", "VBitrate", ori.toInt());
    }
    else {
        bool ok;
        int v = ui.VBitrateValue->currentText().toInt(&ok);
        if (ok) {
            if (v > 0)
                config_set_int(configData, "SimpleOutput", "VBitrate", v);

        }
    }



	WidgetToData(BILI_CONFIG_STRING(), ui.recordPathEdit, static_cast<config_t*>(configData), "SimpleOutput\0FilePath");
	WidgetToData(BILI_CONFIG_BOOL(), ui.SyncRecCheckBox, static_cast<config_t*>(configData), "SimpleOutput\0SyncRec");
	WidgetToData(BILI_CONFIG_STRING(), ui.VEncoderQualityValue, static_cast<config_t*>(configData), "SimpleOutput\0Preset");

//	if (prevX != outputX_ || prevY != outputY_)
//		QMetaObject::invokeMethod(App()->mGetMainWindow(), "ResetPreview");
	//if (backupBaseX != baseX || backupBaseY != baseY )
	//	QMetaObject::invokeMethod(App()->mGetMainWindow(), "sltResetPreviewWid");

	return true;
}

void VideoSettingWid::on_recordPathButton_clicked()
{
	QFileDialog fileDlg;
	fileDlg.setFileMode(QFileDialog::DirectoryOnly);
	fileDlg.setOption(QFileDialog::ShowDirsOnly);

	QDir dir(ui.recordPathEdit->text());
	if (dir.exists())
		fileDlg.setDirectory(dir);
	fileDlg.exec();

	if (fileDlg.result() == QDialog::Accepted)
	{
        QString copy = fileDlg.directory().absolutePath();
        QString::iterator it = copy.begin();
        for (; copy.end() != it; it++)
            if (*it == '/')
                *it = '\\';
        
        ui.recordPathEdit->setText(copy);
	}
}

void VideoSettingWid::onOpenPathBtnClicked()
{
    QString copy = ui.recordPathEdit->text();
    QString::iterator it = copy.begin();
    for (; copy.end() != it; it++)
        if (*it == '/')
            *it = '\\';
    
    std::wstring path = copy.toStdWString();
    ShellExecuteW(NULL, L"open", L"explorer.exe", path.c_str(), NULL, SW_SHOWNORMAL);
}

void VideoSettingWid::onRecordEditTextChanged(const QString & text)
{
    open_path_btn_->setVisible(!text.isEmpty());
}
