#ifndef PUSHSTREAMSETTINGWID_H
#define PUSHSTREAMSETTINGWID_H

#include <QWidget>
#include "ui_PushStreamSettingWid.h"

#include "BiliUIConfigSync.hpp"
#include <vector>
#include "util/util.hpp"

#include <memory>

class PushStreamSettingWid : public QWidget
{
	Q_OBJECT

public:
	PushStreamSettingWid(ConfigFile& configData_, bool isDisableCustomPushUrl, QWidget *parent = 0);
	~PushStreamSettingWid();

	void LoadConfig();
	bool SaveConfig();

private:
	void UpdateUI();
	ConfigFile& configData;

private slots:
	void serverTypeToggled(QAbstractButton * button, bool checked);

private:
	Ui::PushStreamSettingWid ui;
};

#endif // PUSHSTREAMSETTINGWID_H
