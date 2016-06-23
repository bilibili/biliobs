#ifndef ADVSETTTINGBASEDLG_H
#define ADVSETTTINGBASEDLG_H

#include <QDialog>
#include "ui_advsetttingbasedlg.h"

class AdvSetttingBaseDlg : public QDialog {

	Q_OBJECT

public:
	AdvSetttingBaseDlg(QWidget *parent = 0);
	~AdvSetttingBaseDlg();

	QStackedWidget *getStackedWid_(){
		return ui.SettingStackedWid;
	};
	QWidget *getBtnContainerWid_(){
		return ui.LeftBtnWid;
	};
	QButtonGroup *btnGroup_;
	virtual int saveSetting_() = 0;
	virtual int cancelSetting_() = 0;
	virtual void show_() = 0;
	virtual QPushButton *addBtn_() = 0;
	virtual void addStackedPageWid_(int index, QWidget* wid = NULL) = 0;

protected:
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);

public slots:
	void sltStackChanged(int btnIndex);
	void sltSaveSetting();
	void sltCancel();

protected:
	Ui::AdvSetttingBaseDlg ui;

private:
	bool mIsPress;
	QPoint mPoint;
};

#endif // ADVSETTTINGBASEDLG_H