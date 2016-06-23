#include "HotkeyItemWid.h"
#include "stdint.h"
#include "../libobs/obs-interaction.h"
#include "../libobs/util/dstr.hpp"
#include <QKeyEvent>

uint32_t TranslateQtKeyboardEventModifiers(Qt::KeyboardModifiers mods);


HotkeyItemWid::HotkeyItemWid(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    ui.removeHotkeyButton->setEnabled(false);
	QObject::connect(ui.removeHotkeyButton, SIGNAL(clicked()), this, SLOT(onRemoveHotkeyButtonClicked()));
    connect(ui.hotkeyEdit, &QLineEdit::textChanged, this, &HotkeyItemWid::contextChanged);

    ui.removeHotkeyButton->setFocusPolicy(Qt::NoFocus);

	ui.hotkeyEdit->installEventFilter(this);

	obsHotKey.key = OBS_KEY_NONE;
	obsHotKey.modifiers = 0;
}

HotkeyItemWid::~HotkeyItemWid()
{
}

void HotkeyItemWid::UpdateText()
{
	if (obsHotKey.key == OBS_KEY_NONE)
	{
		ui.hotkeyEdit->setText("");
	}
	else
	{
		DStr keyStr;
		obs_key_combination_to_str(obsHotKey, keyStr);
		ui.hotkeyEdit->setText(QString::fromUtf8(keyStr));
	}
}

void HotkeyItemWid::contextChanged()
{
    //bool focused;
    //if (ui.removeHotkeyButton->hasFocus())
    //    focused = true;
    //else
    //    focused = false;

    ui.removeHotkeyButton->setEnabled(!ui.hotkeyEdit->text().isEmpty());

    //if (focused)
    //    ui.hotkeyEdit->setFocus();
}

bool HotkeyItemWid::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::KeyPress && watched == static_cast<QObject*>(ui.hotkeyEdit))
	{
		QKeyEvent* kevt = static_cast<QKeyEvent*>(event);

		if (kevt->isAutoRepeat())
			return true;

		obs_key_combination_t new_key;

		switch (kevt->key()) {
		case Qt::Key_Shift:
		case Qt::Key_Control:
		case Qt::Key_Alt:
		case Qt::Key_Meta:
			new_key.key = OBS_KEY_NONE;
			break;

		default:
			new_key.key =
				obs_key_from_virtual_key(kevt->nativeVirtualKey());
		}

		new_key.modifiers =
			TranslateQtKeyboardEventModifiers(kevt->modifiers());

		obsHotKey = new_key;

		UpdateText();

		return true;
	}
	else
		return false;
}

void HotkeyItemWid::SetHotkeyName(const QString& hotkeyName)
{
	ui.hotkeyName->setText(hotkeyName);
}

void HotkeyItemWid::SetHotkey(obs_key_combination_t hotkey)
{
	obsHotKey = hotkey;
	UpdateText();
}

obs_key_combination_t HotkeyItemWid::GetHotkey()
{
	return obsHotKey;
}

void HotkeyItemWid::onRemoveHotkeyButtonClicked()
{
	ui.hotkeyEdit->setText("");
	obsHotKey.key = OBS_KEY_NONE;
	obsHotKey.modifiers = 0;
}
