#ifndef VIDEOSETTINGWID_H
#define VIDEOSETTINGWID_H

#include <QWidget>
#include "ui_VideoSettingWid.h"
#include <stdint.h>

#include <vector>
#include "BiliUIConfigSync.hpp"

#include "util/util.hpp"

struct VBitratePreset
{
	int bitrate;
	QString itemText;

	VBitratePreset(int b_, const QString& t_) : bitrate(b_), itemText(t_) {}
};

struct FPSPreset
{
	int fps;
	QString itemText;

	FPSPreset(int f_, const QString& t_) : fps(f_), itemText(t_) {}
};

struct ResolutionPreset
{
	int baseX;
	int baseY;
	QString itemText;

	ResolutionPreset(int x_, int y_, const QString& t_) : baseX(x_), baseY(y_), itemText(t_) {}
};

struct QualityPreset
{
	QString presetText;
	QString itemText;

	QualityPreset(const QString& pt, const QString& it) : presetText(pt), itemText(it) {}
};

class VideoSettingWid : public QWidget {

	Q_OBJECT

public:
	VideoSettingWid(ConfigFile& configData, QWidget *parent = 0);
	~VideoSettingWid();

	bool SaveConfig();

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	Ui::VideoSettingWid ui;

	ConfigFile& configData;

	void LoadConfig();

	std::vector<VBitratePreset> vbPresets;
	std::vector<FPSPreset> fpsPresets;
	std::vector<ResolutionPreset> resolutionPresets;
	std::vector<ResolutionPreset> previewResolutionPresets;
	std::vector<QualityPreset> qualityPresets;

	uint64_t outputX_;		//stream size
	uint64_t outputY_;

	uint64_t viewX_;		//left center wid size
	uint64_t viewY_;

public slots:
	void on_recordPathButton_clicked();

private slots:
    void onOpenPathBtnClicked();
    void onRecordEditTextChanged(const QString & text);
private:
    QPushButton *open_path_btn_;


};

#endif // VIDEOSETTINGWID_H
