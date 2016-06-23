#ifndef AUDIODEVICESETTINGWID_H
#define AUDIODEVICESETTINGWID_H

#include <QWidget>
#include "ui_AudioDeviceSettingWid.h"

#include "BiliUIConfigSync.hpp"

class AudioDeviceSettingWid : public QWidget
{
	Q_OBJECT

public:
	AudioDeviceSettingWid(QWidget *parent = 0);
	~AudioDeviceSettingWid();

	bool SaveConfig();

private:
	Ui::AudioDeviceSettingWid ui;
};

#endif // AUDIODEVICESETTINGWID_H
