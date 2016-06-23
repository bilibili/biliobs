
#ifndef RIGHT_LIST_TOOL_BAR_H
#define RIGHT_LIST_TOOL_BAR_H

class QPushButton;

#include <qwidget.h>

class RightListToolbar : public QWidget  {
	Q_OBJECT

public:
	RightListToolbar(QWidget *parent = 0);
	

signals:
	void mvUpSignal();
	void mvDnSignal();
	void mvTopSignal();
	void mvBtmSignal();
	void dltSignal();


private:
	void setUpUi();
	void establishConn();

private:
	QPushButton *mv_up_btn_;
	QPushButton *mv_dn_btn_;
	QPushButton *mv_top_btn_;
	QPushButton *mv_btm_btn_;
	QPushButton *dlt_btn_;
};

#endif