#ifndef HOTKEYSETTINGWID_H
#define HOTKEYSETTINGWID_H

#include <QWidget>
#include "ui_HotkeySettingWid.h"

#include "HotkeyItemWid.h"
#include "../libobs/util/util.hpp"

#include <vector>
#include <functional>

class HotkeySettingWid : public QWidget
{
	Q_OBJECT

public:
	HotkeySettingWid(ConfigFile& config, QWidget *parent = 0);
	~HotkeySettingWid();

	bool SaveConfig();

protected:
	std::vector<HotkeyItemWid*> GetHotkeyItems();
	std::vector<HotkeyItemWid*> GetHotkeyItems(std::initializer_list<int> types);

private:
	Ui::HotkeySettingWid ui;
	ConfigFile& mBasicConfig;
};

#endif // HOTKEYSETTINGWID_H
