#include "HotkeyTriggeredNotice.h"
#include <QDesktopWidget>
#include <QTimer>
#include "BiLiApp.h"
#include "BiLiOBSMainWid.h"

#include <Windows.h>

HotkeyTriggeredNotice::HotkeyTriggeredNotice(QString icon, QString text, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowOpacity(0.618);
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground, true);

	ui.iconLabel->setPixmap(QPixmap(icon));
	ui.textLabel->setText(text);

	if (curInstance)
		curInstance->deleteLater();
	curInstance = this;
}

HotkeyTriggeredNotice::~HotkeyTriggeredNotice()
{
	if (curInstance == this)
		curInstance->deleteLater();
}

void HotkeyTriggeredNotice::showEvent(QShowEvent* event)
{
	HWND hwnd = (HWND)winId();
	LONG_PTR curExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	curExStyle |= WS_EX_TOPMOST;
	curExStyle |= WS_EX_TOOLWINDOW;
	curExStyle |= WS_EX_TRANSPARENT;
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, curExStyle);

	SetWindowPos(hwnd, HWND_TOPMOST,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE
		);

	QRect scrRect = QApplication::desktop()->screenGeometry(QCursor::pos());
	move(scrRect.left() + scrRect.width() / 2 - width() / 2,
		scrRect.top() + scrRect.height() * 0.618 - height() / 2);

	QTimer::singleShot(2000, this, SLOT(onAutoClose()));
}

void HotkeyTriggeredNotice::onAutoClose()
{
	close();
}

HotkeyTriggeredNotice* HotkeyTriggeredNotice::curInstance = 0;
