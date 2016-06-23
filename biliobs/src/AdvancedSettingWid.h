#ifndef ADVANCEDSETTINGWID_H
#define ADVANCEDSETTINGWID_H

#include <QWidget>
#include "ui_AdvancedSettingWid.h"

class AdvancedSettingWid : public QDialog {
	Q_OBJECT

public:
	AdvancedSettingWid(QWidget *parent = 0);
	~AdvancedSettingWid();

	void mAddStackedPageWid(int index, QWidget* wid = NULL);
	void mShow();

protected:
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);

private:
	Ui::AdvancedSettingWid ui;

	bool mIsPress;
	QPoint mPoint;

private slots:
	void mSltStackChanged(int index);
	void mSltSaveSetting();
};

#endif // ADVANCEDSETTINGWID_H
