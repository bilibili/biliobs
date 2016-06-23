#ifndef SUSPENDLOCKWID_H
#define SUSPENDLOCKWID_H

#include <QLabel>
#include "ui_suspendlockwid.h"

class SuspendLockWid : public QLabel {

	Q_OBJECT

public:
	SuspendLockWid(QWidget *parent = 0);
	~SuspendLockWid();

	void changeOpacity(float d);
signals:
	void sglClicked();
protected:
	virtual bool eventFilter(QObject *o, QEvent *e);

private:
	Ui::SuspendLockWid ui;

	QString bg_style_;
	QString icon_style_;
};

#endif // SUSPENDLOCKWID_H
