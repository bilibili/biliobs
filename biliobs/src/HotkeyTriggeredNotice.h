#ifndef HOTKEYTRIGGEREDNOTICE_H
#define HOTKEYTRIGGEREDNOTICE_H

#include <QWidget>
#include "ui_HotkeyTriggeredNotice.h"

class HotkeyTriggeredNotice : public QWidget
{
	Q_OBJECT

public:
	HotkeyTriggeredNotice(QString icon, QString text, QWidget *parent = 0);
	~HotkeyTriggeredNotice();

public slots:
	void onAutoClose();

protected:
	void showEvent(QShowEvent* event) override;

	static HotkeyTriggeredNotice* curInstance;

private:
	Ui::HotkeyTriggeredNotice ui;
};

#endif // HOTKEYTRIGGEREDNOTICE_H
