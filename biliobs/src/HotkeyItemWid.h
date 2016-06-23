#ifndef HOTKEYITEMWID_H
#define HOTKEYITEMWID_H

#include <QWidget>
#include "ui_HotkeyItemWid.h"

#include "../libobs/obs.h"
#include "../libobs/obs-hotkey.h"

class HotkeyItemWid : public QWidget
{
	Q_OBJECT

public:
	HotkeyItemWid(QWidget *parent = 0);
	~HotkeyItemWid();

	void SetHotkeyName(const QString& hotkeyName);
	void SetHotkey(obs_key_combination_t hotkey);
	obs_key_combination_t GetHotkey();

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	void UpdateText();

private slots:
    void contextChanged();

private:
	Ui::HotkeyItemWid ui;
	obs_key_combination_t obsHotKey;

private slots:
	void onRemoveHotkeyButtonClicked();
};

#endif // HOTKEYITEMWID_H
